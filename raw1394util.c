/* $Id: raw1394util.c,v 1.4 2001/07/11 10:53:51 ami Exp $
 *
 * raw1394util.c - Linux IEEE-1394 Subsystem utility routines for
 * libraw1394
 * These routines provide cooked versions of the routines in libraw1394. They
 * do automatic error checking and retries, return code munging, etc.
 *
 * When this code has settleted into a stable form it should probably merged
 * into libraw1394.
 *
 * Written 4.10.2000 - 26.01.2001 by Andreas Micklei
 *
 * 26.01.2001: Adapted to new error handling scheme in libraw1394
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "raw1394util.h"

#define DEBUG_ACK_RCODE(ackcode,rcode) DEBUG_LOWLEVEL_ERR fprintf(stderr, "Ack code: 0x%0x, Response code: 0x%0x\n",(ackcode),(rcode));
#define MAXTRIES 20
#define DELAY 10000

int cooked1394_read(raw1394handle_t handle, nodeid_t node, nodeaddr_t addr,
                 size_t length, quadlet_t *buffer) {
	int retval, i;
	for(i=0; i<MAXTRIES; i++) {
		retval = raw1394_read(handle, node, addr, length, buffer);
		if( retval >= 0 ) return retval;	/* Everything is OK */
		DEBUG_ACK_RCODE( raw1394_get_ack(raw1394_get_errcode(handle)),
			raw1394_get_rcode(raw1394_get_errcode(handle)) );
		if( errno != EAGAIN ) break;
		usleep(DELAY);
	}
	perror("Error while reading from IEEE1394: ");
	return retval;
}

int cooked1394_write(raw1394handle_t handle, nodeid_t node, nodeaddr_t addr,
                  size_t length, quadlet_t *data) {
	int retval, i;
	for(i=0; i<MAXTRIES; i++) {
		retval = raw1394_write(handle, node, addr, length, data);
		if( retval >= 0 ) return retval;	/* Everything is OK */
		DEBUG_ACK_RCODE( raw1394_get_ack(raw1394_get_errcode(handle)),
			raw1394_get_rcode(raw1394_get_errcode(handle)) );
		if( errno != EAGAIN ) break;
		usleep(DELAY);
	}
	perror("Error while writing to IEEE1394: ");
	return retval;
}

