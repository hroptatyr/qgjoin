/*** qgjoin.c -- q-gram joining
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

static uint_fast32_t
hash2(const char s[static 2U], size_t UNUSED(z))
{
	uint_fast32_t res = 0U;
	uint_fast8_t c0 = 0U;
	uint_fast8_t c1 = 0U;

	/* bit of string massage */
	if (s[0U] >= ' ') {
		if ((c0 = s[0U]) >= '`') {
			c0 -= ' ';
		}
		c0 -= ' ';
	}
	/* again */
	if (s[1U] >= ' ') {
		if ((c1 = s[1U]) >= '`') {
			c1 -= ' ';
		}
		c1 -= ' ';
	}
	res = c0 << 4U ^ c1;
	return res;
}

static uint_fast32_t
hash5(const char *s, size_t z)
{
	static const uint_fast8_t tbl[256U] = {
		[' '] = 31,
		['-'] = 31,
		['_'] = 31,
		['0'] = 'O' - '@',
		['1'] = 'I' - '@',
		['2'] = 'Z' - '@',
		['3'] = 27,
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
	uint_fast32_t res = 0U;

	for (size_t i = 0U, j = 0U; i < z && j < 5U; i++) {
		const uint_fast8_t h = tbl[(unsigned char)s[i]];

		if (h) {
			res <<= 4U;
			res ^= h;
			j++;
		}
	}
	return res;
}


typedef size_t factor_t;

static char *pool;
static size_t npool;
static size_t zpool;
static factor_t ipool;
#define nfactor	ipool
static size_t *poff;
static size_t zpoff;

static factor_t *qgrams[1U << 21U];
static size_t zqgrams[1U << 21U];
static size_t nqgrams[1U << 21U];

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
bang(uint_fast32_t h, factor_t f)
{
	if (UNLIKELY(nqgrams[h] >= zqgrams[h])) {
		zqgrams[h] = (zqgrams[h] * 2U) ?: 64U;
		qgrams[h] = realloc(qgrams[h], zqgrams[h] * sizeof(*qgrams[h]));
	}
	qgrams[h][nqgrams[h]++] = f;
	return 0;
}

static size_t
lstrk(uint_fast64_t x)
{
/* find longest streak of ones in X */
	size_t n, i;

	x >>= __builtin_ctzll(x);
	if (UNLIKELY((n = i = __builtin_ctzll(~x)) >= 64)) {
		return 64U;
	}
	x >>= i;
	do {
		x >>= __builtin_ctzll(x);
		i = __builtin_ctzll(~x);
		n = n > i ? n : i;
	} while ((x >>= i));
	return n;
}


#include "qgjoin.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}

	FILE *fp1, *fp2;
	if (UNLIKELY((fp1 = fopen(argi->args[0U], "r")) == NULL)) {
		rc = 1;
		goto out;
	} else if (UNLIKELY((fp2 = fopen(argi->args[1U], "r")) == NULL)) {
		fp2 = stdin;
	}

	char *line = NULL;
	size_t llen = 0U;
	ssize_t nrd;
	while ((nrd = getline(&line, &llen, fp1)) > 0) {
		nrd -= line[nrd - 1U] == '\n';
		line[nrd] = '\0';

		/* intern */
		factor_t f = intern(line, nrd);

		for (size_t i = 0U; (ssize_t)(i + 5U) <= nrd; i++) {
			/* build a 5-gram */
			uint_fast32_t x = hash5(line + i, nrd - i);

			/* store */
			bang(x, f);
		}
	}
	/* proceed with fp2 */
	fclose(fp1);

	while ((nrd = getline(&line, &llen, fp2)) > 0) {
		uint_fast64_t qc[nfactor];
		uint_fast8_t qs[nfactor];
		uint_fast64_t w;
		size_t nq;

		nrd -= line[nrd - 1U] == '\n';
		line[nrd] = '\0';

		memset(qc, 0, sizeof(qc));
		memset(qs, 0, sizeof(qs));
		w = 1U;
		nq = 0U;
		for (size_t i = 0U; (ssize_t)(i + 5U) <= nrd; i++, w <<= 1U) {
			/* build a 5-gram */
			uint_fast32_t x = hash5(line + i, nrd - i);
			/* look up factors in global qgram array */
			for (size_t j = 0U; j < nqgrams[x]; j++) {
				qc[qgrams[x][j] - 1U] |= w;
			}
			nq += nqgrams[x];
		}

		/* find longest streaks */
		for (size_t i = 0U; i < countof(qc); i++) {
			if (UNLIKELY(qc[i])) {
				qs[i] = lstrk(qc[i]);
			}
		}

		/* longest longest streaks */
		size_t max = 0U;
		for (size_t i = 0U; i < countof(qs); i++) {
			max = max > qs[i] ? max : qs[i];
		}

		const double nom = (double)__builtin_ctzll(w);
		double sco = (double)max / nom;
		double ref = nom / sqrt((double)nq);

		if (max-- < 3U || sco + ref < 1) {
			continue;
		}

		for (size_t i = 0U; i < countof(qs); i++) {
			if (UNLIKELY(qs[i] >= max)) {
				size_t plen = poff[i + 1U] - poff[i];

				fwrite(pool + poff[i], 1, plen, stdout);
				fputc('\t', stdout);
				fwrite(line, 1, nrd, stdout);
				fputc('\t', stdout);
				fprintf(stdout, "%zu", max);
				fputc('\n', stdout);
			}
		}
	}
	fclose(fp2);
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

/* qgjoin.c ends here */
