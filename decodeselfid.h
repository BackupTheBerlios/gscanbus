/* 
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * decodeselfid.h
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

#ifndef __DECODESELFID_H__
#define __DECODESELFID_H__

#include "raw1394support.h"
#include "debug.h"
#include <libraw1394/raw1394.h>
#include <stdio.h>

#define SHIFT_START	30
#define WIDTH_START	2
#define SHIFT_PHY_ID	24
#define WIDTH_PHY_ID	6
#define SHIFT_CONT	23
#define WIDTH_CONT	1
#define SHIFT_L		22
#define WIDTH_L		1
#define SHIFT_GAP_CNT	16
#define WIDTH_GAP_CNT	6
#define SHIFT_SP	14
#define WIDTH_SP	2
#define SHIFT_DEL	12
#define WIDTH_DEL	2
#define SHIFT_C		11
#define WIDTH_C		1
#define SHIFT_PWR	8
#define WIDTH_PWR	3
#define SHIFT_P0	6
#define WIDTH_P0	2
#define SHIFT_P1	4
#define WIDTH_P1	2
#define SHIFT_P2	2
#define WIDTH_P2	2
#define SHIFT_I		1
#define WIDTH_I		1
#define SHIFT_M		0
#define WIDTH_M		1

#define SHIFT_N		20
#define WIDTH_N		3
#define SHIFT_RSV	18
#define WIDTH_RSV	2
#define SHIFT_PA	16
#define WIDTH_PA	2
#define SHIFT_PB	14
#define WIDTH_PB	2
#define SHIFT_PC	12
#define WIDTH_PC	2
#define SHIFT_PD	10
#define WIDTH_PD	2
#define SHIFT_PE	8
#define WIDTH_PE	2
#define SHIFT_PF	6
#define WIDTH_PF	2
#define SHIFT_PG	4
#define WIDTH_PG	2
#define SHIFT_PH	2
#define WIDTH_PH	2
#define SHIFT_R		1
#define WIDTH_R		1

void printbin(FILE *stream, unsigned int i, unsigned char width);

int decode_selfid(SelfIdPacket_t *selfid, unsigned int *selfid_raw);

char *yes_no(unsigned char i);

char *decode_speed(unsigned char i);

char *decode_delay(unsigned char i);

char *decode_pwr(unsigned char i);

char *decode_port_status(unsigned char i);

char *decode_all_ports_status(SelfIdPacket_t *selfid);

void print_selfid(SelfIdPacket_t *selfid);

#endif

