/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 Philippe Gerum  <rpm@xenomai.org>
 */

#ifndef _EVL_COBALT_WRAPPERS_H
#define _EVL_COBALT_WRAPPERS_H

#include <evl/compiler.h>

/*
 * For each function() from lib/cobalt which implements a standard
 * POSIX service, the following symbols are visible:
 *
 *          function()  => original POSIX implementation (from glibc/musl/...).
 *   cobalt_function() => libevl/cobalt implementation of function().
 */
#define __COBALT(__call)	cobalt_ ## __call
#define __RT(__call)		__COBALT(__call)
#define __EVL(__call)		__COBALT(__call)

#define COBALT_DECL(__type, __name, __arglist)		\
	__typeof__(__type) cobalt_ ## __name __arglist

#define COBALT_IMPL(__type, __name, __arglist)		\
	__typeof__(__type) cobalt_ ## __name __arglist

#endif /* _EVL_COBALT_WRAPPERS_H */
