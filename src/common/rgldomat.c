#ifndef lint
static const char	RCSid[] = "$Id$";
#endif
/*
 * Invocation routines for Radiance -> OpenGL materials.
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

#include "radogl.h"


void
domatobj(mp, cent)		/* generate OpenGL material for object */
register MATREC	*mp;
FVECT	cent;
{
	GLfloat	vec[4];

	if (mp == NULL | !domats)
		return;
	if (islight(mp->type)) {
		vec[0] = colval(mp->u.l.emission,RED);
		vec[1] = colval(mp->u.l.emission,GRN);
		vec[2] = colval(mp->u.l.emission,BLU);
		vec[3] = 1.;
		glMaterialfv(GL_FRONT, GL_EMISSION, vec);
		vec[0] = vec[1] = vec[2] = 0.; vec[3] = 1.;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, vec);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, vec);
	} else {
		vec[0] = colval(mp->u.m.ambdiff,RED);
		vec[1] = colval(mp->u.m.ambdiff,GRN);
		vec[2] = colval(mp->u.m.ambdiff,BLU);
		vec[3] = 1.;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, vec);
		vec[0] = colval(mp->u.m.specular,RED);
		vec[1] = colval(mp->u.m.specular,GRN);
		vec[2] = colval(mp->u.m.specular,BLU);
		vec[3] = 1.;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, vec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mp->u.m.specexp);
		vec[0] = vec[1] = vec[2] = 0.; vec[3] = 1.;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, vec);
	}
	rgl_checkerr("in domatobj");
}


void
domatvert(mp, v, n)		/* generate OpenGL material for vertex */
MATREC	*mp;
FVECT	v, n;
{
	/* unimplemented */
}
