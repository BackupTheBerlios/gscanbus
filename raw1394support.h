/* $Id: raw1394support.h,v 1.3 2001/05/09 10:27:02 ami Exp $
 *
 * raw1394.h - Linux IEEE-1394 Subsystem RAW 1394 support library
 * for some compatibility with older library
 */

#ifndef __RAW1394SUPPORT_H__
#define __RAW1394SUPPORT_H__

#include <libraw1394/raw1394.h>
#include <libraw1394/csr.h>

#define SELFID_PORT_CHILD        0x3
#define SELFID_PORT_PARENT       0x2
#define SELFID_PORT_NCONN        0x1
#define SELFID_PORT_NONE         0x0

typedef union {
    struct packetZero_t {
	unsigned designator :2;
	unsigned phyID      :6;
	unsigned ZeroOrMore :1;
	unsigned linkActive :1;
	unsigned gapCount   :6;
	unsigned phySpeed   :2;
	unsigned phyDelay   :2;
	unsigned contender  :1;
	unsigned powerClass :3;
	unsigned port0      :2;
	unsigned port1      :2;
	unsigned port2      :2;
	unsigned initiatedReset :1;
	unsigned morePackets    :1;
    } packetZero;
    struct packetMore_t {
	unsigned designator :2;
	unsigned phyID      :6;
	unsigned ZeroOrMore :1;
	unsigned packetNumber :3;
	unsigned rsv        :2;
	unsigned portA      :2;
	unsigned portB      :2;
	unsigned portC      :2;
	unsigned portD      :2;
	unsigned portE      :2;
	unsigned portF      :2;
	unsigned portG      :2;
	unsigned portH      :2;
	unsigned r          :1;
	unsigned morePackets :1;
    } packetMore;
} SelfIdPacket_t;

typedef struct RAW1394topologyMap_t {
    u_int16_t length;
    u_int16_t crc;
    u_int32_t generationNumber;
    u_int16_t nodeCount;
    u_int16_t selfIdCount;
    SelfIdPacket_t selfIdPacket[(0x400 - 4)];
} RAW1394topologyMap;

#endif
