/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#include <errno.h>
#include <error.h>
#include <memory.h>
#include <semaphore.h>
#include <evl/thread.h>
#include <evl/sched.h>
#include <cobalt/pthread.h>

struct __cobalt_spawn_args {
	const struct evl_sched_attrs *sched_attrs;
	void *(*start)(void *);
	void *arg;
	sem_t sync;
	int efd;
};

static void *__cobalt_pthread_tramp(void *p)
{
	struct __cobalt_spawn_args *spa = p;
	void *(*start)(void *) = spa->start;
	void *arg = spa->arg, *retval;
	int efd, ret;

	/*
	 * CAUTION: start, arg and efd must be stack-based, since the
	 * spawn args may vanish with the parent thread once the sync
	 * event is posted.
	 */
	efd = evl_attach_self(NULL);
	spa->efd = efd;
	if (efd < 0)
		goto fail;

	ret = evl_set_schedattr(spa->efd, spa->sched_attrs);
	if (ret < 0) {
		spa->efd = ret;
		goto fail;
	}

	sem_post(&spa->sync);
	evl_switch_oob();
	retval = start(arg);
	evl_clear_thread_mode(efd, EVL_T_WOSS, NULL);

	return retval;
fail:
	sem_post(&spa->sync);

	return NULL;
}

COBALT_IMPL(int, pthread_create, (pthread_t *__restrict pptid,
				const pthread_attr_t *__restrict attr,
				void *(*start)(void *arg), void *__restrict arg))
{
	pthread_eattr_t _eattr, *eattr = NULL;
	struct sched_param param;
	int policy;

	if (attr) {
		eattr = &_eattr;
		memset(&eattr->evl, 0, sizeof(eattr->evl));
		pthread_attr_getschedpolicy(attr, &policy);
		pthread_attr_getschedparam(attr, &param);
		eattr->evl.sched_policy = policy;
		eattr->evl.sched_priority = param.sched_priority;
	}

	return pthread_ecreate(pptid, eattr, start, arg);
}

int pthread_ecreate(pthread_t *__restrict pptid,
		const pthread_eattr_t *__restrict eattr,
		void *(*start)(void *), void *__restrict arg)
{
	struct __cobalt_spawn_args spa;
	pthread_eattr_t default_eattr;
	struct timespec timeout;
	pthread_t ptid;
	int ret;

	if (!eattr) {
		pthread_eattr_init(&default_eattr);
		eattr = &default_eattr;
	}

	spa.sched_attrs = &eattr->evl;
	spa.start = start;
	spa.arg = arg;
	spa.efd = -EAGAIN;
	sem_init(&spa.sync, 0, 0);

	ret = pthread_create(&ptid, &eattr->std, __cobalt_pthread_tramp, &spa);
	if (ret) {
		sem_destroy(&spa.sync);
		return ret;
	}

	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 5;
	timeout.tv_nsec = 0;

	for (;;) {
		ret = sem_timedwait(&spa.sync, &timeout);
		if (ret && errno == EINTR)
			continue;
		if (ret == 0) {
			if (spa.efd < 0) {
				ret = -spa.efd;
			} else {
				*pptid = ptid;
				ret = 0;
			}
			break;
		} else if (errno == ETIMEDOUT) {
			ret = EAGAIN;
			break;
		}
		error(1, errno, "sem_wait()");
	}

	sem_destroy(&spa.sync);

	return ret;
}
