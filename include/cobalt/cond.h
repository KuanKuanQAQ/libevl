/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#pragma GCC system_header

#ifndef _EVL_COBALT_COND_H
#define _EVL_COBALT_COND_H

#include <pthread.h>
#include <cobalt/mutex.h>
#include <cobalt/wrappers.h>

typedef struct pthread_econdattr {
	pthread_condattr_t std;
} pthread_econdattr_t;

#ifdef __cplusplus
extern "C" {
#endif

COBALT_DECL(int, pthread_cond_init,
	    (pthread_cond_t *__restrict cond,
		    const pthread_condattr_t *__restrict attr));

COBALT_DECL(int, pthread_cond_destroy, (pthread_cond_t *cond));

COBALT_DECL(int, pthread_cond_wait,
	    (pthread_cond_t *__restrict cond,
		    pthread_mutex_t *__restrict mutex));

COBALT_DECL(int, pthread_cond_timedwait,
	(pthread_cond_t *__restrict cond,
		pthread_mutex_t *__restrict mutex,
		const struct timespec *__restrict abstime));

COBALT_DECL(int, pthread_cond_signal, (pthread_cond_t *cond));

COBALT_DECL(int, pthread_cond_broadcast, (pthread_cond_t *cond));

int pthread_econd_init(pthread_cond_t *__restrict cond,
		const pthread_econdattr_t *__restrict eattr);

#ifdef __cplusplus
}
#endif

#endif /* _EVL_COBALT_COND_H */
