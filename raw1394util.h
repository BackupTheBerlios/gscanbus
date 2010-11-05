/* $Id: raw1394util.h,v 1.3 2001/05/09 10:27:02 ami Exp $
 *
 * raw1394util.h - Linux IEEE-1394 Subsystem utility routines for
 * libraw1394
 * These routines provide cooked versions of the routines in libraw1394. They
 * do automatic error checking and retries, return code munging, etc.
 *
 * When this code has settleted into a stable form it should probably merged
 * into libraw1394.
 *
 * Written 4.10.2000 by Andreas Micklei
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

#include <libraw1394/raw1394.h>
#include "debug.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int cooked1394_read(raw1394handle_t handle, nodeid_t node, nodeaddr_t addr,
                 size_t length, quadlet_t *buffer);

int cooked1394_write(raw1394handle_t handle, nodeid_t node, nodeaddr_t addr,
                  size_t length, quadlet_t *data);

