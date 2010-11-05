/* $Id: simpleavc.c,v 1.9 2001/07/11 10:53:51 ami Exp $
 *
 * simpleavc.c - Linux IEEE-1394 Subsystem AV/C routines
 * These routines are very basic. They can only be used for sending simple
 * commands to AV/C equipment. No control of Input and Output Plugs, etc. is
 * provided.
 * Written 8.12.1999 - 22.5.2000 by Andreas Micklei
 * 14.1.2000: added block operations
 * 6.4.2000: adapted to new fcp handling for libraw1394 0.6
 *           avc_transaction() and avc_transaction_block() are much cleaner
 *           now thanks to the new fcp handling. get_avc_response() and
 *           get_avc_response_block() are broken at the moment and will
 *           probably bee removed.
 * 22.5.2000: fixed block transactions
 *            added lots of defines and some new convenience functions for
 *            special operations like AV/C descriptor processing
 * 4.10.2000: switched to cooked functions from raw1394util
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

#include "simpleavc.h"

/* For select() */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <netinet/in.h>

#include <stdio.h>	//DEBUG


#define MAX_RESPONSE_SIZE 512
unsigned char fcp_response[MAX_RESPONSE_SIZE];

void htonl_block(quadlet_t *buf, int len) {
	int i;
	for (i=0; i<len; i++) {
		buf[i] = htonl(buf[i]);
	}
}

void ntohl_block(quadlet_t *buf, int len) {
	int i;
	for (i=0; i<len; i++) {
		buf[i] = ntohl(buf[i]);
	}
}

char *decode_response(quadlet_t response) {
	quadlet_t resp = AVC_MASK_RESPONSE(response);
	if (resp == AVC_RESPONSE_NOT_IMPLEMENTED) return "NOT IMPLEMENTED";
	if (resp == AVC_RESPONSE_ACCEPTED) return "ACCEPTED";
	if (resp == AVC_RESPONSE_REJECTED) return "REJECTED";
	if (resp == AVC_RESPONSE_IN_TRANSITION) return "IN TRANSITION";
	if (resp == AVC_RESPONSE_IMPLEMENTED) return "IMPLEMENTED / STABLE";
	if (resp == AVC_RESPONSE_CHANGED) return "CHANGED";
	if (resp == AVC_RESPONSE_INTERIM) return "INTERIM";
	return "huh?";
}

int avc_fcp_handler(raw1394handle_t handle, nodeid_t nodeid, int response,
                   size_t length, unsigned char *data)
{
	DEBUG_LOWLEVEL {
		unsigned char *pdata = data;
		size_t length2 = length;
        	fprintf(stderr,
			"fcp_response: got fcp %s from node %d of %d bytes:",
               	(response ? "response" : "command"), nodeid & 0x3f, length);

        	while (length2--) fprintf(stderr, " %02x", *pdata++);

        	fprintf(stderr, "\n");
	}

	if (response) {
		memcpy(fcp_response, data, length);
	}

	return 0;
}

void init_avc_response_handler(raw1394handle_t handle) {
	memset(fcp_response, 0, MAX_RESPONSE_SIZE);
	raw1394_set_fcp_handler(handle, avc_fcp_handler);
	raw1394_start_fcp_listen(handle);
}

void stop_avc_response_handler(raw1394handle_t handle) {
	raw1394_stop_fcp_listen(handle);
}

int send_avc_command(raw1394handle_t handle, nodeid_t node, quadlet_t command) {
	quadlet_t cmd = htonl(command);
	DEBUG_LOWLEVEL fprintf(stderr,
		"AV/C request: node: %i, Command: 0x%08x\n", node, command);

	return cooked1394_write(handle, 0xffc0 | node, FCP_COMMAND_ADDR,
		4, &cmd);
}

int send_avc_command_block(raw1394handle_t handle, nodeid_t node,
	quadlet_t *command, int command_len) {
	
	DEBUG_LOWLEVEL {
		int i;
		fprintf(stderr, "send_avc_command_block: ");
		for (i=0; i<command_len; i++)
			fprintf(stderr, " 0x%08X", command[i]);
		fprintf(stderr, "\n");
	}

	htonl_block(command, command_len);

	return cooked1394_write(handle, 0xffc0 | node, FCP_COMMAND_ADDR,
		command_len*4, command);
}

/*
 * Send an AV/C request to a device, wait for the corresponding AV/C
 * response and return that. This version only uses quadlet transactions.
 * IN:		handle:		the libraw1394 handle
 *		node:		the phyisical ID of the node
 *		quadlet: 	the FCP request to send
 *		retry:		retry sending the request this many times
 * RETURNS:	the AV/C response if everything went well, -1 in case of an
 * 		error
 */
quadlet_t avc_transaction(raw1394handle_t handle, nodeid_t node,
	quadlet_t quadlet, int retry) {

	quadlet_t response;

	init_avc_response_handler(handle);

	do {
		if (send_avc_command(handle, node, quadlet) < 0) {
			fprintf(stderr,"send oops\n");
			usleep(10);
			continue;
		}

		raw1394_loop_iterate(handle);
		response = ntohl(*((quadlet_t *)fcp_response));
		while ((response & 0x0F000000) == 0x0F000000) {
			fprintf(stderr,"INTERIM\n");
			raw1394_loop_iterate(handle);
			response = ntohl(*((quadlet_t *)fcp_response));
		}
		stop_avc_response_handler(handle);
		/*fprintf(stderr, "avc_transaction: Got AVC response 0x%0x (%s)\n", response, decode_response(response));*/
		return response;
	} while (--retry >= 0);

	stop_avc_response_handler(handle);

	return -1;
}

/*
 * Send an AV/C request to a device, wait for the corresponding AV/C
 * response and return that. This version uses block transactions.
 * IN:		handle:		the libraw1394 handle
 *		node:		the phyisical ID of the node
 *		buf:	 	the FCP request to send
 *		len:		the length of the FCP request
 *		retry:		retry sending the request this many times
 * RETURNS:	the AV/C response if everything went well, NULL in case of an
 * 		error. The response always has the same length as the request.
 */
quadlet_t *avc_transaction_block(raw1394handle_t handle, nodeid_t node,
	quadlet_t *buf, int len, int retry) {

	quadlet_t *response;
	/*int i;*/

	init_avc_response_handler(handle);

	do {
		if (send_avc_command_block(handle, node, buf, len) < 0) {
			fprintf(stderr,"send oops\n");
			usleep(10);
			continue;
		}

		raw1394_loop_iterate(handle);
		response = (quadlet_t *)fcp_response;
		while ((response[0] & 0x0F000000) == 0x0F000000) {
			fprintf(stderr,"INTERIM\n");
			raw1394_loop_iterate(handle);
			response = (quadlet_t *)fcp_response;
		}
		stop_avc_response_handler(handle);
		ntohl_block(response, len);
		/*fprintf(stderr, "avc_transaction_block received response: ");
		for (i=0; i<len; i++) fprintf(stderr, " 0x%08X", response[i]);
		fprintf(stderr, " (%s)\n", decode_response(response[0]));*/
		return response;
	} while (--retry >= 0);
	stop_avc_response_handler(handle);
	return NULL;
}

/*---------------------
 * HIGH-LEVEL-FUNCTIONS
 * --------------------
 */

/*
 * Open an AV/C descriptor
 */
int avc_open_descriptor(raw1394handle_t handle, nodeid_t node,
	quadlet_t ctype, quadlet_t subunit,
	unsigned char *descriptor_identifier, int len_descriptor_identifier,
	unsigned char readwrite) {

	//quadlet_t request[2];
	quadlet_t request[2];
	quadlet_t *response;
	int i;
	unsigned char subfunction = readwrite?
		AVC_OPERAND_DESCRIPTOR_SUBFUNCTION_WRITE_OPEN
		:AVC_OPERAND_DESCRIPTOR_SUBFUNCTION_READ_OPEN;

	fprintf(stderr, "Open descriptor: ctype: 0x%08X, subunit:0x%08X,\n     descriptor_identifier:", ctype, subunit);
	for (i=0; i<len_descriptor_identifier; i++)
		fprintf(stderr, " 0x%02X", descriptor_identifier[i]);
	fprintf(stderr,"\n");
	if (len_descriptor_identifier != 1)
		fprintf(stderr, "Unimplemented.\n");
	/*request[0] = ctype | subunit | AVC_COMMAND_OPEN_DESCRIPTOR
		| ((*descriptor_identifier & 0xFF00) >> 16);
	request[1] = ((*descriptor_identifier & 0xFF) << 24) | subfunction;*/

	request[0] = ctype | subunit | AVC_COMMAND_OPEN_DESCRIPTOR
		| *descriptor_identifier;
	request[1] = subfunction << 24;
	if (ctype == AVC_CTYPE_STATUS) request[1] = 0xFF00FFFF;

	response = avc_transaction_block(handle, node, request, 2, 3);
	if (response == NULL) return -1;

	fprintf(stderr, "Open descriptor response: 0x%08X.\n", *response);
	return 0;
}

/*
 * Close an AV/C descriptor
 */
int avc_close_descriptor(raw1394handle_t handle, nodeid_t node,
	quadlet_t ctype, quadlet_t subunit,
	unsigned char *descriptor_identifier, int len_descriptor_identifier) {

	quadlet_t request[2];
	quadlet_t *response;
	int i;
	unsigned char subfunction = AVC_OPERAND_DESCRIPTOR_SUBFUNCTION_CLOSE;

	fprintf(stderr, "Close descriptor: ctype: 0x%08X, subunit:0x%08X,\n      descriptor_identifier:", ctype, subunit);
	for (i=0; i<len_descriptor_identifier; i++)
		fprintf(stderr, " 0x%02X", descriptor_identifier[i]);
	fprintf(stderr,"\n");
	if (len_descriptor_identifier != 1)
		fprintf(stderr, "Unimplemented.\n");
	/*request[0] = ctype | subunit | AVC_COMMAND_OPEN_DESCRIPTOR
		| ((*descriptor_identifier & 0xFF00) >> 16);
	request[1] = ((*descriptor_identifier & 0xFF) << 24) | subfunction;*/

	request[0] = ctype | subunit | AVC_COMMAND_OPEN_DESCRIPTOR
		| *descriptor_identifier;
	request[1] = subfunction << 24;

	response = avc_transaction_block(handle, node, request, 2, 3);
	if (response == NULL) return -1;

	fprintf(stderr, "Close descriptor response: 0x%08X.\n", *response);
	return 0;
}

/*
 * Read an entire AV/C descriptor
 */
unsigned char *avc_read_descriptor(raw1394handle_t handle, nodeid_t node,
	quadlet_t subunit,
	unsigned char *descriptor_identifier, int len_descriptor_identifier) {

	quadlet_t request[128];
	quadlet_t *response;

	if (len_descriptor_identifier != 1)
		fprintf(stderr, "Unimplemented.\n");

	memset(request, 0, 128*4);
	request[0] = AVC_CTYPE_CONTROL | subunit | AVC_COMMAND_READ_DESCRIPTOR
		| *descriptor_identifier;
	request[1] = 0xFF000000;	/* read entire descriptor */
	request[2] = 0x00000000;	/* beginning from 0x0000 */

	response = avc_transaction_block(handle, node, request, 3, 3);
	if (response == NULL) return NULL;

	return (unsigned char *) response;
}

/*
 * Get subunit info
 */
#define EXTENSION_CODE 7
int avc_subunit_info(raw1394handle_t handle, nodeid_t node, quadlet_t *table) {

	quadlet_t request[2];
	quadlet_t *response;
	int page;

	for (page=0; page < 8; page++) {
		request[0] = AVC_CTYPE_STATUS | AVC_SUBUNIT_TYPE_UNIT
			| AVC_SUBUNIT_ID_IGNORE | AVC_COMMAND_SUBUNIT_INFO
			| page << 4 | EXTENSION_CODE;
		request[1] = 0xFFFFFFFF;
		response = avc_transaction_block(handle, node, request, 2, 3);
		if (response == NULL) return -1;
		table[page] = response[1];
	}

	/*fprintf(stderr, "avc_subunit_info:");
	for (page=0; page < 8; page++) fprintf(stderr, " 0x%08X", table[page]);
	fprintf(stderr, "\n");*/

	return 0;
}

quadlet_t *avc_unit_info(raw1394handle_t handle, nodeid_t node) {

	quadlet_t request[2];
	quadlet_t *response;

	request[0] = AVC_CTYPE_STATUS | AVC_SUBUNIT_TYPE_UNIT
		| AVC_SUBUNIT_ID_IGNORE | AVC_COMMAND_UNIT_INFO | 0xFF;
	request[1] = 0xFFFFFFFF;
	response = avc_transaction_block(handle, node, request, 2, 3);
	if (response == NULL) return NULL;

	fprintf(stderr, "avc_unit_info: 0x%08X 0x%08X\n",
		response[0], response[1]);

	return response;
}

char *avc_decode_vcr_response(quadlet_t response) {
	/*quadlet_t resp0 = AVC_MASK_RESPONSE_OPERAND(response, 0);
	quadlet_t resp1 = AVC_MASK_RESPONSE_OPERAND(response, 1);*/
	quadlet_t resp2 = AVC_MASK_RESPONSE_OPERAND(response, 2);
	quadlet_t resp3 = AVC_MASK_RESPONSE_OPERAND(response, 3);

	if (response == 0) {
		return "OK";
	} else if (resp2 == VCR_RESPONSE_TRANSPORT_STATE_LOAD_MEDIUM) {
		return("Loading Medium");
	} else if (resp2 == VCR_RESPONSE_TRANSPORT_STATE_RECORD) {
		return("Recording");
	} else if (resp2 == VCR_RESPONSE_TRANSPORT_STATE_PLAY) {
		if (resp3 >= VCR_OPERAND_PLAY_FAST_FORWARD_1
			&& resp3 <= VCR_OPERAND_PLAY_FASTEST_FORWARD) {
			return("Playing Fast Forward");
		} else if (resp3 >= VCR_OPERAND_PLAY_FAST_REVERSE_1
				&& resp3 <= VCR_OPERAND_PLAY_FASTEST_REVERSE) {
			return("Playing Reverse");
		} else if (resp3 == VCR_OPERAND_PLAY_FORWARD_PAUSE) {
			return("Playing Paused");
		} else {
			return("Playing");
		}
	} else if (resp2 == VCR_RESPONSE_TRANSPORT_STATE_WIND) {
		if (resp3 == VCR_OPERAND_WIND_HIGH_SPEED_REWIND) {
			return("Winding backward at incredible speed");
		} else if (resp3 == VCR_OPERAND_WIND_STOP) {
			return("Winding stopped");
		} else if (resp3 == VCR_OPERAND_WIND_REWIND) {
			return("Winding reverse");
		} else if (resp3 == VCR_OPERAND_WIND_FAST_FORWARD) {
			return("Winding forward");
		} else {
			return("Winding");
		}
	} else {
		return("Unknown");
	}
}
