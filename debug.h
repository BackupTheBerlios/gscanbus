/* $Id: debug.h,v 1.2 2000/10/04 17:12:48 ami Exp $
 *
 * debug.h
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

//extern static char debug_level;
extern char debug_level;

void set_debug_level(char n);

#define DEBUG_GENERAL		if (debug_level > 0)
#define DEBUG_CSR		if (debug_level > 1)
#define DEBUG_AVC		if (debug_level > 1)
#define DEBUG_CONFIG		if (debug_level > 1)
#define DEBUG_LOWLEVEL_ERR	if (debug_level > 2)
#define DEBUG_LOWLEVEL		if (debug_level > 3)
#endif

