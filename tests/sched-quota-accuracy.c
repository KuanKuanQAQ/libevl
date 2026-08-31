/*
 * SPDX-License-Identifier: MIT
 */

#include <sys/types.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <evl/evl.h>
#include "helpers.h"

#define do_trace(__fmt, __args...)				\
	do {							\
		if (verbose)					\
			evl_printf(__fmt "\n", ##__args);	\
	} while (0)

#define PERIOD_NS		ONE_BILLION
#define NR_PERIODS		1
#define QUOTA_PERCENT		15 /* Allot 15% of the CPU bandwidth per period */
#define DISRUPT_NS		(PERIOD_NS / 20) /* Steal 5% per disruption step */

#define MAX_THREADS	8
#define QUOTA_PRIO	1
#define MAIN_PRIO	2

enum runtime_phase {
	idle_phase,
	calibration_phase,
	accuracy_measurement_phase,
	noise_measurement_phase,
} current_phase = idle_phase;

static struct quota_thread_desc {
	int efd;
	pthread_t tid;
	unsigned long long loops;
} threads[MAX_THREADS];

static int nrthreads = 1;

static int tgid = -1;

static struct evl_event barrier;

static struct evl_mutex lock;

static bool verbose;

static int test_cpu = 1;

static struct evl_sem ready;

static void usage(void)
{
        fprintf(stderr, "usage: sched-quota-accuracy [options]:\n");
        fprintf(stderr, "-c --cpu                     run test on given CPU [=1]\n");
        fprintf(stderr, "-n --num-threads             number of sibling threads\n");
        fprintf(stderr, "-v --verbose                 turn on verbosity\n");
}

#define short_optlist "vn:c:"

static const struct option options[] = {
	{
		.name = "num-threads",
		.has_arg = required_argument,
		.val = 'n',
	},
	{
		.name = "cpu",
		.has_arg = required_argument,
		.val = 'c',
	},
	{
		.name = "verbose",
		.has_arg = no_argument,
		.val = 'v',
	},
	{ /* Sentinel */ }
};

static void set_cpu_affinity(void)
{
	cpu_set_t affinity;
	int ret;

	CPU_ZERO(&affinity);
	CPU_SET(test_cpu, &affinity);
	__Tcall_assert(ret, sched_setaffinity(0, sizeof(affinity), &affinity));
}

static void wait_phase(enum runtime_phase phase)
{
	int ret;

	evl_put_sem(&ready);

	__Tcall_assert(ret, evl_lock_mutex(&lock));
	for (;;) {
		if (current_phase == phase)
			break;
		__Tcall_assert(ret, evl_wait_event(&barrier, &lock));
	}
	__Tcall_assert(ret, evl_unlock_mutex(&lock));
}

static void start_phase(enum runtime_phase phase)
{
	int ret;

	__Tcall_assert(ret, evl_lock_mutex(&lock));
	current_phase = phase;
	__Tcall_assert(ret, evl_broadcast_event(&barrier));
	__Tcall_assert(ret, evl_unlock_mutex(&lock));
}

static void __attribute__(( noinline ))
do_work(struct quota_thread_desc *t, long work_ns)
{
	struct timespec now, to;
	int ret;

	__Tcall_assert(ret, evl_read_clock(EVL_CLOCK_MONOTONIC, &now));
	timespec_add_ns(&to, &now, work_ns);

	while (timespec_sub_ns(&to, &now) > 0) {
		__Tcall_assert(ret, evl_read_clock(EVL_CLOCK_MONOTONIC, &now));
		if (t) {
			t->loops++;
			/*
			 * Yield to siblings. We do that every ten
			 * loops not to overconsume the budget with
			 * scheduling overhead.
			 */
			if (nrthreads > 1 && !(t->loops % 10))
				__Tcall_assert(ret, evl_yield());
		}
	}
}

static void *quota_thread(void *arg)
{
	int serial = (int)(long)arg, ret;
	struct quota_thread_desc *t = threads + serial;
	struct evl_sched_attrs attrs;

	__Tcall_assert(t->efd, evl_attach_self("quota-runner:%d.%d",
						getpid(), serial));

	wait_phase(calibration_phase);

	do_work(t, PERIOD_NS * NR_PERIODS);

	attrs.sched_policy = SCHED_QUOTA;
	attrs.sched_priority = QUOTA_PRIO;
	attrs.sched_quota_group = tgid;
	__Tcall_assert(ret, evl_set_schedattr(t->efd, &attrs));

	wait_phase(accuracy_measurement_phase);

	do_work(t, PERIOD_NS * NR_PERIODS);

	wait_phase(noise_measurement_phase);

	do_work(t, PERIOD_NS * NR_PERIODS);

	return NULL;
}

static int cleanup_group(void)
{
	union evl_sched_ctlparam p;

	p.quota.op = evl_quota_force_remove;
	p.quota.u.remove.tgid = tgid;
	return evl_control_sched(SCHED_QUOTA, &p, NULL, test_cpu);
}

static void cleanup_group_on_sig(int sig, siginfo_t *si, void *context)
{
	if (tgid != -1)
		cleanup_group();
}

struct accuracy_results {
	double accuracy;
	double noise;
};

static struct accuracy_results run_quota(void)
{
	unsigned long long loops_at_calibration, loops_at_accuracy, loops_with_disruption;
	struct accuracy_results results;
	union evl_sched_ctlparam p;
	union evl_sched_ctlinfo q;
	struct timespec now, next;
	struct sigaction sa;
	int ret, n;

	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = cleanup_group_on_sig;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);

	p.quota.op = evl_quota_add;
	__Tcall_assert(ret, evl_control_sched(SCHED_QUOTA, &p, &q, test_cpu));
	tgid = q.quota.tgid;

	p.quota.op = evl_quota_set;
	p.quota.u.set.tgid = tgid;
	p.quota.u.set.quota = QUOTA_PERCENT;
	p.quota.u.set.quota_peak = QUOTA_PERCENT;
	__Tcall_assert(ret, evl_control_sched(SCHED_QUOTA, &p, &q, test_cpu));

	do_trace("CPU%d: new thread group #%d, quota sum is %d%%",
		test_cpu, tgid, q.quota.quota_sum);
	do_trace("CPU%d: calibration x %d thread%s", test_cpu,
		nrthreads, nrthreads > 1 ? "s" : "");

	for (n = 0; n < nrthreads; n++) {
		new_thread(&threads[n].tid, SCHED_FIFO, QUOTA_PRIO, quota_thread, (void *)(long)n);
		__Tcall_assert(ret, evl_get_sem(&ready));
	}

	reset_quota_period(PERIOD_NS);
	start_phase(calibration_phase);

	/*
	 * Wait for the calibration to end, then collect the expected
	 * number of loops we should be able to run during the
	 * allotted quota, even in case of runtime disruption.
	 */
	for (n = 0; n < nrthreads; n++)
		__Tcall_assert(ret, evl_get_sem(&ready));
	for (n = 0, loops_at_calibration = 0; n < nrthreads; n++) {
		do_trace("CPU%d: calibration: quota_thread[%d]: %llu loops",
			test_cpu, n, threads[n].loops);
		loops_at_calibration += threads[n].loops;
		threads[n].loops = 0;
	}

	do_trace("CPU%d: starting accuracy measurement", test_cpu);

	reset_quota_period(PERIOD_NS);
	start_phase(accuracy_measurement_phase);

	/*
	 * Wait for the accuracy measurement to end, then collect the
	 * numbers.
	 */
	for (n = 0; n < nrthreads; n++)
		__Tcall_assert(ret, evl_get_sem(&ready));
	for (n = 0, loops_at_accuracy = 0; n < nrthreads; n++) {
		do_trace("CPU%d: accuracy: quota_thread[%d]: %llu loops",
			test_cpu, n, threads[n].loops);
		loops_at_accuracy += threads[n].loops;
		threads[n].loops = 0;
	}

	do_trace("CPU%d: starting noise measurement", test_cpu);

	reset_quota_period(PERIOD_NS);
	start_phase(noise_measurement_phase);

	/* Start disrupting the runtime by steps of DISRUPT_NS nanoseconds. */
	for (n = 0; n < NR_PERIODS * PERIOD_NS / DISRUPT_NS / 2; n++) {
		evl_read_clock(EVL_CLOCK_MONOTONIC, &now);
		timespec_add_ns(&next, &now, DISRUPT_NS);
		__Tcall_assert(ret, evl_sleep_until(EVL_CLOCK_MONOTONIC, &next));
		do_trace("(...disrupting...)");
		do_work(NULL, DISRUPT_NS);
	}

	for (n = 0; n < nrthreads; n++)
		pthread_join(threads[n].tid, NULL);

	for (n = 0, loops_with_disruption = 0; n < nrthreads; n++) {
		loops_with_disruption += threads[n].loops;
		do_trace("CPU%d: noise: quota_thread[%d]: loops=%llu",
			test_cpu, n, threads[n].loops);
	}

	/*
	 * How much of the full time slice we were able to run for
	 * expressed as a percentage. The closest to the QUOTA_PERCENT
	 * target, the better.
	 */
	results.accuracy = 100.0 *
		((double)loops_at_accuracy * 100.0 / QUOTA_PERCENT) / (double)loops_at_calibration;

	/*
	 * Level of noise observed for the disrupted run compared to
	 * the nominal one. A negative value means that the disrupted
	 * run was faster than the calibration one, which would still
	 * be correct.
	 */
	results.noise = 100.0 - (100.0 *
		((double)loops_with_disruption * 100.0 / QUOTA_PERCENT) / (double)loops_at_calibration);

	__Tcall_assert(ret, cleanup_group());

	return results;
}

int main(int argc, char *argv[])
{
	struct accuracy_results results;
	struct sched_param param;
	int ret, c;

	for (;;) {
		c = getopt_long(argc, argv, short_optlist, options, NULL);
		if (c == EOF)
			break;

		switch (c) {
		case 0:
			break;
		case 'v':
			verbose = true;
			break;
		case 'n':
			nrthreads = atoi(optarg);
			break;
		case 'c':
			test_cpu = atoi(optarg);
			if (test_cpu < 0 || test_cpu >= CPU_SETSIZE)
				error(1, EINVAL, "invalid CPU number");
			break;
		case '?':
		default:
			usage();
			return 1;
		}
	}

	if (optind < argc) {
		usage();
		return 1;
	}

	atexit((void (*)(void))cleanup_group);

	/*
	 * We need all threads to be pinned on the same (oob-capable)
	 * CPU, so that evl_set_schedattr() implicitly refers to the
	 * quota settings we are about to install on that particular
	 * CPU.
	 */
	test_cpu = pick_test_cpu(test_cpu, false, NULL);
	do_trace("picked CPU%d for execution", test_cpu);
	set_cpu_affinity();	/* All test threads inherit this. */

	param.sched_priority = MAIN_PRIO;
	__Tcall_assert(ret, pthread_setschedparam(pthread_self(), SCHED_FIFO, &param));
	__Tcall_assert(ret, evl_attach_self("quota-accuracy:%d", getpid()));

	__Tcall_assert(ret, evl_new_sem(&ready, "quota-ready:%d", getpid()));
	__Tcall_assert(ret, evl_new_event(&barrier, "quota-barrier:%d", getpid()));
	__Tcall_assert(ret, evl_new_mutex(&lock, "quota-lock:%d", getpid()));

	if (nrthreads <= 0)
		nrthreads = 3;
	if (nrthreads > MAX_THREADS)
		error(1, EINVAL, "max %d threads", MAX_THREADS);

	results = run_quota();

	if (verbose)
		do_trace("CPU%d: %d thread%s: quota=%d%%, accuracy=%.1f%% noise=%.1f%%",
			test_cpu,
			nrthreads,
			nrthreads > 1 ?	"s": "",
			QUOTA_PERCENT,
			results.accuracy,
			results.noise);

	emit_info("accuracy(%.1f%%) noise(%.1f%%)", results.accuracy, results.noise);

	return EXIT_NO_STATUS;
}
