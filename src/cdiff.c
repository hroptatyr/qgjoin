/*** cdiff.c -- cross out bits which are identical to the previous line
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


static int
cdiff(FILE *fp)
{
	char *line = NULL;
	size_t llen = 0UL;
	char *prev = NULL;
	size_t plen = 0UL;

	for (ssize_t nrd; (nrd = getline(&line, &llen, stdin)) > 0;) {
		size_t i;

		if (LIKELY(line[nrd - 1U] == '\n')) {
			nrd--;
		}
		for (i = 0U; i < plen && i < nrd && prev[i] == line[i]; i++) {
			putchar(' ');
		}
		if (LIKELY(i < nrd)) {
			fwrite(line + i, 1, nrd - i, stdout);
		}
		putchar('\n');

		/* save line for next time */
		if (UNLIKELY(plen < nrd)) {
			prev = realloc(prev, plen = nrd);
		}
		memcpy(prev, line, nrd);
	}
	free(line);
	return 0;
}


#include "cdiff.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc;

	if (yuck_parse(argi, argc, argv) < 0) {
		rc = 1;
		goto out;
	}

	rc = cdiff(stdin) < 0;

out:
	yuck_free(argi);
	return rc;
}

/* cdiff.c ends here */
