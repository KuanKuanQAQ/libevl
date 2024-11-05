/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#ifndef _EVL_POSIX_INTERNAL_H
#define _EVL_POSIX_INTERNAL_H

#include <pthread.h>
#include <evl/mutex.h>

struct __evl_cobalt_mutex {
	union {
		struct evl_mutex evl;
		pthread_mutex_t std;
	};
};

#define __evl_from_std(__ptr, __type)	\
	(&container_of(__ptr, __type, std)->evl)

int __evl_cobalt_init(void);

#endif /* _EVL_POSIX_INTERNAL_H */
