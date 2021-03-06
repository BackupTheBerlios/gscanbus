/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * simpleavc.h - Linux IEEE-1394 Subsystem AV/C routines
 * These routines are very basic. They can only be used for sending simple
 * commands to AV/C equipment. No control of Input and Output Plugs, etc. is
 * provided.
 * Written 8.12.1999 - 14.1.2000 by Andreas Micklei
 * 14.1.2000: added AVC constants and macros
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

#ifndef __SIMPLEAVC_H__
#define __SIMPLEAVC_H__
#include <libraw1394/raw1394.h>
#include <libraw1394/csr.h>
#include "debug.h"
#include "raw1394util.h"

/* FCP Register Space */
#define FCP_COMMAND_ADDR 0xFFFFF0000B00
#define FCP_RESPONSE_ADDR 0xFFFFF0000D00

/* AV/C Mask macros */
#define AVC_MASK_START(x) ((x) & 0xF0000000)
#define AVC_MASK_CTYPE(x) ((x) & 0x0F000000)
#define AVC_MASK_RESPONSE(x) ((x) & 0x0F000000)
#define AVC_MASK_SUBUNIT_TYPE(x) ((x) & 0x00F80000)
#define AVC_MASK_SUBUNIT_ID(x) ((x) & 0x00070000)
#define AVC_MASK_OPCODE(x) ((x) & 0x0000FF00)
#define AVC_MASK_OPERAND0(x) ((x) & 0x000000FF)
#define AVC_MASK_OPERAND(x, n) ((x) & (0xFF000000 >> ((((n)-1)%4)*8)))
#define AVC_MASK_RESPONSE_OPERAND(x, n) ((x) & (0xFF000000 >> (((n)%4)*8)))

/* AV/C Mask and shift macros */
#define AVC_GET_CTYPE(x) (((x) & 0x0F000000) >> 24)
#define AVC_GET_RESPONSE(x) (((x) & 0x0F000000) >> 24)
#define AVC_GET_SUBUNIT_TYPE(x) (((x) & 0x00F80000) >> 19)
#define AVC_GET_SUBUNIT_ID(x) (((x) & 0x00070000) >> 16)
#define AVC_GET_OPCODE(x) (((x) & 0x0000FF00) >> 8)
#define AVC_GET_OPERAND0(x) ((x) & 0x000000FF)
#define AVC_GET_OPERAND(x, n) (((x) & (0xFF000000 >> ((((n)-1)%4)*8))) >> ((((n)-1)%4)*8))
#define AVC_GET_RESPONSE_OPERAND(x, n) (((x) & (0xFF000000 >> (((n)%4)*8))) >> (((n)%4)*8))

/* AV/C command types */
#define AVC_CTYPE_CONTROL 0x00000000
#define AVC_CTYPE_STATUS 0x01000000
#define AVC_CTYPE_SPECIFIC_INQUIRY 0x02000000
#define AVC_CTYPE_NOTIFY 0x03000000
#define AVC_CTYPE_GENERAL_INQUIRY 0x04000000

/* AV/C response codes */
#define AVC_RESPONSE_NOT_IMPLEMENTED 0x08000000
#define AVC_RESPONSE_ACCEPTED 0x09000000
#define AVC_RESPONSE_REJECTED 0x0A000000
#define AVC_RESPONSE_IN_TRANSITION 0x0B000000
#define AVC_RESPONSE_IMPLEMENTED 0x0C000000
#define AVC_RESPONSE_STABLE 0x0C000000
#define AVC_RESPONSE_CHANGED 0x0D000000
#define AVC_RESPONSE_INTERIM 0x0F000000

/* AV/C subunit types */
#define AVC_SUBUNIT_TYPE_VIDEO_MONITOR (0 <<19)
#define AVC_SUBUNIT_TYPE_DISC_RECORDER (3 <<19)
#define AVC_SUBUNIT_TYPE_TAPE_RECORDER (4 <<19)
#define AVC_SUBUNIT_TYPE_TUNER (5 <<19)
#define AVC_SUBUNIT_TYPE_VIDEO_CAMERA (7 <<19)
#define AVC_SUBUNIT_TYPE_VENDOR_UNIQUE (0x1C <<19)
#define AVC_SUBUNIT_TYPE_EXTENDED (0x1E <<19)	/* Not implemented */
#define AVC_SUBUNIT_TYPE_UNIT (0x1F <<19)

/* AV/C subunit IDs */
#define AVC_SUBUNIT_ID_0 (0 << 16)
#define AVC_SUBUNIT_ID_1 (1 << 16)
#define AVC_SUBUNIT_ID_2 (2 << 16)
#define AVC_SUBUNIT_ID_3 (3 << 16)
#define AVC_SUBUNIT_ID_4 (4 << 16)
#define AVC_SUBUNIT_ID_EXTENDED (5 <<16)	/* Not implemented */
#define AVC_SUBUNIT_ID_IGNORE (7 << 16)

/* AV/C Unit commands */
#define AVC_COMMAND_CHANNEL_USAGE 0x00001200
#define AVC_COMMAND_CONNECT 0x00002400
#define AVC_COMMAND_CONNECT_AV 0x00002000
#define AVC_COMMAND_CONNECTIONS 0x00002200
#define AVC_COMMAND_DIGITAL_INPUT 0x00001100
#define AVC_COMMAND_DIGITAL_OUTPUT 0x00001000
#define AVC_COMMAND_DISCONNECT 0x00002500
#define AVC_COMMAND_DISCONNECT_AV 0x00002100
#define AVC_COMMAND_INPUT_PLUG_SIGNAL_FORMAT 0x00001900
#define AVC_COMMAND_OUTPUT_PLUG_SIGNAL_FORMAT 0x00001800
#define AVC_COMMAND_SUBUNIT_INFO 0x00003100
#define AVC_COMMAND_UNIT_INFO 0x00003000

/* AV/C Common unit and subunit commands */
#define AVC_COMMAND_OPEN_DESCRIPTOR 0x00000800
#define AVC_COMMAND_READ_DESCRIPTOR 0x00000900
#define AVC_COMMAND_WRITE_DESCRIPTOR 0x00000A00
#define AVC_COMMAND_SEARCH_DESCRIPTOR 0x00000B00
#define AVC_COMMAND_OBJECT_NUMBER_SELECT 0x00000D00
#define AVC_COMMAND_POWER 0x0000B200
#define AVC_COMMAND_RESERVE 0x00000100
#define AVC_COMMAND_PLUG_INFO 0x00000200
#define AVC_COMMAND_VENDOR_DEPENDENT 0x00000000

/* AV/C Common unit and subunit command operands */
#define AVC_OPERAND_DESCRIPTOR_TYPE_SUBUNIT_IDENTIFIER_DESCRIPTOR 0x00
#define AVC_OPERAND_DESCRIPTOR_TYPE_OBJECT_LIST_DESCRIPTOR_ID 0x10
#define AVC_OPERAND_DESCRIPTOR_TYPE_OBJECT_LIST_DESCRIPTOR_TYPE 0x11
#define AVC_OPERAND_DESCRIPTOR_TYPE_OBJECT_ENTRY_DESCRIPTOR_POSITION 0x20
#define AVC_OPERAND_DESCRIPTOR_TYPE_OBJECT_ENTRY_DESCRIPTOR_ID 0x21
#define AVC_OPERAND_DESCRIPTOR_SUBFUNCTION_CLOSE 0x00
#define AVC_OPERAND_DESCRIPTOR_SUBFUNCTION_READ_OPEN 0x01
#define AVC_OPERAND_DESCRIPTOR_SUBFUNCTION_WRITE_OPEN 0x03

/* VCR subunit commands (Alphabetically) */
#define VCR_COMMAND_ANALOG_AUDIO_OUTPUT_MODE 0x000007000
#define VCR_COMMAND_AREA_MODE 0x000007200
#define VCR_COMMAND_ABSOLUTE_TRACK_NUMBER 0x000005200
#define VCR_COMMAND_AUDIO_MODE 0x000007100
#define VCR_COMMAND_BACKWARD 0x000005600
#define VCR_COMMAND_BINARY_GROUP 0x000005A00
#define VCR_COMMAND_EDIT_MODE 0x000004000
#define VCR_COMMAND_FORWARD 0x000005500
#define VCR_COMMAND_INPUT_SIGNAL_MODE 0x000007900
#define VCR_COMMAND_LOAD_MEDIUM 0x00000C100
#define VCR_COMMAND_MARKER 0x00000CA00
#define VCR_COMMAND_MEDIUM_INFO 0x00000DA00
#define VCR_COMMAND_OPEN_MIC 0x000006000
#define VCR_COMMAND_OUTPUT_SIGNAL_MODE 0x000007800
#define VCR_COMMAND_PLAY 0x00000C300
#define VCR_COMMAND_PRESET 0x000004500
#define VCR_COMMAND_READ_MIC 0x000006100
#define VCR_COMMAND_RECORD 0x00000C200
#define VCR_COMMAND_RECORDING_DATE 0x000005300
#define VCR_COMMAND_RECORDING_SPEED 0x00000DB00
#define VCR_COMMAND_RECORDING_TIME 0x000005400
#define VCR_COMMAND_RELATIVE_TIME_COUNTER 0x000005700
#define VCR_COMMAND_SEARCH_MODE 0x000005000
#define VCR_COMMAND_SMPTE_EBU_RECORDING_TIME 0x000005C00
#define VCR_COMMAND_SMPTE_EBU_TIME_CODE 0x000005900
#define VCR_COMMAND_TAPE_PLAYBACK_FORMAT 0x00000D300
#define VCR_COMMAND_TAPE_RECORDING_FORMAT 0x00000D200
#define VCR_COMMAND_TIME_CODE 0x000005100
#define VCR_COMMAND_TRANSPORT_STATE 0x00000D000
#define VCR_COMMAND_WIND 0x00000C400
#define VCR_COMMAND_WRITE_MIC 0x000006200

/* VCR subunit command operands */
#define VCR_OPERAND_LOAD_MEDIUM_EJECT 0x60
#define VCR_OPERAND_LOAD_MEDIUM_OPEN_TRAY 0x31
#define VCR_OPERAND_LOAD_MEDIUM_CLOSE_TRAY 0x32

#define VCR_OPERAND_PLAY_NEXT_FRAME 0x30
#define VCR_OPERAND_PLAY_SLOWEST_FORWARD 0x31
#define VCR_OPERAND_PLAY_FAST_FORWARD_1 0x39
#define VCR_OPERAND_PLAY_FAST_FORWARD_2 0x3A
#define VCR_OPERAND_PLAY_FAST_FORWARD_3 0x3B
#define VCR_OPERAND_PLAY_FAST_FORWARD_4 0x3C
#define VCR_OPERAND_PLAY_FAST_FORWARD_5 0x3D
#define VCR_OPERAND_PLAY_FAST_FORWARD_6 0x3E
#define VCR_OPERAND_PLAY_FASTEST_FORWARD 0x3F
#define VCR_OPERAND_PLAY_PREVIOUS_FRAME 0x40
#define VCR_OPERAND_PLAY_SLOWEST_REVERSE 0x41
#define VCR_OPERAND_PLAY_FAST_REVERSE_1 0x49
#define VCR_OPERAND_PLAY_FAST_REVERSE_2 0x4A
#define VCR_OPERAND_PLAY_FAST_REVERSE_3 0x4B
#define VCR_OPERAND_PLAY_FAST_REVERSE_4 0x4C
#define VCR_OPERAND_PLAY_FAST_REVERSE_5 0x4D
#define VCR_OPERAND_PLAY_FAST_REVERSE_6 0x4E
#define VCR_OPERAND_PLAY_FASTEST_REVERSE 0x4F
#define VCR_OPERAND_PLAY_FORWARD 0x75
#define VCR_OPERAND_PLAY_FORWARD_PAUSE 0x7D

#define VCR_OPERAND_RECORD_RECORD 0x75
#define VCR_OPERAND_RECORD_PAUSE 0x7D

#define VCR_OPERAND_TRANSPORT_STATE 0x7F

#define VCR_RESPONSE_TRANSPORT_STATE_LOAD_MEDIUM 0x0000C100
#define VCR_RESPONSE_TRANSPORT_STATE_RECORD 0x0000C200
#define VCR_RESPONSE_TRANSPORT_STATE_PLAY 0x0000C300
#define VCR_RESPONSE_TRANSPORT_STATE_WIND 0x0000C400

#define VCR_OPERAND_WIND_HIGH_SPEED_REWIND 0x45
#define VCR_OPERAND_WIND_STOP 0x60
#define VCR_OPERAND_WIND_REWIND 0x65
#define VCR_OPERAND_WIND_FAST_FORWARD 0x75

#define VCR_OPERAND_RELATIVE_TIME_COUNTER_CONTROL 0x20
#define VCR_OPERAND_RELATIVE_TIME_COUNTER_STATUS 0x71

#define VCR_OPERAND_TIME_CODE_CONTROL 0x20
#define VCR_OPERAND_TIME_CODE_STATUS 0x71

#define VCR_OPERAND_TRANSPORT_STATE 0x7F

#define VCR_OPERAND_RECORDING_TIME_STATUS 0x71

/* Tuner subunit commands (Alphabetically) */
#define TUNER_COMMAND_DIRECT_SELECT_INFORMATION_TYPE 0xC8
#define TUNER_COMMAND_DIRECT_SELECT_DATA 0xCB
#define TUNER_COMMAND_CA_ENABLE 0xCC
#define TUNER_COMMAND_TUNER_STATUS 0xCD

/* Tuner subunit commands operands (Alphabetically) */
#define TUNER_COMMAND_DIRECT_SELECT_INFORMATION_TYPE 0xC8
#define TUNER_COMMAND_DIRECT_SELECT_DATA 0xCB
#define TUNER_COMMAND_CA_ENABLE 0xCC
#define TUNER_COMMAND_TUNER_STATUS 0xCD

/* deprecated */
#if 0
#define DVCR_PLAY 0x0020C375
#define DVCR_PLAY_FF 0x0020C33F
#define DVCR_PLAY_REWIND 0x0020C34F
#define DVCR_PLAY_PAUSE 0x0020C37D
#define DVCR_PLAY_SLOWEST_FORWARD 0x0020C331

#define DVCR_LOAD_EJECT 0x0020C160

#define DVCR_WIND_HIGH_SPEED_REWIND 0x0020C445
#define DVCR_WIND_STOP 0x0020C460
#define DVCR_WIND_REWIND 0x0020C465
#define DVCR_WIND_FAST_FORWARD 0x0020C475

#define DVCR_STATUS_TRANSPORT_STATE 0x0120D07F

#define DVCR_TRANSPORT_STATE_LOAD_MEDIUM 0x0000C100
#define DVCR_TRANSPORT_STATE_RECORD 0x0000C200
#define DVCR_TRANSPORT_STATE_PLAY 0x0000C300
#define DVCR_TRANSPORT_STATE_WIND 0x0000C400

#define DVCR_TRANSPORT_STATE_PLAY_FORWARD 0x0000C375

#define DVCR_RELATIVE_TIME_COUNTER
#endif

int send_avc_command(raw1394handle_t handle, nodeid_t node, quadlet_t command);

int send_avc_command_block(raw1394handle_t handle, nodeid_t node,
	quadlet_t *command, int command_len);

quadlet_t get_avc_response(raw1394handle_t handle);

quadlet_t *get_avc_response_block(raw1394handle_t handle, quadlet_t *buf,
	int len);

quadlet_t avc_transaction(raw1394handle_t handle, nodeid_t node,
	quadlet_t quadlet, int retry);

quadlet_t *avc_transaction_block(raw1394handle_t handle, nodeid_t node,
	quadlet_t *buf, int len, int retry);

int avc_open_descriptor(raw1394handle_t handle, nodeid_t node,
	quadlet_t ctype, quadlet_t subunit,
	unsigned char *descriptor_identifier, int len_descriptor_identifier,
	unsigned char readwrite);

int avc_close_descriptor(raw1394handle_t handle, nodeid_t node,
	quadlet_t ctype, quadlet_t subunit,
	unsigned char *descriptor_identifier, int len_descriptor_identifier);

int avc_subunit_info(raw1394handle_t handle, nodeid_t node, quadlet_t *table);

quadlet_t *avc_unit_info(raw1394handle_t handle, nodeid_t node);

char *avc_decode_vcr_response(quadlet_t response);

#endif

