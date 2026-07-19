/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 Philippe Gerum  <rpm@xenomai.org>
 */

#ifndef _EVL_TRACING_H
#define _EVL_TRACING_H

#include <sys/types.h>
#include <evl/trace-abi.h>

#ifdef __cplusplus
extern "C" {
#endif

int evl_open_ktracer(int flags);

int evl_close_ktracer(int fd);

int evl_print_ktrace(int fd, const char *fmt, ...);

int evl_write_ktrace(int fd, const char *buf, size_t len);

int evl_freeze_ktrace(int fd);

#ifdef __cplusplus
}
#endif

#endif /* _EVL_TRACING_H */
