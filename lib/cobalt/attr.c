/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#include <memory.h>
#include <errno.h>
#include <evl/thread.h>
#include <cobalt/pthread.h>
#include "internal.h"

COBALT_IMPL(int, pthread_attr_init, (pthread_attr_t *attr))
{
	pthread_attr_init(attr);
	return pthread_attr_setstacksize(attr, EVL_STACK_DEFAULT);
}

int pthread_eattr_init(pthread_eattr_t *eattr)
{
	struct sched_param param;
	int policy;

	/*
	 * First we set up a default attribute set using our
	 * implementation of the standard POSIX call, next we fill the
	 * extended scheduling parameters with the policy and priority
	 * values such call picked. Normally, this should be
	 * (SCHED_OTHER,0).
	 */
	__COBALT(pthread_attr_init)(&eattr->std);
	memset(&eattr->evl, 0, sizeof(eattr->evl));
	pthread_attr_getschedpolicy(&eattr->std, &policy);
	pthread_attr_getschedparam(&eattr->std, &param);
	eattr->evl.sched_policy = policy;
	eattr->evl.sched_priority = param.sched_priority;

	return 0;
}

int pthread_eattr_destroy(pthread_eattr_t *eattr)
{
	return pthread_attr_destroy(&eattr->std);
}

int pthread_eattr_setschedpolicy(
	pthread_eattr_t *eattr,
	int policy)
{
	if (policy < 0)
		return EINVAL;

	/* Leave the actual check to the core. */
	eattr->evl.sched_policy = policy;

	return 0;
}

int pthread_eattr_getschedpolicy(
	const pthread_eattr_t *__restrict eattr,
	int *policy)
{
	*policy = eattr->evl.sched_policy;

	return 0;
}

int pthread_eattr_setschedparam(
	pthread_eattr_t *__restrict eattr,
	const struct sched_eparam *__restrict eparam)
{
	/* Leave the actual check to the core. */
	eattr->evl = eparam->evl;

	return 0;
}

int pthread_eattr_getschedparam(
	const pthread_eattr_t *__restrict eattr,
	struct sched_eparam *__restrict eparam)
{
	eparam->evl = eattr->evl;

	return 0;
}

int pthread_eattr_setinheritsched(
	pthread_eattr_t *eattr,
	int inheritsched)
{
	pthread_attr_setinheritsched(&eattr->std, inheritsched);

	return 0;
}

int pthread_eattr_getinheritsched(
	const pthread_eattr_t *__restrict eattr,
	int *__restrict inheritsched)
{
	return pthread_attr_getinheritsched(&eattr->std, inheritsched);
}

int pthread_eattr_setdetachstate(
	pthread_eattr_t *eattr,
	int detachstate)
{
	pthread_attr_setdetachstate(&eattr->std, detachstate);

	return 0;
}

int pthread_eattr_getdetachstate(
	const pthread_eattr_t *eattr,
	int *detachstate)
{
	return pthread_attr_getdetachstate(&eattr->std, detachstate);
}

int pthread_eattr_setstacksize(
	pthread_eattr_t *eattr,
	size_t stacksize)
{
	pthread_attr_setstacksize(&eattr->std, stacksize);

	return 0;
}

int pthread_eattr_getstacksize(
	const pthread_eattr_t *__restrict eattr,
	size_t *__restrict stacksize)
{
	pthread_attr_getstacksize(&eattr->std, stacksize);

	return 0;
}

int pthread_eattr_setscope(
	pthread_eattr_t *eattr,
	int scope)
{
	pthread_attr_setscope(&eattr->std, scope);

	return 0;
}

int pthread_eattr_getscope(
	const pthread_eattr_t *__restrict eattr,
	int *__restrict scope)
{
	pthread_attr_getscope(&eattr->std, scope);

	return 0;
}
