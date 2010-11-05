/* $Id: topologyMap.c,v 1.5 2000/10/04 17:12:48 ami Exp $
 *
 * topologyMap.c - Linux IEEE-1394 Subsystem Topology Map fetching routine.
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

#include <topologyMap.h>
#include <netinet/in.h>
/*
 * This routine fetches the topology map from the CSR space of the local node
 * and returns it in a struct RAW1394topologyMap. Note that this behaviour is
 * not fully complient with the IEEE1394 standard, since the topology map is
 * only guaranteed to be correct in the bus manager node. It does work
 * however.
 * IN:		handle: The handle from libraw1394
 * RESULT:	The topology map.
 */
RAW1394topologyMap *raw1394GetTopologyMap(raw1394handle_t handle) {
	static RAW1394topologyMap topoMap;
	int ret,p;
	quadlet_t buf;

	if ((ret = cooked1394_read(handle, 0xffc0 | raw1394_get_local_id(handle),
		CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP, 4,
		(quadlet_t *) &buf)) < 0) return NULL;
	buf = htonl(buf);
	topoMap.length = (u_int16_t) (buf>>16);
	topoMap.crc = (u_int16_t) buf;
	if ((ret = cooked1394_read(handle, 0xffc0 | raw1394_get_local_id(handle),
		CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP + 4, 4,
		(quadlet_t *) &topoMap.generationNumber)) < 0) return NULL;

	topoMap.generationNumber = htonl(topoMap.generationNumber);

	if ((ret = cooked1394_read(handle, 0xffc0 | raw1394_get_local_id(handle),
		CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP + 8, 4,
		(quadlet_t *) &buf)) < 0) return NULL;
	buf = htonl(buf);
	topoMap.nodeCount = (u_int16_t) (buf>>16);
	topoMap.selfIdCount = (u_int16_t) buf;
	if (cooked1394_read(handle, 0xffc0 | raw1394_get_local_id(handle),
		CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP + 3*4,
		(topoMap.length-2)*4, ((quadlet_t *)&topoMap)+3) < 0)
		return NULL;
	for ( p=0 ; p < topoMap.length-2 ; p++) {
		*( ((quadlet_t *)&topoMap) +3+p) = 
			htonl( *( ( (quadlet_t *)&topoMap ) +3+p )   ); 
	}
	return &topoMap;
}
