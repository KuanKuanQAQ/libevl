/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#pragma GCC system_header

#ifndef _EVL_COBALT_MUTEX_H
#define _EVL_COBALT_MUTEX_H

#include <pthread.h>
#include <cobalt/wrappers.h>

typedef struct pthread_emutexattr {
	pthread_mutexattr_t std;
} pthread_emutexattr_t;

#ifdef __cplusplus
extern "C" {
#endif

COBALT_DECL(int, pthread_mutex_init,
	    (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr));

COBALT_DECL(int, pthread_mutex_destroy, (pthread_mutex_t *mutex));

COBALT_DECL(int, pthread_mutex_lock, (pthread_mutex_t *mutex));

COBALT_DECL(int, pthread_mutex_timedlock,
	(pthread_mutex_t *__restrict mutex, const struct timespec *__restrict to));

COBALT_DECL(int, pthread_mutex_trylock, (pthread_mutex_t *mutex));

COBALT_DECL(int, pthread_mutex_unlock, (pthread_mutex_t *mutex));

COBALT_DECL(int, pthread_mutex_setprioceiling,
	    (pthread_mutex_t *__restrict mutex, int prioceiling,
	     int *__restrict old_ceiling));

COBALT_DECL(int, pthread_mutex_getprioceiling,
	    (pthread_mutex_t *__restrict mutex, int *__restrict old_ceiling));

int pthread_emutex_init(pthread_mutex_t *mutex,
			const pthread_emutexattr_t *eattr);

#ifdef __cplusplus
}
#endif

#endif /* _EVL_COBALT_MUTEX_H */
