/*** cprfx.c -- collapse lines by prefixes
 *
 * Copyright (C) 2016-2017 Sebastian Freundt
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#if defined HAVE_DFP754_H
# include <dfp754.h>
#endif	/* HAVE_DFP754_H */
#include "nifty.h"

static size_t thresh = 1U;


static size_t
prnt(const char *str, size_t len, size_t *restrict strk, size_t n)
{
	size_t s = 0U;

	for (size_t j = len; j > n; j--) {
		if (strk[j] <= thresh) {
			continue;
		}
		fwrite(str, 1, j, stdout);
		fputc('\t', stdout);
		fprintf(stdout, "%zu", s += strk[j]);
		fputc('\n', stdout);
		strk[j] = 0U;
	}
	return s;
}

static int
cprfx(FILE *fp)
{
	char *line = NULL;
	size_t llen = 0UL;
	char *prev = NULL;
	size_t plen = 0UL;
	size_t prrd = 0UL;
	size_t *strk = NULL, zstrk = 0U;

	for (ssize_t nrd; (nrd = getline(&line, &llen, fp)) > 0;) {
		size_t i;

		nrd -= line[nrd - 1] == '\n';
		line[nrd] = '\0';

		for (i = 0U; i < prrd &&
			     i < (size_t)nrd && prev[i] == line[i]; i++);

		if (UNLIKELY((size_t)nrd >= zstrk)) {
			size_t nuz = (zstrk * 2U) ?: 256U;
			while (nuz <= (size_t)nrd) {
				nuz *= 2U;
			}
			strk = realloc(strk, nuz * sizeof(*strk));
			memset(strk + zstrk, 0, (nuz - zstrk) * sizeof(*strk));
			zstrk = nuz;
		}

		/* print the old stuff */
		with (size_t s = prnt(prev, prrd, strk, i)) {
			/* start counting anew */
			strk[nrd]++;
			strk[i] += s;
		}

		/* save line for next time */
		if (UNLIKELY(plen < llen)) {
			prev = realloc(prev, plen = llen);
		}
		memcpy(prev + i, line + i, (prrd = nrd) - i);
	}
	/* last one then */
	prnt(prev, prrd, strk, 0U);

	free(line);
	if (prev) {
		free(prev);
	}
	if (strk) {
		free(strk);
	}
	return 0;
}


#include "cprfx.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc;

	if (yuck_parse(argi, argc, argv) < 0) {
		rc = 1;
		goto out;
	}

	thresh -= !!argi->verbose_flag;

	rc = cprfx(stdin) < 0;

out:
	yuck_free(argi);
	return rc;
}

/* cprfx.c ends here */
