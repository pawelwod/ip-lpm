/*
 * All rights reserved.
 * Author: Paweł Wódkowski
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>

#define IP4(a, b, c, d) \
	( \
		((UINT32_C(a) & 0xFF) << 24u) | \
		((UINT32_C(b) & 0xFF) << 16u) | \
		((UINT32_C(c) & 0xFF) << 8u) | \
		((UINT32_C(d) & 0xFF) << 0u) \
	)

#define IP4_NETMASK(_len) (~(uint32_t) ((UINT64_C(1) << _len) - 1))
#define SIZE(_x) (sizeof(_x) /sizeof(_x[0]))

#endif
