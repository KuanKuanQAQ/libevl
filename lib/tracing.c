/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 Philippe Gerum  <rpm@xenomai.org>
 */

#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <evl/syscall.h>
#include <evl/tracing.h>
#include "internal.h"

int evl_open_ktracer(int _flags)
{
	int fd;

	fd = open(EVL_TRACE_DEV, O_WRONLY);
	if (fd < 0)
		return -errno;

	return fd;
}

int evl_close_ktracer(int fd)
{
	int ret;

	ret = close(fd);
	if (ret < 0)
		return -errno;

	return 0;
}

int evl_write_ktrace(int fd, const char *buf, size_t len)
{
	ssize_t ret;

	ret = __evl_transparent_call(fd, write, buf, len);
	if (ret < 0)
		return -errno;

	return ret;
}

int evl_print_ktrace(int fd, const char *fmt, ...)
{
	ssize_t len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(__evl_local_fmtbuf, sizeof(__evl_local_fmtbuf), fmt, ap);
	va_end(ap);

	if (len < 0)
		return -errno;	/* assume POSIX behavior. */

	return evl_write_ktrace(fd, __evl_local_fmtbuf, len);
}

int evl_freeze_ktrace(int fd)
{
	long ret;

	ret = __evl_transparent_call(fd, ioctl, EVL_TRCIOC_SNAPSHOT);
	if (ret < 0)
		return -errno;

	return 0;
}
