#ifndef lint
static const char	RCSid[] = "$Id$";
#endif
/*
 *  readoct.c - routines to read octree information.
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

#include  "standard.h"

#include  "octree.h"

#include  "object.h"

#include  "otypes.h"

static double  ogetflt();
static long  ogetint();
static char  *ogetstr();
static int  nonsurfinset();
static int  getobj(), octerror(), skiptree();
static OCTREE  getfullnode(), gettree();

static char  *infn;			/* input file specification */
static FILE  *infp;			/* input file stream */
static int  objsize;			/* size of stored OBJECT's */
static OBJECT  objorig;			/* zeroeth object */
static OBJECT  fnobjects;		/* number of objects in this file */
static short  otypmap[NUMOTYPE+8];	/* object type map */


int
readoct(inpspec, load, scene, ofn)	/* read in octree file or stream */
char  *inpspec;
int  load;
CUBE  *scene;
char  *ofn[];
{
	char  sbuf[512];
	int  nf;
	register int  i;
	long  m;
	
	if (inpspec == NULL) {
		infn = "standard input";
		infp = stdin;
	} else if (inpspec[0] == '!') {
		infn = inpspec;
		if ((infp = popen(inpspec+1, "r")) == NULL) {
			sprintf(errmsg, "cannot execute \"%s\"", inpspec);
			error(SYSTEM, errmsg);
		}
	} else {
		infn = inpspec;
		if ((infp = fopen(inpspec, "r")) == NULL) {
			sprintf(errmsg, "cannot open octree file \"%s\"",
					inpspec);
			error(SYSTEM, errmsg);
		}
	}
#ifdef MSDOS
	setmode(fileno(infp), O_BINARY);
#endif
					/* get header */
	if (checkheader(infp, OCTFMT, load&IO_INFO ? stdout : (FILE *)NULL) < 0)
		octerror(USER, "not an octree");
					/* check format */
	if ((objsize = ogetint(2)-OCTMAGIC) <= 0 ||
			objsize > MAXOBJSIZ || objsize > sizeof(long))
		octerror(USER, "incompatible octree format");
					/* get boundaries */
	if (load & IO_BOUNDS) {
		for (i = 0; i < 3; i++)
			scene->cuorg[i] = atof(ogetstr(sbuf));
		scene->cusize = atof(ogetstr(sbuf));
	} else {
		for (i = 0; i < 4; i++)
			ogetstr(sbuf);
	}
	objorig = nobjects;		/* set object offset */
	nf = 0;				/* get object files */
	while (*ogetstr(sbuf)) {
		if (load & IO_SCENE)
			readobj(sbuf);
		if (load & IO_FILES)
			ofn[nf] = savqstr(sbuf);
		nf++;
	}
	if (load & IO_FILES)
		ofn[nf] = NULL;
					/* get number of objects */
	fnobjects = m = ogetint(objsize);
	if (fnobjects != m)
		octerror(USER, "too many objects");

	if (load & IO_TREE)		/* get the octree */
		scene->cutree = gettree();
	else if (load & IO_SCENE && nf == 0)
		skiptree();
		
	if (load & IO_SCENE)		/* get the scene */
	    if (nf == 0) {
		for (i = 0; *ogetstr(sbuf); i++)
			if ((otypmap[i] = otype(sbuf)) < 0) {
				sprintf(errmsg, "unknown type \"%s\"", sbuf);
				octerror(WARNING, errmsg);
			}
		while (getobj() != OVOID)
			;
	    } else {			/* consistency checks */
				/* check object count */
		if (nobjects != objorig+fnobjects)
			octerror(USER, "bad object count; octree stale?");
				/* check for non-surfaces */
		if (dosets(nonsurfinset))
			octerror(USER, "modifier in tree; octree stale?");
	    }
				/* close the input */
	if (infn[0] == '!')
		pclose(infp);
	else
		fclose(infp);
	return(nf);
}


static char *
ogetstr(s)			/* get null-terminated string */
char  *s;
{
	extern char  *getstr();

	if (getstr(s, infp) == NULL)
		octerror(USER, "truncated octree");
	return(s);
}


static OCTREE
getfullnode()			/* get a set, return fullnode */
{
	OBJECT	set[MAXSET+1];
	register int  i;
	register long  m;

	if ((set[0] = ogetint(objsize)) > MAXSET)
		octerror(USER, "bad set in getfullnode");
	for (i = 1; i <= set[0]; i++) {
		m = ogetint(objsize) + objorig;
		if ((set[i] = m) != m)
			octerror(USER, "too many objects");
	}
	return(fullnode(set));
}	


static long
ogetint(siz)			/* get a siz-byte integer */
int  siz;
{
	extern long  getint();
	register long  r;

	r = getint(siz, infp);
	if (feof(infp))
		octerror(USER, "truncated octree");
	return(r);
}


static double
ogetflt()			/* get a floating point number */
{
	extern double  getflt();
	double	r;

	r = getflt(infp);
	if (feof(infp))
		octerror(USER, "truncated octree");
	return(r);
}
	

static OCTREE
gettree()			/* get a pre-ordered octree */
{
	register OCTREE	 ot;
	register int  i;
	
	switch (getc(infp)) {
	case OT_EMPTY:
		return(EMPTY);
	case OT_FULL:
		return(getfullnode());
	case OT_TREE:
		if ((ot = octalloc()) == EMPTY)
			octerror(SYSTEM, "out of tree space in gettree");
		for (i = 0; i < 8; i++)
			octkid(ot, i) = gettree();
		return(ot);
	case EOF:
		octerror(USER, "truncated octree");
	default:
		octerror(USER, "damaged octree");
	}
}


static int
nonsurfinset(os)			/* check set for modifier */
register OBJECT  *os;
{
	register OBJECT  s;
	register int  i;

	for (i = *os; i-- > 0; )
		if ((s = *++os) >= objorig && s < objorig+fnobjects &&
				ismodifier(objptr(s)->otype))
			return(1);
	return(0);
}


static
skiptree()				/* skip octree on input */
{
	register int  i;
	
	switch (getc(infp)) {
	case OT_EMPTY:
		return;
	case OT_FULL:
		for (i = ogetint(objsize)*objsize; i-- > 0; )
			if (getc(infp) == EOF)
				octerror(USER, "truncated octree");
		return;
	case OT_TREE:
		for (i = 0; i < 8; i++)
			skiptree();
		return;
	case EOF:
		octerror(USER, "truncated octree");
	default:
		octerror(USER, "damaged octree");
	}
}


static
getobj()				/* get next object */
{
	char  sbuf[MAXSTR];
	int  obj;
	register int  i;
	register long  m;
	register OBJREC	 *objp;

	i = ogetint(1);
	if (i == -1)
		return(OVOID);		/* terminator */
	if ((obj = newobject()) == OVOID)
		error(SYSTEM, "out of object space");
	objp = objptr(obj);
	if ((objp->otype = otypmap[i]) < 0)
		octerror(USER, "reference to unknown type");
	if ((m = ogetint(objsize)) != OVOID) {
		m += objorig;
		if ((OBJECT)m != m)
			octerror(USER, "too many objects");
	}
	objp->omod = m;
	objp->oname = savqstr(ogetstr(sbuf));
	if (objp->oargs.nsargs = ogetint(2)) {
		objp->oargs.sarg = (char **)malloc
				(objp->oargs.nsargs*sizeof(char *));
		if (objp->oargs.sarg == NULL)
			goto memerr;
		for (i = 0; i < objp->oargs.nsargs; i++)
			objp->oargs.sarg[i] = savestr(ogetstr(sbuf));
	} else
		objp->oargs.sarg = NULL;
#ifdef	IARGS
	if (objp->oargs.niargs = ogetint(2)) {
		objp->oargs.iarg = (long *)malloc
				(objp->oargs.niargs*sizeof(long));
		if (objp->oargs.iarg == NULL)
			goto memerr;
		for (i = 0; i < objp->oargs.niargs; i++)
			objp->oargs.iarg[i] = ogetint(4);
	} else
		objp->oargs.iarg = NULL;
#endif
	if (objp->oargs.nfargs = ogetint(2)) {
		objp->oargs.farg = (FLOAT *)malloc
				(objp->oargs.nfargs*sizeof(FLOAT));
		if (objp->oargs.farg == NULL)
			goto memerr;
		for (i = 0; i < objp->oargs.nfargs; i++)
			objp->oargs.farg[i] = ogetflt();
	} else
		objp->oargs.farg = NULL;
						/* initialize */
	objp->os = NULL;
						/* insert */
	insertobject(obj);
	return(obj);
memerr:
	error(SYSTEM, "out of memory in getobj");
}


static
octerror(etyp, msg)			/* octree error */
int  etyp;
char  *msg;
{
	char  msgbuf[128];

	sprintf(msgbuf, "(%s): %s", infn, msg);
	error(etyp, msgbuf);
}
