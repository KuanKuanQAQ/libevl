/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#pragma GCC system_header
#include_next <pthread.h>

#ifndef _EVL_COBALT_PTHREAD_H
#define _EVL_COBALT_PTHREAD_H

#include <cobalt/sched.h>
#include <cobalt/cond.h>
#include <cobalt/mutex.h>
#include <cobalt/wrappers.h>

typedef struct pthread_eattr {
	pthread_attr_t std;
	struct evl_sched_attrs evl;
} pthread_eattr_t;

#ifdef __cplusplus
extern "C" {
#endif

COBALT_DECL(int, pthread_attr_init, (pthread_attr_t *attr));

COBALT_DECL(int, pthread_create,
	(pthread_t *__restrict ptid_r,
		const pthread_attr_t *__restrict attr,
		void *(*start)(void *),
		void *__restrict arg));

COBALT_DECL(int, pthread_getschedparam,
	    (pthread_t ptid, int *policy,
		    struct sched_param *__restrict param));

COBALT_DECL(int, pthread_setschedparam,
	    (pthread_t ptid, int policy,
		    const struct sched_param *param));

COBALT_DECL(int, pthread_setschedprio, (pthread_t ptid, int prio));

COBALT_DECL(int, pthread_kill, (pthread_t ptid, int sig));

COBALT_DECL(int, pthread_join, (pthread_t ptid, void **retval));

COBALT_DECL(int, pthread_yield, (void));

COBALT_DECL(int, pthread_kill, (pthread_t ptid, int sig));

COBALT_DECL(int, pthread_join, (pthread_t ptid, void **retval));

COBALT_DECL(int, pthread_yield, (void));

int pthread_ecreate(pthread_t *__restrict ptid_r,
		const pthread_eattr_t *__restrict eattr,
		void *(*start)(void *), void *__restrict arg);

int pthread_esetschedparam(pthread_t ptid,
			int policy,
			const struct sched_param *eparam);

int pthread_eattr_init(pthread_eattr_t *eattr);

int pthread_eattr_destroy(pthread_eattr_t *eattr);

int pthread_eattr_setschedpolicy(pthread_eattr_t *eattr,
				int policy);

int pthread_eattr_getschedpolicy(const pthread_eattr_t *__restrict eattr,
				int *__restrict policy);

int pthread_eattr_setschedparam(pthread_eattr_t *__restrict eattr,
				const struct sched_eparam *__restrict eparam);

int pthread_eattr_getschedparam(const pthread_eattr_t *__restrict eattr,
				struct sched_eparam *__restrict eparam);

int pthread_eattr_setinheritsched(pthread_eattr_t *eattr,
				int inheritsched);

int pthread_eattr_getinheritsched(const pthread_eattr_t *__restrict eattr,
				int *__restrict inheritsched);

int pthread_eattr_setdetachstate(pthread_eattr_t *eattr,
				int detachstate);

int pthread_eattr_getdetachstate(const pthread_eattr_t *eattr,
				int *detachstate);

int pthread_eattr_setstacksize(pthread_eattr_t *eattr,
			size_t stacksize);

int pthread_eattr_getstacksize(const pthread_eattr_t *__restrict eattr,
			size_t *__restrict stacksize);

int pthread_eattr_setscope(pthread_eattr_t *eattr,
			int scope);

int pthread_eattr_getscope(const pthread_eattr_t *__restrict eattr,
			int *__restrict scope);

#ifdef __cplusplus
}
#endif

#endif /* _EVL_COBALT_PTHREAD_H */
