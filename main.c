/*
Paweł Wódkowski
All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>

#include "lpm.h"
#include "utils.h"

#define MAX_IPS 100

enum {
	ADD,
	DEL
};
static void
test_add_del(int do_add,
             const unsigned *prefixes, char len, size_t num_prefixes, int exp_rc)
{
	for (size_t i = 0; i < num_prefixes; i++) {
		unsigned prefix = prefixes[i];

		int rc = do_add == ADD ? add(prefix, len) : del(prefix, len);
		if (rc != exp_rc) {
			fprintf(stderr,
					"Unexpected %s() result: expected %d, actual: %d\n"
			        "	prefix[%zu]: %08x/%d (netmask: %08x)\n",
			        do_add == ADD ? "add" : "del",
					exp_rc, rc,
					i, prefix, len, IP4_NETMASK(len));
			exit(1);
		}
	}
}

static void
test_add(unsigned pref, char len, int exp_rc)
{
	return test_add_del(ADD, &pref, len, 1, exp_rc);
}

static void
test_del(unsigned pref, char len, int exp_rc)
{
	return test_add_del(DEL, &pref, len, 1, exp_rc);
}

static void
test_check_ip(unsigned ip, int exp_rc)
{
	int rc = check(ip);
	if (rc != exp_rc) {
		fprintf(stderr,
				"Unexpected check() result - expected %d, actual: %d\n"
				"	ip: %08x\n",
				exp_rc, rc,
				ip);
		exit(1);
	}
}

static void
test_check(unsigned prefix, char len, int exp_rc)
{
	unsigned netmask = IP4_NETMASK(len);
	unsigned num_ips = ~netmask <= MAX_IPS ? ~netmask : MAX_IPS;

	for (size_t j = 0; j < num_ips; j++) {
		unsigned ip = prefix | j;
		int rc = check(ip);

		if (rc != exp_rc) {
			fprintf(stderr,
					"Unexpected check() result - expected %d, actual: %d\n"
					"	prefix: %08x/%d (netmask: %8x), ip: %08x,\n",
					exp_rc, rc,
					prefix, len, netmask, ip);
			exit(1);
		}
	}
}

#define IP4_PREFIX(_ip, _len) ((_ip) & IP4_NETMASK(_len))

int
main(void)
{
	const unsigned ips[] = {
		IP4(0xFE, 0xA0, 0xFF, 0xFF),
		IP4(0xF1, 0xA1, 0xFF, 0xFF),
		IP4(0xF7, 0xA2, 0xFF, 0xFF),
	};

	char len;

	/* Invalid prefix */
	unsigned prefix = IP4(10,0,0,1);
	test_add(prefix, 5, -1);
	test_del(prefix, 5, -1);

	/* OK */
	for (len = 0; len < 32; len++) {
		prefix = IP4_PREFIX(ips[0], len);

		test_add(prefix, len, 0);

		test_del(prefix, len, 0);
		test_del(prefix, len, -1);

		/* Can add/del agin? */
		test_add(prefix, len, 0);

		test_del(prefix, len, 0);
		test_del(prefix, len, -1);
	}

	len = 10;

	/* The same two subnets with different prefixes */
	test_add(prefix, len, 0);

	test_add(IP4_PREFIX(ips[0], 10), 10, 0);
	test_add(IP4_PREFIX(ips[1], 10), 10, 0);
	test_add(IP4_PREFIX(ips[0], 11), 11, 0);
	test_add(IP4_PREFIX(ips[1], 11), 11, 0);

	/* del() try to delete third prefix */
	test_del(IP4_PREFIX(ips[2], 10), 10, -1);
	test_del(IP4_PREFIX(ips[2], 11), 11, -1);

	/* Lowest mask is 11. */
	test_check_ip(ips[0], 11);

	/* Move to different subnet */
	unsigned ip  = ips[0] + (1u << 12);
	test_check_ip(ip, -1);

	/* Find IPs from subnet /11 */
	test_check(ips[0], 11, 11);

	/* Del subnet /11 for first IP. */
	test_del(IP4_PREFIX(ips[0], 11), 11, 0);

	/* Now it must be /10 */
	test_check_ip(ips[0], 10);

	test_del(IP4_PREFIX(ips[1], 11), 11, 0);

	test_del(IP4_PREFIX(ips[0], 10), 10, 0);
	test_del(IP4_PREFIX(ips[1], 10), 10, 0);
	/* etc... */

	printf("Success\n");
	return 0;
}
