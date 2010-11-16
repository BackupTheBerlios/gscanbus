/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * topologyTree.c - Linux IEEE-1394 Subsystem Topology Tree
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
#include <netinet/in.h>
#include "topologyTree.h"

#define MIN(x,y) ((x)<(y))?(x):(y)
#define MAX(a,b) ((a)>(b)?(a):(b))

static const char *decodeCameraSwVersion(quadlet_t quadlet) 
{
	if ((quadlet & 0xFFFFFF) == 0x000101) return "1.20";
	if ((quadlet & 0xFFFFFF) == 0x000100) return "1.04";
	return "Unknown";
}

static int flipcoin(void) 
{
	return (random()/(RAND_MAX/2));
}

RAW1394topologyMap *generateTestTopologyMap(int nnodes) {
	RAW1394topologyMap *map;
	quadlet_t *pselfid;
	int i, leafes;
	map = malloc(sizeof(RAW1394topologyMap));
	if (!map) fatal("out of memory!");
	leafes = 1;
	for (i=nnodes-1; i>=0; i--) {
		pselfid = ((quadlet_t *) &(map->selfIdPacket[i]));
		*pselfid = (TEST_SELFID | (i<<24));
		if (i<nnodes-1) *pselfid = *pselfid | (2<<6);	/*has parent*/
		if (i<=leafes) { /*no childs*/
		} else {	 /*has childs*/
			*pselfid = *pselfid | (3<<4);
			if (flipcoin()) {
				*pselfid = *pselfid | (3<<2);
				leafes++;
			}
		}
		/*printf("Node: %i: %08x\n",i,*pselfid);*/
	}
	map->length = nnodes*4+4;
	map->crc = 0;	/*invalid*/
	map->generationNumber = 0;
	map->nodeCount = nnodes;
	map->selfIdCount = nnodes;
	return map;
}

/*
 * in:	topologyTree: topology Tree from lowest node on
 * 	nodeid: number of root node of current subtree
 *	parent: parent node of subtree
 * out:	number of next lower unprocessed child node
 */
int spawnTopologySubTree(TopologyTree *topologyTree, int nodeid,
			TopologyTree *parent) 
{
	SelfIdPacket_t *selfid0, *selfid1, *selfid2, *selfid3;
	TopologyTree *pnode;
	int myid;
	/* FIXME - only handles three ports */
	/*printf("SpawnTopologySubTree called with nodeid: %d\n",nodeid);*/
	myid = nodeid;			/* remember current id */
	pnode = &topologyTree[nodeid];	/* point at current node */
	selfid0 = pnode->selfid;		/* get current node selfid */
	pnode->parent = parent;		/* set parent node */
	nodeid--;			/* process only lower nodes */
	/* get nodeids of child nodes */
	if (selfid0->packetZero.morePackets == 1) {
		selfid1 = selfid0+1;
		if (selfid1->packetMore.morePackets == 1) {
			selfid2 = selfid1+1;
			if (selfid2->packetMore.morePackets == 1) {
				selfid3 = selfid2+1;
				/* FIXME */
			}
			/* FIXME */
		}
		if (selfid1->packetMore.portH == SELFID_PORT_CHILD)  {
			pnode->child[10] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
		if (selfid1->packetMore.portG == SELFID_PORT_CHILD)  {
			pnode->child[9] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
		if (selfid1->packetMore.portF == SELFID_PORT_CHILD)  {
			pnode->child[8] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
		if (selfid1->packetMore.portE == SELFID_PORT_CHILD)  {
			pnode->child[7] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
		if (selfid1->packetMore.portD == SELFID_PORT_CHILD)  {
			pnode->child[6] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
		if (selfid1->packetMore.portC == SELFID_PORT_CHILD)  {
			pnode->child[5] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
		if (selfid1->packetMore.portB == SELFID_PORT_CHILD)  {
			pnode->child[4] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
		if (selfid1->packetMore.portA == SELFID_PORT_CHILD)  {
			pnode->child[3] = &topologyTree[nodeid];
			nodeid = spawnTopologySubTree(topologyTree, nodeid,
				pnode);
		}
	}
	if (selfid0->packetZero.port2 == SELFID_PORT_CHILD)  {
		/*printf("Child3 of node %d is node %d\n", myid, nodeid);*/
		pnode->child[2] = &topologyTree[nodeid];
		nodeid = spawnTopologySubTree(topologyTree, nodeid, pnode);
	}
	if (selfid0->packetZero.port1 == SELFID_PORT_CHILD)  {
		/*printf("Child2 of node %d is node %d\n", myid, nodeid);*/
		pnode->child[1] = &topologyTree[nodeid];
		nodeid = spawnTopologySubTree(topologyTree, nodeid, pnode);
	}
	if (selfid0->packetZero.port0 == SELFID_PORT_CHILD)  {
		/*printf("Child1 of node %d is node %d\n", myid, nodeid);*/
		pnode->child[0] = &topologyTree[nodeid];
		nodeid = spawnTopologySubTree(topologyTree, nodeid, pnode);
	}
	return nodeid;
}

TopologyTree *spawnTopologyTree(raw1394handle_t handle,
				RAW1394topologyMap *topologyMap) 
{
	int i, j, ret, selfIdCount, nodeCount;
	unsigned int *pselfid_int;
	//unsigned char *pselfid_char;
	TopologyTree *topologyTree, *ptopologyTree;

	if (topologyMap == NULL) return NULL;
	selfIdCount = topologyMap->selfIdCount;
	nodeCount = topologyMap->nodeCount;
	//topologyTree = calloc(nodeCount, sizeof(TopologyTree));
	topologyTree = malloc(nodeCount*sizeof(TopologyTree));
	if (!topologyTree) fatal("out of memory!");
	ptopologyTree = topologyTree;
	for (i=0; i < selfIdCount; i++) {
		//pselfid_int = (void *) &topologyMap->selfIdPacket[i];
		//ret = decode_selfid(&(ptopologyTree->selfid), *pselfid_int);
		pselfid_int = (unsigned int *) &topologyMap->selfIdPacket[i];
		ret = decode_selfid(ptopologyTree->selfid, pselfid_int);
		if (ret < 0) {
			//printbin(*((unsigned int *)ptopologyTree->selfid),32);
			//printbin(*((unsigned int *)ptopologyTree->selfid),32);
			//printbin(*((unsigned int *)ptopologyTree->selfid),32);
			//printbin(*((unsigned int *)ptopologyTree->selfid),32);
			fatal("invalid or unsupported selfid format!");
		}
		if (ptopologyTree->selfid[0].packetZero.linkActive) {
			get_rom_info(handle,
				ptopologyTree->selfid[0].packetZero.phyID,
				&ptopologyTree->rom_info);
		} else {
			init_rom_info(&ptopologyTree->rom_info);
		}
		ptopologyTree->parent = NULL;
		for (j=0; j < MAX_CHILDS; j++) 
			ptopologyTree->child[j] = NULL;
		ptopologyTree++;
		DEBUG_GENERAL fprintf(stderr, "selfIdCount: %i, nodeCount: %i, i: %i, selfids: %i\n",
			selfIdCount, nodeCount, i, ret);
		i += (ret-1);
	};
	spawnTopologySubTree(topologyTree, nodeCount-1, NULL);
	return &topologyTree[nodeCount-1];	/* return root node */
}

TopologyTree *topologyTreeLowestNode(TopologyTree *topologyTree) 
{
	TopologyTree *p = topologyTree;
	int i;
	for (i=0; i<MAX_CHILDS; i++) {
		if (p->child[i] != NULL) p = MIN(topologyTreeLowestNode(
			p->child[i]), p);
	}
	return p;
}

void freeSubTopologyTree(TopologyTree *topologyTree) 
{
	int i;
	for (i=0; i<MAX_CHILDS; i++) {
		if (topologyTree->child[i] != NULL)
			freeSubTopologyTree(topologyTree->child[i]);
	}
	free_rom_info(&topologyTree->rom_info);
}

void freeTopologyTree(TopologyTree *topologyTree) 
{
	TopologyTree *ptopologyTree = topologyTreeLowestNode(topologyTree);
	freeSubTopologyTree(topologyTree);
	return;	//FIXME
	free(ptopologyTree);
}

TopologyTree *topologyTreeRoot(TopologyTree *topologyTree) 
{
	if (topologyTree->parent == NULL) return topologyTree;
	else return topologyTreeRoot(topologyTree->parent);
}

int topologySubTreeDepth(TopologyTree *node, int level) 
{
	int maxdepth, i;
	if (node == NULL) return level;
	maxdepth = 0;
	level++;
	for (i=0; i<MAX_CHILDS; i++) {
		maxdepth = MAX(maxdepth, topologySubTreeDepth(node->child[i],
			level));
	}
	return maxdepth;
}

int topologyTreeDepth(TopologyTree *topologyTree) 
{
	return topologySubTreeDepth(topologyTreeRoot(topologyTree), 0);
}

int numberOfChilds(TopologyTree *node) 
{
	int i, n = 0;

	if(!node)
		return 0;

	for (i=0; i<MAX_CHILDS; i++) 
		if (node->child[i] != NULL) 
			n++;

	return n;
}

TopologyTree *getNthChild(TopologyTree *node, int n) 
{
	int i;
	if (n < 1 || n > MAX_CHILDS) return NULL;
	for (i=0; i<MAX_CHILDS; i++) {
		if (node->child[i] != NULL) n--;
		if (n < 1) return node->child[i];
	}
	fatal("Unexpected Error in getNthChild");
	return NULL;
}

