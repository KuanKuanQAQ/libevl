/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#include <assert.h>
#include <pthread.h>
#include "internal.h"

/*
 * FIXME: handle NDEBUG globally on the meson side.
 */
int __evl_cobalt_init(void)
{
	assert(sizeof(struct __evl_cobalt_mutex) <= sizeof(pthread_mutex_t));

	return 0;
}
