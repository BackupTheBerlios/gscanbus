/* $Id: fatal.c,v 1.2 2000/06/16 14:00:22 ami Exp $
 *
 * fatal.c
 */

#include "fatal.h"

void fatal(char *s) {
	fprintf(stderr, "Error: %s\n", s);
	exit(1);
}
