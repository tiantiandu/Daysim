#ifndef lint
static const char	RCSid[] = "$Id$";
#endif
/*
 * Table lookup routines
 */

/* ====================================================================
 * The Radiance Software License, Version 1.0
 *
 * Copyright (c) 1990 - 2002 The Regents of the University of California,
 * through Lawrence Berkeley National Laboratory.   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *           if any, must include the following acknowledgment:
 *             "This product includes Radiance software
 *                 (http://radsite.lbl.gov/)
 *                 developed by the Lawrence Berkeley National Laboratory
 *               (http://www.lbl.gov/)."
 *       Alternately, this acknowledgment may appear in the software itself,
 *       if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Radiance," "Lawrence Berkeley National Laboratory"
 *       and "The Regents of the University of California" must
 *       not be used to endorse or promote products derived from this
 *       software without prior written permission. For written
 *       permission, please contact radiance@radsite.lbl.gov.
 *
 * 5. Products derived from this software may not be called "Radiance",
 *       nor may "Radiance" appear in their name, without prior written
 *       permission of Lawrence Berkeley National Laboratory.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.   IN NO EVENT SHALL Lawrence Berkeley National Laboratory OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of Lawrence Berkeley National Laboratory.   For more
 * information on Lawrence Berkeley National Laboratory, please see
 * <http://www.lbl.gov/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include "lookup.h"

#ifdef	NOSTRUCTASS
#define	 copystruct(d,s)	bcopy((char *)(s),(char *)(d),sizeof(*(d)))
#else
#define	 copystruct(d,s)	(*(d) = *(s))
#endif


int
lu_init(tbl, nel)		/* initialize tbl for at least nel elements */
register LUTAB	*tbl;
int	nel;
{
	static int  hsiztab[] = {
		31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 
		32749, 65521, 131071, 262139, 524287, 1048573, 2097143, 
		4194301, 8388593, 0
	};
	register int  *hsp;

	nel += nel>>1;			/* 66% occupancy */
	for (hsp = hsiztab; *hsp; hsp++)
		if (*hsp > nel)
			break;
	if (!(tbl->tsiz = *hsp))
		tbl->tsiz = nel*2 + 1;		/* not always prime */
	tbl->tabl = (LUENT *)calloc(tbl->tsiz, sizeof(LUENT));
	if (tbl->tabl == NULL)
		tbl->tsiz = 0;
	tbl->ndel = 0;
	return(tbl->tsiz);
}


unsigned long
lu_shash(s)			/* hash a nul-terminated string */
char	*s;
{
	static unsigned char shuffle[256] = {
		0, 157, 58, 215, 116, 17, 174, 75, 232, 133, 34,
		191, 92, 249, 150, 51, 208, 109, 10, 167, 68, 225,
		126, 27, 184, 85, 242, 143, 44, 201, 102, 3, 160,
		61, 218, 119, 20, 177, 78, 235, 136, 37, 194, 95,
		252, 153, 54, 211, 112, 13, 170, 71, 228, 129, 30,
		187, 88, 245, 146, 47, 204, 105, 6, 163, 64, 221,
		122, 23, 180, 81, 238, 139, 40, 197, 98, 255, 156,
		57, 214, 115, 16, 173, 74, 231, 132, 33, 190, 91,
		248, 149, 50, 207, 108, 9, 166, 67, 224, 125, 26,
		183, 84, 241, 142, 43, 200, 101, 2, 159, 60, 217,
		118, 19, 176, 77, 234, 135, 36, 193, 94, 251, 152,
		53, 210, 111, 12, 169, 70, 227, 128, 29, 186, 87,
		244, 145, 46, 203, 104, 5, 162, 63, 220, 121, 22,
		179, 80, 237, 138, 39, 196, 97, 254, 155, 56, 213,
		114, 15, 172, 73, 230, 131, 32, 189, 90, 247, 148,
		49, 206, 107, 8, 165, 66, 223, 124, 25, 182, 83,
		240, 141, 42, 199, 100, 1, 158, 59, 216, 117, 18,
		175, 76, 233, 134, 35, 192, 93, 250, 151, 52, 209,
		110, 11, 168, 69, 226, 127, 28, 185, 86, 243, 144,
		45, 202, 103, 4, 161, 62, 219, 120, 21, 178, 79,
		236, 137, 38, 195, 96, 253, 154, 55, 212, 113, 14,
		171, 72, 229, 130, 31, 188, 89, 246, 147, 48, 205,
		106, 7, 164, 65, 222, 123, 24, 181, 82, 239, 140,
		41, 198, 99
	};
	register int	i = 0;
	register unsigned long	h = 0;
	register unsigned char *t = (unsigned char *)s;

	while (*t)
	    	h ^= (unsigned long)shuffle[*t++] << ((i+=11) & 0xf);

	return(h);
}


LUENT *
lu_find(tbl, key)		/* find a table entry */
register LUTAB	*tbl;
char	*key;
{
	unsigned long	hval;
	int	i, n;
	register int	ndx;
	register LUENT	*le;
					/* look up object */
	if (tbl->tsiz == 0 && !lu_init(tbl, 1))
		return(NULL);
	hval = (*tbl->hashf)(key);
tryagain:
	ndx = hval % tbl->tsiz;
	for (i = 0, n = 1; i < tbl->tsiz; i++, n += 2) {
		le = &tbl->tabl[ndx];
		if (le->key == NULL) {
			le->hval = hval;
			return(le);
		}
		if (le->hval == hval && 
		      (tbl->keycmp == NULL || !(*tbl->keycmp)(le->key, key))) {
			return(le);
		}
		if ((ndx += n) >= tbl->tsiz)	/* this happens rarely */
			ndx = ndx % tbl->tsiz;
	}
					/* table is full, reallocate */
	le = tbl->tabl;
	ndx = tbl->tsiz;
	i = tbl->ndel;
	if (!lu_init(tbl, ndx-i+1)) {	/* no more memory! */
		tbl->tabl = le;
		tbl->tsiz = ndx;
		tbl->ndel = i;
		return(NULL);
	}
	/*
	 * The following code may fail if the user has reclaimed many
	 * deleted entries and the system runs out of memory in a
	 * recursive call to lu_find().
	 */
	while (ndx--)
		if (le[ndx].key != NULL)
			if (le[ndx].data != NULL)
				copystruct(lu_find(tbl,le[ndx].key), &le[ndx]);
			else if (tbl->freek != NULL)
				(*tbl->freek)(le[ndx].key);
	free((void *)le);
	goto tryagain;			/* should happen only once! */
}


void
lu_delete(tbl, key)		/* delete a table entry */
register LUTAB	*tbl;
char	*key;
{
	register LUENT	*le;

	if ((le = lu_find(tbl, key)) == NULL)
		return;
	if (le->key == NULL || le->data == NULL)
		return;
	if (tbl->freed != NULL)
		(*tbl->freed)(le->data);
	le->data = NULL;
	tbl->ndel++;
}


int
lu_doall(tbl, f)		/* loop through all valid table entries */
register LUTAB	*tbl;
int	(*f)();
{
	int	rval = 0;
	register LUENT	*tp;

	for (tp = tbl->tabl + tbl->tsiz; tp-- > tbl->tabl; )
		if (tp->data != NULL)
			if (f != NULL)
				rval += (*f)(tp);
			else
				rval++;
	return(rval);
}


void
lu_done(tbl)			/* free table and contents */
register LUTAB	*tbl;
{
	register LUENT	*tp;

	if (!tbl->tsiz)
		return;
	for (tp = tbl->tabl + tbl->tsiz; tp-- > tbl->tabl; )
		if (tp->key != NULL) {
			if (tbl->freek != NULL)
				(*tbl->freek)(tp->key);
			if (tp->data != NULL && tbl->freed != NULL)
				(*tbl->freed)(tp->data);
		}
	free((void *)tbl->tabl);
	tbl->tabl = NULL;
	tbl->tsiz = 0;
	tbl->ndel = 0;
}
