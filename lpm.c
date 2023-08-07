/*
 * All rights reserved.
 * Author: Paweł Wódkowski
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define LPM_BUCKET_SIZE 128

struct lpm_range {
	size_t idx;

	size_t len;
	size_t capacity;
	uint32_t *bases;
};

struct lpm_range g_ranges[33];

static inline int
prefix_cmp(struct lpm_range *r, size_t idx, uint32_t prefix)
{
	return prefix - r->bases[idx];
}

static int
find(struct lpm_range *r, uint32_t prefix)
{
	/* Trivial cases. */
	if (r->len == 0 || prefix_cmp(r, 0, prefix) < 0) {
		r->idx = 0;
		return -1;
	} else if (prefix_cmp(r, r->len - 1, prefix) > 0) {
		r->idx = r->len;
		return -1;
	}

	size_t left = 0;
	size_t right = r->len - 1;
	size_t m;

	while (right != left) {
		m = left + right;
		m = m / 2 + (m & 1); /* ceil(m/2) */

		int cmp = prefix_cmp(r, m, prefix);
		if (cmp < 0)
			right = m - 1;
		else
			left = m;
	}

	r->idx = left;
	return prefix_cmp(r, left, prefix);
}

static int
check_capacity(struct lpm_range *r, size_t new_len)
{
	size_t new_cap = r->capacity;

	if (new_len > new_cap)
		new_cap += LPM_BUCKET_SIZE;
	else if(new_len + LPM_BUCKET_SIZE <= new_cap)
		new_cap -= LPM_BUCKET_SIZE;

	if (new_cap == r->capacity)
		return 0;
	else if (new_cap > 0) {
		void *ptr = realloc(r->bases, new_cap * sizeof(r->bases[0]));
		if (!ptr)
			return -1;

		r->capacity = new_cap;
		r->bases = ptr;
	}

	return 0;
}

static int
add_at(struct lpm_range *r, uint32_t base)
{
	if (check_capacity(r, r->len + 1))
		return -1;

	size_t move_cnt = r->len - r->idx;
	memmove(&r->bases[r->idx + 1], &r->bases[r->idx], move_cnt * sizeof(r->bases[0]));
	r->bases[r->idx] = base;
	r->len++;
	return 0;
}


static void
del_at(struct lpm_range *r)
{
	size_t move_cnt = r->len - (r->idx + 1);
	memmove(&r->bases[r->idx], &r->bases[r->idx + 1], move_cnt * sizeof(r->bases[0]));
	r->len--;

	check_capacity(r, r->len);
}

int
add(unsigned int base, char mask)
{
	struct lpm_range *r;

	if (mask < 0 || mask > 32)
		return -1;

	uint32_t netmask = IP4_NETMASK(mask);
	if (mask > 0 && (base & ~netmask) != 0)
		return -1;

	r = &g_ranges[(size_t)mask];
	if (find(r, base) != 0)
		return add_at(r, base);

	return 0;
}

int
del(unsigned int base, char mask)
{
	struct lpm_range *r;

	if (mask < 0 || mask > 32)
		return -1;

	uint32_t netmask = IP4_NETMASK(mask);
	if (mask > 0 && (base & ~netmask) != 0)
		return -1;

	r = &g_ranges[(size_t)mask];
	if (find(r, base) != 0)
		return -1;

	del_at(r);
	return 0;
}

char
check(unsigned int ip)
{
	struct lpm_range *r;
	ssize_t mask;

	for (mask = SIZE(g_ranges) - 1; mask >= 0; mask--) {
		r = &g_ranges[mask];

		if (find(r, ip & IP4_NETMASK(mask)) == 0)
			break;
	}

	return mask;
}
