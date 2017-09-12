/*** qgstat.c -- q-gram stats
 *
 * Copyright (C) 2015-2017 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of qgjoin.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "nifty.h"

typedef uint_fast32_t qgram_t;


static void
__attribute__((format(printf, 1, 2)))
error(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return;
}

static size_t
mkqgrams(qgram_t *restrict r, const char *s, size_t z)
{
/* build all qgrams from S of length Z and store in R */
	static const int_fast8_t tbl[256U] = {
		[' '] = -1,
		['-'] = -1,
		['_'] = -1,
		['0'] = 'O' - '@',
		['1'] = 'I' - '@',
		['2'] = 'Z' - '@',
		['3'] = -2,
		['4'] = 'A' - '@',
		['5'] = 'S' - '@',
		['6'] = 'G' - '@',
		['7'] = 'T' - '@',
		['8'] = 'B' - '@',
		['9'] = 'Q' - '@',
		['A'] = 'A' - '@',
		['B'] = 'B' - '@',
		['C'] = 'C' - '@',
		['D'] = 'D' - '@',
		['E'] = 'E' - '@',
		['F'] = 'F' - '@',
		['G'] = 'G' - '@',
		['H'] = 'H' - '@',
		['I'] = 'I' - '@',
		['J'] = 'J' - '@',
		['K'] = 'K' - '@',
		['L'] = 'L' - '@',
		['M'] = 'M' - '@',
		['N'] = 'N' - '@',
		['O'] = 'O' - '@',
		['P'] = 'P' - '@',
		['Q'] = 'Q' - '@',
		['R'] = 'R' - '@',
		['S'] = 'S' - '@',
		['T'] = 'T' - '@',
		['U'] = 'U' - '@',
		['V'] = 'V' - '@',
		['W'] = 'W' - '@',
		['X'] = 'X' - '@',
		['Y'] = 'Y' - '@',
		['Z'] = 'Z' - '@',
		['a'] = 'A' - '@',
		['b'] = 'B' - '@',
		['c'] = 'C' - '@',
		['d'] = 'D' - '@',
		['e'] = 'E' - '@',
		['f'] = 'F' - '@',
		['g'] = 'G' - '@',
		['h'] = 'H' - '@',
		['i'] = 'I' - '@',
		['j'] = 'J' - '@',
		['k'] = 'K' - '@',
		['l'] = 'L' - '@',
		['m'] = 'M' - '@',
		['n'] = 'N' - '@',
		['o'] = 'O' - '@',
		['p'] = 'P' - '@',
		['q'] = 'Q' - '@',
		['r'] = 'R' - '@',
		['s'] = 'S' - '@',
		['t'] = 'T' - '@',
		['u'] = 'U' - '@',
		['v'] = 'V' - '@',
		['w'] = 'W' - '@',
		['x'] = 'X' - '@',
		['y'] = 'Y' - '@',
		['z'] = 'Z' - '@',
	};
	qgram_t x = 0U;
	size_t n = 0U;
	size_t condens;
	size_t i, j;

	for (i = 0U, j = 0U, condens = 1U; i < z && j < 5U; i++) {
		const int_fast8_t h = tbl[(unsigned char)s[i]];

		if (h > 0 || !condens) {
			x <<= 5;
			x ^= h & 0b11111U;
			j++;
		}
		condens = h < 0;
	}
	x &= (1U << 25) - 1U;
	if (r) {
		r[n] = x;
	}
	n += !!x;
	/* keep going */
	for (; i < z; i++) {
		const int_fast8_t h = tbl[(unsigned char)s[i]];

		x ^= x & 0b1111100000000000000000000U;
		if (h > 0 || !condens) {
			x <<= 5;
			x ^= h & 0b11111U;
			j++;
		}
		condens = h < 0;
		if (r) {
			r[n] = x;
		}
		n += h > 0 || !condens;
	}
	return n;
}

static const char*
mkstring(qgram_t q)
{
	static char buf[6U];
	for (size_t i = 5U; i-- > 0U; q >>= 5U) {
		buf[i] = (q & 0b11111U) + '@';
	}
	return buf;
}


typedef size_t factor_t;

static char *pool;
static size_t npool;
static size_t zpool;
static factor_t ipool;
#define nfactor	ipool
static size_t *poff;
static size_t zpoff;

static factor_t *qgrams[1U << 25U];
static size_t zqgrams[1U << 25U];
static size_t nqgrams[1U << 25U];

static factor_t
intern(const char *str, size_t len)
{
	factor_t r = 0U;

	if (UNLIKELY(npool + len >= zpool)) {
		zpool = (zpool * 2U) ?: 4096U;
		pool = realloc(pool, zpool * sizeof(*pool));
	}
	/* copy */
	memcpy(pool + npool, str, len);
	npool += len;

	if (UNLIKELY(ipool >= zpoff)) {
		zpoff = (zpoff * 2U) ?: 512U;
		poff = realloc(poff, zpoff * sizeof(*poff));
		poff[0U] = 0U;
	}
	poff[r = ++ipool] = npool;
	return r;
}

static int
bang(qgram_t h, factor_t f)
{
	if (UNLIKELY(nqgrams[h] >= zqgrams[h])) {
		zqgrams[h] = (zqgrams[h] * 2U) ?: 64U;
		qgrams[h] = realloc(qgrams[h], zqgrams[h] * sizeof(*qgrams[h]));
	}
	qgrams[h][nqgrams[h]++] = f;
	return 0;
}


#include "qgstat.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}

	char *line = NULL;
	size_t llen = 0U;
	ssize_t nrd;
	while ((nrd = getline(&line, &llen, stdin)) > 0) {
		nrd -= line[nrd - 1U] == '\n';
		line[nrd] = '\0';

		if (UNLIKELY(nrd < 5U)) {
			continue;
		}

		/* intern */
		factor_t f = intern(line, nrd);
		/* build all 5-grams */
		qgram_t x[nrd - 5U + 1];
		const size_t n = mkqgrams(x, line, nrd);

		for (size_t i = 0U; i < n; i++) {
			/* store */
			bang(x[i], f);
		}
	}
	/* proceed with fp2 */
	fclose(stdin);

	for (size_t i = 0U; i < countof(qgrams); i++) {
		if (nqgrams[i]) {
			const char *x = mkstring(i);
			printf("%s\t%zu\n", x, nqgrams[i]);
		}
	}
	free(line);

	for (size_t i = 0U; i < countof(qgrams); i++) {
		if (qgrams[i]) {
			free(qgrams[i]);
		}
	}
	free(pool);
	free(poff);

out:
	yuck_free(argi);
	return rc;
}

/* qgstat.c ends here */
