/*
 * Terminology:
 *
 *	cpuset	- (libc) cpu_set_t data structure represents set of CPUs
 *	cpumask	- string with hex mask (e.g. "0x00000001")
 *	cpulist - string with CPU ranges (e.g. "0-3,5,7,8")
 *
 * Based on code from taskset.c and Linux kernel.
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2010 Karel Zak <kzak@redhat.com>
 */
#ifndef PARSE_H
#define PARSE_H
#include <stdio.h>
#include <string.h>

static const char *nexttoken(const char *q,  int sep)
{
	if (q)
		q = strchr(q, sep);
	if (q)
		q++;
	return q;
}

static int cpulist_parse(const char *str, char *mask)
{
	const char *p, *q;
	int r = 0;

	q = str;

	while (p = q, q = nexttoken(q, ','), p) {
		unsigned int a;	/* beginning of range */
		unsigned int b;	/* end of range */
		unsigned int s;	/* stride */
		const char *c1, *c2;
		char c;

		if ((r = sscanf(p, "%u%c", &a, &c)) < 1)
			return 1;
		b = a;
		s = 1;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');
		if (c1 != NULL && (c2 == NULL || c1 < c2)) {
			if ((r = sscanf(c1, "%u%c", &b, &c)) < 1)
				return 1;
			c1 = nexttoken(c1, ':');
			if (c1 != NULL && (c2 == NULL || c1 < c2)) {
				if ((r = sscanf(c1, "%u%c", &s, &c)) < 1)
					return 1;
				if (s == 0)
					return 1;
			}
		}

		if (!(a <= b))
			return 1;
		while (a <= b) {
			mask[a] = '1';
			a += s;
		}
	}
	
	if (r == 2)
		return 1;
	return 0;
}
#endif
