/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#include <errno.h>
#include <cobalt/pthread.h>

COBALT_IMPL(int, pthread_mutex_init,
	    (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr))
{
	return 0;
}

COBALT_IMPL(int, pthread_mutex_destroy, (pthread_mutex_t *mutex))
{
	return 0;
}

COBALT_IMPL(int, pthread_mutex_lock, (pthread_mutex_t *mutex))
{
	return 0;
}

COBALT_IMPL(int, pthread_mutex_timedlock,
	(pthread_mutex_t *mutex, const struct timespec *to))
{
	return 0;
}

COBALT_IMPL(int, pthread_mutex_trylock, (pthread_mutex_t *mutex))
{
	return 0;
}

COBALT_IMPL(int, pthread_mutex_unlock, (pthread_mutex_t *mutex))
{
	return 0;
}

COBALT_IMPL(int, pthread_mutex_setprioceiling,
	    (pthread_mutex_t *__restrict mutex, int prioceiling,
	     int *__restrict old_ceiling))
{
	return 0;
}

COBALT_IMPL(int, pthread_mutex_getprioceiling,
	    (pthread_mutex_t *__restrict mutex, int *__restrict old_ceiling))
{
	return 0;
}

int pthread_emutex_init(
	pthread_mutex_t *mutex,
	const pthread_emutexattr_t *eattr)
{
	return 0;
}
