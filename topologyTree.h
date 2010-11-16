/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * topologyTree.h - Linux IEEE-1394 Subsystem Topology Tree
 * spawning and traversing routines
 * Written 8.11.1999 - 2.6.2000 by Andreas Micklei
 * 2.6.2000: added support for >=4 port phys
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

#ifndef __TOPOLOGYTREE_H__
#define __TOPOLOGYTREE_H__
#include "raw1394support.h"
#include "rominfo.h"
#include "decodeselfid.h"
#include "fatal.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <libraw1394/raw1394.h>

#define MAX(a,b) ((a)>(b)?(a):(b))

#define TEST_SELFID 0x80000000

#define MAX_CHILDS (3+3*8)

void fatal(char *s);

typedef struct TopologyTree_t {
	//SelfIdPacket_t		selfid;
	SelfIdPacket_t			selfid[4];
	Rom_info			rom_info;
	char				label[256];
	struct TopologyTree_t		*parent;
	//struct TopologyTree_t		*child1;
	//struct TopologyTree_t		*child2;
	//struct TopologyTree_t		*child3;
	struct TopologyTree_t		*child[MAX_CHILDS];
} TopologyTree;

char *decodeCameraSwVersion(quadlet_t quadlet);

int flipcoin(void);

RAW1394topologyMap *generateTestTopologyMap(int nnodes);

/*
 * in:	topologyTree: topology Tree from lowest node on
 * 	nodeid: number of root node of current subtree
 *	parent: parent node of subtree
 * out:	number of next lower unprocessed child node
 */
int spawnTopologySubTree(TopologyTree *topologyTree, int nodeid,
	TopologyTree *parent);

TopologyTree *spawnTopologyTree(raw1394handle_t handle,
	RAW1394topologyMap *topologyMap);

void freeTopologyTree(TopologyTree *topologyTree);

TopologyTree *topologyTreeRoot(TopologyTree *topologyTree);

int topologySubTreeDepth(TopologyTree *node, int level);

int topologyTreeDepth(TopologyTree *topologyTree);

int numberOfChilds(TopologyTree *node);

TopologyTree *getNthChild(TopologyTree *node, int n);

void generateTopologySubTreeString(char *topologyTreeString,
	TopologyTree *node, int left, int width, int level, int localid);

char *generateTopologyTreeString(TopologyTree *topologyTree, int localid);

#endif
