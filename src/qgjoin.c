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

typedef uint_fast32_t qgram_t;

#if !defined ILEAVE
# define ILEAVE		4U
#elif ILEAVE < 3U || ILEAVE > 5U
# error ILEAVE must be 3, 4 or 5
#endif


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
	qgram_t x = 0U;
	size_t n = 0U;
	size_t condens;
	size_t i, j;

	for (i = 0U, j = 0U, condens = 1U; i < z && j < 5U; i++) {
		const int_fast8_t h = tbl[(unsigned char)s[i]];

		if (h > 0 || !condens) {
			x <<= ILEAVE;
			x ^= h & 0b11111U;
			j++;
		}
		condens = h < 0;
	}
	x &= (1U << ILEAVE * 5 + (ILEAVE < 5)) - 1U;
	r[n] = x;
	n += !!x;
	/* keep going */
	for (; i < z; i++) {
		const int_fast8_t h = tbl[(unsigned char)s[i]];

#if ILEAVE == 3
		x ^= x & 0b1111000000000000U;
#elif ILEAVE == 4
		x ^= x & 0b111110000000000000000U;
#elif ILEAVE == 5
		x ^= x & 0b1111100000000000000000000U;
#endif
		if (h > 0 || !condens) {
			x <<= ILEAVE;
			x ^= h & 0b11111U;
			j++;
		}
		condens = h < 0;
		r[n] = x;
		n += h > 0 || !condens;
	}
	return n;
}


typedef size_t factor_t;

static char *pool;
static size_t npool;
static size_t zpool;
static factor_t ipool;
#define nfactor	ipool
static size_t *poff;
static size_t zpoff;

static factor_t *qgrams[1U << (ILEAVE * 5 + (ILEAVE < 5))];
static size_t zqgrams[1U << (ILEAVE * 5 + (ILEAVE < 5))];
static size_t nqgrams[1U << (ILEAVE * 5 + (ILEAVE < 5))];

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

static size_t
lstrk(uint_fast64_t x)
{
/* find longest streak of ones in X */
	size_t n, i;

	if (LIKELY(!x)) {
		return 0;
	} else if (UNLIKELY(!~x)) {
		return 64U;
	}

	x >>= __builtin_ctzll(x);
	n = i = __builtin_ctzll(~x);
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
	FILE *fp1, *fp2;
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}

	if (!argi->nargs) {
		errno = 0, error("\
Error: left input file not given");
		rc = 1;
		goto out;
	} else if (UNLIKELY((fp1 = fopen(argi->args[0U], "r")) == NULL)) {
		error("\
Error: cannot open left input file");
		rc = 1;
		goto out;
	} else if (argi->nargs == 1U) {
		fp2 = stdin;
	} else if (UNLIKELY((fp2 = fopen(argi->args[1U], "r")) == NULL)) {
		error("\
Error: cannot open right input file");
		rc = 1;
		fclose(fp1);
		goto out;
	}

	char *line = NULL;
	size_t llen = 0U;
	ssize_t nrd;
	while ((nrd = getline(&line, &llen, fp1)) > 0) {
		nrd -= line[nrd - 1U] == '\n';
		line[nrd] = '\0';

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
	fclose(fp1);

	/* for streak track-keeping */
	static size_t *strk;
	static size_t zstrk;

	uint_fast64_t *qc = malloc(nfactor * sizeof(*qc));
	uint_fast64_t *cc = malloc(((nfactor / 64U) + 1U) * sizeof(*cc));

	while ((nrd = getline(&line, &llen, fp2)) > 0) {
		uint_fast64_t w;
		size_t nq;

		nrd -= line[nrd - 1U] == '\n';
		line[nrd] = '\0';

		memset(qc, 0, nfactor * sizeof(*qc));
		memset(cc, 0, ((nfactor / 64U) + 1U) * sizeof(*cc));
		w = 1U;
		nq = 0U;

		/* build all 5-grams */
		qgram_t x[nrd - 5U + 1U];
		const size_t n = mkqgrams(x, line, nrd);

		for (size_t i = 0U; i < n; i++) {
			/* look up factors in global qgram array */
			const qgram_t y = x[i];
			for (size_t j = 0U; j < nqgrams[y]; j++) {
				qc[qgrams[y][j] - 1U] |= w;
			}
			nq += nqgrams[y];
			w <<= 1U;
		}

		/* make up candidates */
		for (size_t i = 0U; i < n; i++) {
			/* look up factors in global qgram array */
			const qgram_t y = x[i];
			for (size_t j = 0U; j < nqgrams[y]; j++) {
				const size_t k = qgrams[y][j] - 1U;
				cc[k / 64U] |= (uint_fast64_t)(1ULL << k % 64U);
			}
		}

		/* find longest longest streaks */
		size_t max = 3U;
		size_t nstrk = 0U;

		for (size_t i = 0U; i <= nfactor / 64U; i++) {
			for (uint_fast64_t c = cc[i], j = 0U; c; c >>= 1U, j++) {
				const size_t k = 64U * i + j;
				size_t s;

				if (LIKELY(!(c & 0b1U))) {
					continue;
				} else if (LIKELY((s = lstrk(qc[k])) < max)) {
					/* nothing to see here */
					continue;
				} else if (UNLIKELY(s > max)) {
					max = s;
					nstrk = 0U;
				}
				/* append to streak array */
				if (UNLIKELY(nstrk >= zstrk)) {
					zstrk = (zstrk * 2U) ?: 16U;
					strk = realloc(
						strk, zstrk * sizeof(*strk));
				}
				strk[nstrk++] = k;
			}
		}

		const double nom = (double)__builtin_ctzll(w);
		double sco = (double)max / nom;
		double ref = nom / sqrt((double)nq);

		if (max-- < 3U || sco + ref < 1) {
			continue;
		}

		for (size_t j = 0U; j < nstrk; j++) {
			const size_t i = strk[j];
			size_t plen = poff[i + 1U] - poff[i];

			fwrite(pool + poff[i], 1, plen, stdout);
			fputc('\t', stdout);
			fwrite(line, 1, nrd, stdout);
			fputc('\t', stdout);
			fprintf(stdout, "%zu", max);
			fputc('\n', stdout);
		}
	}
	fclose(fp2);
	free(line);

	free(qc);
	free(cc);

	if (zstrk) {
		free(strk);
	}

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
