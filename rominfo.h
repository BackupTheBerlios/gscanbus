/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * rominfo.h - Linux IEEE-1394 Subsystem CSR ROM info reading routines
 * written 23.11.1999 by Andreas Micklei
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

#ifndef __ROMINFO_H__
#define __ROMINFO_H__
#include "raw1394support.h"
#include "raw1394util.h"
//#include "topologyTree.h"
#include "fatal.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NODE_TYPE_UNKNOWN	0
#define NODE_TYPE_CONF_CAM	1
#define NODE_TYPE_AVC		2
#define NODE_TYPE_SBP2		3
#define NODE_TYPE_CPU		4

/*
 * This structure holds various interesting data about a device which can be
 * obtained from the configuration rom
 */
typedef struct rom_info_t {
	quadlet_t	magic;	/* should contain 1394 in ASCII */
	char		irmc;
	char		cmc;
	char		isc;
	char		bmc;
	unsigned char	cyc_clk_acc;
	char		max_rec;
	quadlet_t	guid_hi;
	quadlet_t	guid_lo;
	quadlet_t	node_capabilities;
	quadlet_t	vendor_id;
	quadlet_t	unit_spec_id;
	quadlet_t	unit_sw_version;
	quadlet_t	model_id;
	int		nr_textual_leafes;
	char		**textual_leafes;
	char		*label;	/* aggregated from textual leafes */
	char		*vendor;
	int		node_type;	/* NODE_TYPE_AVC, etc. */
} Rom_info;

/*
 * Fill out the structure with zeroes
 * IN:  rom_info:	Pointer to the unitiliazied structure
 */
void init_rom_info(Rom_info *rom_info);

/*
 * Resolve a guid into a name from the configuration file. Read in the file on
 * first invocation
 * IN:		guid_hi:	High quadlet of GUI
 * 		guid_lo:	Low quadlet of GUI
 * RETURNS:	Pointer to the description string
 */
char *resolv_guid(int guid_hi, int guid_lo, char *cpu);

/*
 * Get the type / protocol of a node
 * IN:		rom_info:	pointer to the Rom_info structure of the node
 * RETURNS:	one of the defined node types, i.e. NODE_TYPE_AVC, etc.
 */
int get_node_type(Rom_info *rom_info);

/*
 * Obtain the global unique identifier of a node from its configuration ROM.
 * The GUID can also be read from a filled out Rom_info structure, but this
 * method is of course faster than reading the whole configuration ROM and can
 * for instance be used to obtain a hash key for caching Rom_info structures
 * in memory.
 * IN:  phyID:	Physical ID of the node to read from
 *      hi:	Pointer to an integer which should receive the HI quadlet
 *      hi:	Pointer to an integer which should receive the LOW quadlet
 */
/*void get_guid(raw1394handle_t handle, int phyID,
	unsigned int *hi, unsigned int *lo);*/

/*
 * Read a textual leaf into a malloced ASCII string
 * TODO: This routine should probably care about character sets, Unicode, etc.
 * IN:		phyID:	Physical ID of the node to read from
 *		offset:	Memory offset to read from
 * RETURNS:	pointer to a freshly malloced string that contains the
 *		requested text or NULL if the text could not be read.
 */
/*char *read_textual_leaf(raw1394handle_t handle, int phyID, octlet_t offset);*/

/*
 * Read a whole bunch of textual leafes from a node into an array of ASCII
 * strings.
 * IN:		phyID:		Physical ID of the node to read from
 *		offsets:	Memory offsets to read from
 *		n:		Number of Strings to read
 * RETURNS:	pointer to a freshly malloced array of freshly malloced
 *		strings that contains the requested texts. Some of the
 *		strings might be NULL however.
 */
/*char **read_textual_leafes(raw1394handle_t handle, int phyID, octlet_t offsets[],
	int n);*/

/*
 * Read various information from the configuration ROM of a device into a
 * Rom_info struct.
 * IN:		phyID:		Physical ID of the node to read from
 *		rom_info:	Pointer to a structure to fill
 * RETURNS:	0 on success, -1 on error
 * NOTE:	Some strings may be malloced by this routine. free_rom_info
 *		schould therefore be called, when the contents of this
 *		structure are no longer needed.
 */
int get_rom_info(raw1394handle_t handle, int phyID, Rom_info *rom_info);

/*
 * Free up all memory malloced by get_rom_info.
 * IN:  rom_info:	pointer to the Rom_info structure which is no longer
 * 			needed
 */
void free_rom_info(Rom_info *rom_info);

#endif

