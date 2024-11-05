/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#pragma GCC system_header
#include_next <sched.h>

#ifndef _EVL_COBALT_SCHED_H
#define _EVL_COBALT_SCHED_H

#include <evl/sched.h>
#include <cobalt/wrappers.h>

struct sched_eparam {
	struct sched_param std;
	struct evl_sched_attrs evl;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _EVL_COBALT_SCHED_H */
