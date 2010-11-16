/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * topologyMap.h - Linux IEEE-1394 Subsystem Topology Map fetching routine.
 * This routine serves as a temporary replacement for the
 * raw1394GetTopologyMap routine found in version 0.2 of libraw1394.
 * Written 8.12.1999 by Andreas Micklei
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

#ifndef __TOPOLOGYMAP_H__
#define __TOPOLOGYMAP_H__
#include <stdlib.h>
#include <libraw1394/raw1394.h>
#include "raw1394support.h"
#include "raw1394util.h"

RAW1394topologyMap *raw1394GetTopologyMap(raw1394handle_t handle);
#endif

