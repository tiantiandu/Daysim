#ifndef lint
static const char	RCSid[] = "$Id$";
#endif
/*
 * Initialize ofun[] list for octree generator
 */

#include  "standard.h"

#include  "octree.h"

#include  "otypes.h"

extern int  o_sphere();
extern int  o_face();
extern int  o_cone();
extern int  o_instance();

FUN  ofun[NUMOTYPE] = INIT_OTYPE;


initotypes()			/* initialize ofun array */
{
	ofun[OBJ_SPHERE].funp =
	ofun[OBJ_BUBBLE].funp = o_sphere;
	ofun[OBJ_FACE].funp = o_face;
	ofun[OBJ_CONE].funp =
	ofun[OBJ_CUP].funp =
	ofun[OBJ_CYLINDER].funp =
	ofun[OBJ_TUBE].funp =
	ofun[OBJ_RING].funp = o_cone;
	ofun[OBJ_INSTANCE].funp = o_instance;
}


o_default()			/* default action is no intersection */
{
	return(O_MISS);
}
