/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * decodeselfid.c
 * written 19.10.1999 - 31.5.2000 by Andreas Micklei
 * 20.10.1999: initial revision
 * 31.5.2000: added support for >=4 port phys, therefore changed return type
 *            and parameters of decode_selfid
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

#include "decodeselfid.h"

void printbin(FILE *stream, unsigned int i, unsigned char width) {
	int j;
	for (j=0; j<width; j++) {
		fprintf(stream, "%i", (i & (1<<(width-1)))>>(width-1));
		i = i << 1;
	}
}

unsigned int bit_extract(unsigned int shift, unsigned int width, unsigned int i)
{
	unsigned int mask = 0;
	while (width>0) {
		mask=mask<<1;
		mask++;
		width--;
	}
	return (i >> shift) & mask;
}

#if 0
int decode_selfid(SelfIdPacket_t *selfid, unsigned int i) {
	selfid->packetZero.designator = bit_extract(SHIFT_START, WIDTH_START, i);
	selfid->packetZero.phyID = bit_extract(SHIFT_PHY_ID, WIDTH_PHY_ID, i);
	selfid->packetZero.ZeroOrMore = bit_extract(SHIFT_CONT, WIDTH_CONT, i);
	selfid->packetZero.linkActive = bit_extract(SHIFT_L, WIDTH_L, i);
	selfid->packetZero.gapCount = bit_extract(SHIFT_GAP_CNT, WIDTH_GAP_CNT, i);
	selfid->packetZero.phySpeed = bit_extract(SHIFT_SP, WIDTH_SP, i);
	selfid->packetZero.phyDelay = bit_extract(SHIFT_DEL, WIDTH_DEL, i);
	selfid->packetZero.contender = bit_extract(SHIFT_C, WIDTH_C, i);
	selfid->packetZero.powerClass = bit_extract(SHIFT_PWR, WIDTH_PWR, i);
	selfid->packetZero.port0 = bit_extract(SHIFT_P0, WIDTH_P0, i);
	selfid->packetZero.port1 = bit_extract(SHIFT_P1, WIDTH_P1, i);
	selfid->packetZero.port2 = bit_extract(SHIFT_P2, WIDTH_P2, i);
	selfid->packetZero.initiatedReset = bit_extract(SHIFT_I, WIDTH_I, i);
	selfid->packetZero.morePackets = bit_extract(SHIFT_M, WIDTH_M, i);
	if (selfid->packetZero.designator != 2) return -1;
	if (selfid->packetZero.morePackets == 1) return -2;
	else return 0;
}
#endif

void decode_selfid_zero(SelfIdPacket_t *p, unsigned int i) {
	DEBUG_LOWLEVEL {
		fprintf(stderr, "decode_selfid_zero:");
		printbin(stderr,i,32);
		fprintf(stderr, "\n");
	}
	p->packetZero.designator = bit_extract(SHIFT_START, WIDTH_START, i);
	p->packetZero.phyID = bit_extract(SHIFT_PHY_ID, WIDTH_PHY_ID, i);
	p->packetZero.ZeroOrMore = bit_extract(SHIFT_CONT, WIDTH_CONT, i);
	p->packetZero.linkActive = bit_extract(SHIFT_L, WIDTH_L, i);
	p->packetZero.gapCount = bit_extract(SHIFT_GAP_CNT, WIDTH_GAP_CNT, i);
	p->packetZero.phySpeed = bit_extract(SHIFT_SP, WIDTH_SP, i);
	p->packetZero.phyDelay = bit_extract(SHIFT_DEL, WIDTH_DEL, i);
	p->packetZero.contender = bit_extract(SHIFT_C, WIDTH_C, i);
	p->packetZero.powerClass = bit_extract(SHIFT_PWR, WIDTH_PWR, i);
	p->packetZero.port0 = bit_extract(SHIFT_P0, WIDTH_P0, i);
	p->packetZero.port1 = bit_extract(SHIFT_P1, WIDTH_P1, i);
	p->packetZero.port2 = bit_extract(SHIFT_P2, WIDTH_P2, i);
	p->packetZero.initiatedReset = bit_extract(SHIFT_I, WIDTH_I, i);
	p->packetZero.morePackets = bit_extract(SHIFT_M, WIDTH_M, i);
}

void decode_selfid_more(SelfIdPacket_t *p, unsigned int i) {
	DEBUG_LOWLEVEL {
		fprintf(stderr, "decode_selfid_more:");
		printbin(stderr,i,32);
		fprintf(stderr, "\n");
	}
	p->packetMore.designator = bit_extract(SHIFT_START, WIDTH_START, i);
	p->packetMore.phyID = bit_extract(SHIFT_PHY_ID, WIDTH_PHY_ID, i);
	p->packetMore.ZeroOrMore = bit_extract(SHIFT_CONT, WIDTH_CONT, i);
	p->packetMore.packetNumber = bit_extract(SHIFT_N, WIDTH_N, i);
	p->packetMore.rsv = bit_extract(SHIFT_RSV, WIDTH_RSV, i);
	p->packetMore.portA = bit_extract(SHIFT_PA, WIDTH_PA, i);
	p->packetMore.portB = bit_extract(SHIFT_PB, WIDTH_PB, i);
	p->packetMore.portC = bit_extract(SHIFT_PC, WIDTH_PC, i);
	p->packetMore.portD = bit_extract(SHIFT_PD, WIDTH_PD, i);
	p->packetMore.portE = bit_extract(SHIFT_PE, WIDTH_PE, i);
	p->packetMore.portF = bit_extract(SHIFT_PF, WIDTH_PF, i);
	p->packetMore.portG = bit_extract(SHIFT_PG, WIDTH_PG, i);
	p->packetMore.portH = bit_extract(SHIFT_PH, WIDTH_PH, i);
	p->packetMore.r = bit_extract(SHIFT_R, WIDTH_R, i);
	p->packetMore.morePackets = bit_extract(SHIFT_M, WIDTH_M, i);
}

int decode_selfid(SelfIdPacket_t *selfid, unsigned int *selfid_raw) {
	SelfIdPacket_t *p;

	p = selfid;
	decode_selfid_zero(p, selfid_raw[0]);
	//if (p->packetZero.designator != 2) return -1;
	//if (p->packetZero.ZeroOrMore != 0) return -2;

	if (p->packetZero.morePackets == 0) return 1;

	p++;
	decode_selfid_more(p, selfid_raw[1]);
	//if (p->packetMore.designator != 2) return -1;
	//if (p->packetZero.ZeroOrMore != 1) return -2;

	if (p->packetMore.morePackets == 0) return 2;

	p++;
	decode_selfid_more(p, selfid_raw[2]);
	//if (p->packetMore.designator != 2) return -1;
	//if (p->packetZero.ZeroOrMore != 1) return -2;

	if (p->packetMore.morePackets == 0) return 3;

	p++;
	decode_selfid_more(p, selfid_raw[3]);
	//if (p->packetMore.designator != 2) return -1;
	//if (p->packetZero.ZeroOrMore != 1) return -2;

	return 4;
}

char *yes_no(unsigned char i) {
	if (i) return "Yes";
	else return "No";
}

char *decode_speed(unsigned char i) {
	if (i == 0) return "S100";
	if (i == 1) return "S200";
	if (i == 2) return "S400";
	if (i == 3) return "Unknown";
	else return "Error";
}

char *decode_delay(unsigned char i) {
	if (i == 0) return "<=144ns";
	else return "Unknown";
}

char *decode_pwr(unsigned char i) {
	if (i == 0) return "None";
	if (i == 1) return "+15W";
	if (i == 2) return "+30W";
	if (i == 3) return "+45W";
	if (i == 4) return "-1W";
	if (i == 5) return "-3W";
	if (i == 6) return "-6W";
	if (i == 7) return "-10W";
	else return "Error";
}

char *decode_port_status(unsigned char i) {
	if (i == 3) return "Connected to child node";
	if (i == 2) return "Connected to parent node";
	if (i == 1) return "Not connected";
	if (i == 0) return "Not present";
	return "Unknown";
}

int append_port_status(char *p, unsigned char port_status,
	unsigned char port_number) {
	if (port_status == 0) return 0;
	sprintf(p, "Port %i: %s\n", port_number,
		decode_port_status(port_status));
	return strlen(p);
}

char *decode_all_ports_status(SelfIdPacket_t *selfid) {
	static char buf[34*3+34*8*3+1]; /* 3 Ports + 3*8 Ports + '\0' */
	int i = 0;
	char *p = buf;
	p += append_port_status(p, selfid->packetZero.port0, i++);
	p += append_port_status(p, selfid->packetZero.port1, i++);
	p += append_port_status(p, selfid->packetZero.port2, i++);
	if (selfid->packetZero.morePackets == 0) return buf;
	selfid++;
	p += append_port_status(p, selfid->packetMore.portA, i++);
	p += append_port_status(p, selfid->packetMore.portB, i++);
	p += append_port_status(p, selfid->packetMore.portC, i++);
	p += append_port_status(p, selfid->packetMore.portD, i++);
	p += append_port_status(p, selfid->packetMore.portE, i++);
	p += append_port_status(p, selfid->packetMore.portF, i++);
	p += append_port_status(p, selfid->packetMore.portG, i++);
	p += append_port_status(p, selfid->packetMore.portH, i++);
	if (selfid->packetMore.morePackets == 0) return buf;
	selfid++;
	p += append_port_status(p, selfid->packetMore.portA, i++);
	p += append_port_status(p, selfid->packetMore.portB, i++);
	p += append_port_status(p, selfid->packetMore.portC, i++);
	p += append_port_status(p, selfid->packetMore.portD, i++);
	p += append_port_status(p, selfid->packetMore.portE, i++);
	p += append_port_status(p, selfid->packetMore.portF, i++);
	p += append_port_status(p, selfid->packetMore.portG, i++);
	p += append_port_status(p, selfid->packetMore.portH, i++);
	if (selfid->packetMore.morePackets == 0) return buf;
	selfid++;
	p += append_port_status(p, selfid->packetMore.portA, i++);
	p += append_port_status(p, selfid->packetMore.portB, i++);
	p += append_port_status(p, selfid->packetMore.portC, i++);
	p += append_port_status(p, selfid->packetMore.portD, i++);
	p += append_port_status(p, selfid->packetMore.portE, i++);
	p += append_port_status(p, selfid->packetMore.portF, i++);
	p += append_port_status(p, selfid->packetMore.portG, i++);
	p += append_port_status(p, selfid->packetMore.portH, i++);
	return buf;
}

void print_selfid(SelfIdPacket_t *selfid) {
	printf("Physical ID:\t%i (0x%x)\n",selfid->packetZero.phyID,selfid->packetZero.phyID);
	printf("  Link active:\t%s\n",yes_no(selfid->packetZero.linkActive));
	printf("  Gap Count:\t%i\n",selfid->packetZero.gapCount);
	printf("  PHY Speed:\t%s\n",decode_speed(selfid->packetZero.phySpeed));
	printf("  PHY Delay:\t%s\n",decode_delay(selfid->packetZero.phyDelay));
	printf("  IRM Capable:\t%s\n",yes_no(selfid->packetZero.contender));
	printf("  Power Class:\t%s\n",decode_pwr(selfid->packetZero.powerClass));
	printf("  Port 0:\t%s\n",decode_port_status(selfid->packetZero.port0));
	printf("  Port 1:\t%s\n",decode_port_status(selfid->packetZero.port1));
	printf("  Port 2:\t%s\n",decode_port_status(selfid->packetZero.port2));
	printf("  Init. reset:\t%s\n",yes_no(selfid->packetZero.initiatedReset));
}

