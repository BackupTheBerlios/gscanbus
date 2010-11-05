/* $Id: debug.c,v 1.1 2000/06/27 19:02:58 ami Exp $
 *
 * debug.c
 */

#include <stdio.h>

char debug_level = 0;

void set_debug_level(char n) {
	debug_level = n;
	fprintf(stderr,"Setting Debug level: %i\n",n);
}

