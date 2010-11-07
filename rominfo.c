/* $Id: rominfo.c,v 1.15 2001/07/11 10:53:51 ami Exp $
 *
 * rominfo.c - Linux IEEE-1394 Subsystem CSR ROM info reading routines
 * written 23.11.1999 - 24.11. 1999 by Andreas Micklei
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

#include "rominfo.h"
#include <netinet/in.h>
#define MAXLINE 80
#define GUIDFILENAME0 "guid-resolv.conf"
#define GUIDFILENAME1 SYSCONFDIR "/guid-resolv.conf"
#define GUIDERROR "Error while opening guid-resolv.conf"
#define OUIFILENAME0 "oui-resolv.conf"
#define OUIFILENAME1 SYSCONFDIR "/oui-resolv.conf"
#define OUIERROR "Error while opening oui-resolv.conf"

#define QUADINC(x) x = x + 4
#define WARN(s, phyID, adr) fprintf(stderr,"%i/0x%08x%08x: %s\n",phyID,(int) (adr>>32), (int) adr,s)
#define QUADREADERR(handle, phyID, offset, buf) if(cooked1394_read(handle, 0xffc0 | phyID, offset, 4, buf) < 0) WARN("read failed", phyID, offset);


/*
 * Fill out the structure with zeroes
 * IN:  rom_info:	Pointer to the unitiliazied structure
 */
void init_rom_info(Rom_info *rom_info) {
	rom_info->magic = 0;
	rom_info->irmc = 0;
	rom_info->cmc = 0;
	rom_info->isc = 0;
	rom_info->bmc = 0;
	rom_info->cyc_clk_acc = 0;
	rom_info->max_rec = 0;
	rom_info->guid_hi = 0;
	rom_info->guid_lo = 0;
	rom_info->node_capabilities = 0;
	rom_info->vendor_id = 0;
	rom_info->unit_spec_id = 0;
	rom_info->unit_sw_version = 0;
	rom_info->model_id = 0;
	rom_info->nr_textual_leafes = 0;
	rom_info->textual_leafes = NULL;
	rom_info->label = NULL;
	rom_info->vendor = NULL;
}

int check_guid_line(char *s) {
	if (s == NULL) return 0;
	if (s[0] == '#') return 0;
	return 1;
}

/*
 * Resolve a guid into a name from the configuration file. Read in the file on
 * first invocation
 * IN:		guid_hi:	High quadlet of GUI
 * 		guid_lo:	Low quadlet of GUI
 * OUT:		cpu:		1 if device is a CPU, 0 otherwise
 * RETURNS:	Pointer to the description string
 */
char *resolv_guid(int guid_hi, int guid_lo, char *cpu) {
	static int nr_guids = -1;
	static int *guids_hi;
	static int *guids_lo;
	static char **descriptions;
	static char *cpus;
	int *pguid_hi, *pguid_lo, i;
	FILE *file;
	char s[MAXLINE+1], description[MAXLINE+1], **pdescription, *pdesc,
		*pcpu, *pcpus;
	char *filename;

	/* read in descriptions on first call */
	if (nr_guids < 0) {
		filename = GUIDFILENAME0;
		file = fopen(filename, "r");
		DEBUG_GENERAL
			fprintf(stderr,"Opening \"%s\": ",filename);
		if (file == NULL) {
			filename = GUIDFILENAME1;
			DEBUG_GENERAL
				fprintf(stderr,"Error\nOpening \"%s\": ",
					filename);
			file = fopen(filename, "r");
		}
		if (file == NULL) {
			DEBUG_GENERAL
				fprintf(stderr,"Error\n");
			perror(GUIDERROR);
			nr_guids = 0;	/* Never try again */
			return NULL;
		}
		DEBUG_GENERAL
			fprintf(stderr,"OK\n");
		nr_guids = 0;
		/* Count guids */
		while (fgets(s, MAXLINE+1, file) != NULL) {
			if (check_guid_line(s)) nr_guids++;
		}
		DEBUG_GENERAL
			fprintf(stderr,"%s: %i lines\n",filename,nr_guids);
		/* Allocate memory */
		guids_hi = (int *) calloc(nr_guids, sizeof(int));
		if (!guids_hi) fatal("out of memory!");
		guids_lo = (int *) calloc(nr_guids, sizeof(int));
		if (!guids_lo) fatal("out of memory!");
		descriptions = (char **) calloc(nr_guids, sizeof(char *));
		if (!descriptions) fatal("out of memory!");
		cpus = (char *) calloc(nr_guids, sizeof(char));
		if (!cpus) fatal("out of memory!");
		rewind(file);
		/* read in guids */
		pguid_hi = guids_hi;
		pguid_lo = guids_lo;
		pdescription = descriptions;
		pcpus = cpus;
		while (fgets(s, MAXLINE+1, file) != NULL) {
			if (!check_guid_line(s)) continue;
			sscanf(s, "%8x%8x%[^\n]",
				pguid_hi++, pguid_lo++, description);
			pdesc = description;
			while (*pdesc == ' ' || *pdesc == '\t') pdesc++;
			pcpu = pdesc;
			while (*pcpu != '\t' && *pcpu != '\0') pcpu++;
			*pdescription = (char *) malloc((pcpu - pdesc) + 1);
			if (!*pdescription) fatal("out of memory!");
			strncpy(*pdescription, pdesc, pcpu - pdesc);
			*(*pdescription + (pcpu - pdesc)) = '\0';
			pdescription++;
			while (*pcpu == '\t' || *pcpu == ' ') pcpu++;
			*pcpus = *pcpu - '0';
			pcpus++;
			DEBUG_CONFIG fprintf(stderr,"%08x%08x_%s_%i\n",
				(unsigned) pguid_hi-1, (unsigned) pguid_lo-1,
				*(pdescription-1), *(pcpus-1));
		}
		fclose(file);
	}
	for (i=0; i<nr_guids; i++) {
		if (guids_hi[i] == guid_hi && guids_lo[i] == guid_lo) {
			*cpu = cpus[i];
			return descriptions[i];
		}
	}
	return NULL;
}

int check_oui_line(char *s) {
	return check_guid_line(s);
}

/*
 * Resolve a oui into a vendor name from the configuration file. Read in the
 * file on first invocation
 * IN:		oui:		vendor ID
 * RETURNS:	Pointer to the vendor name string
 */
char *resolv_oui(int oui) {
	static int nr_ouis = -1;
	static int *ouis;
	static char **descriptions;
	int *poui, i;
	FILE *file;
	char s[MAXLINE+1], description[MAXLINE+1], **pdescription;
	char *filename;

	/* read in descriptions on first call */
	if (nr_ouis < 0) {
		filename = OUIFILENAME0;
		DEBUG_GENERAL
			fprintf(stderr,"Opening \"%s\": ",filename);
		file = fopen(filename, "r");
		if (file == NULL) {
			filename = OUIFILENAME1;
			DEBUG_GENERAL
				fprintf(stderr,"Error\nOpening \"%s\": ",
					filename);
			file = fopen(filename, "r");
		}
		if (file == NULL) {
			DEBUG_GENERAL
				fprintf(stderr,"Error\n");
			perror(OUIERROR);
			nr_ouis = 0;	/* Never try again */
			return NULL;
		}
		DEBUG_GENERAL
			fprintf(stderr,"OK\n");
		nr_ouis = 0;
		/* Count ouis */
		while (fgets(s, MAXLINE+1, file) != NULL) {
			if (check_oui_line(s)) nr_ouis++;
		}
		DEBUG_GENERAL
			fprintf(stderr,"%s: %i lines\n",filename,nr_ouis);
		/* Allocate memory */
		ouis = (int *) calloc(nr_ouis, sizeof(int));
		if (!ouis) fatal("out of memory!");
		descriptions = (char **) calloc(nr_ouis, sizeof(char *));
		if (!descriptions) fatal("out of memory!");
		rewind(file);
		/* read in ouis */
		poui = ouis;
		pdescription = descriptions;
		while (fgets(s, MAXLINE+1, file) != NULL) {
			if (!check_oui_line(s)) continue;
			sscanf(s, "%6x%[^\n]",
				poui++, description);
			//pdesc = description;
			//while (*pdesc == ' ' || *pdesc == '\t') pdesc++;
			/*pcpu = pdesc;
			while (*pcpu != '\t' && *pcpu != '\0') pcpu++;
			*pdescription = (char *) malloc((pcpu - pdesc) + 1);
			if (!*pdescription) fatal("out of memory!");
			strncpy(*pdescription, pdesc, pcpu - pdesc);
			*(*pdescription + (pcpu - pdesc)) = '\0';
			pdescription++;
			while (*pcpu == '\t' || *pcpu == ' ') pcpu++;
			*pcpus = *pcpu - '0';
			pcpus++;*/
			*pdescription = (char *) malloc(strlen(description)+1);
			if (!*pdescription) fatal("out of memory!");
			strncpy(*pdescription,description,strlen(description));
			*(*pdescription + (strlen(description))) = '\0';
			pdescription++;
			DEBUG_CONFIG fprintf(stderr,"%06x_%s\n",
				(unsigned) poui-1, *(pdescription-1));
		}
		fclose(file);
	}
	for (i=0; i<nr_ouis; i++) {
		if (ouis[i] == oui) {
			return descriptions[i];
		}
	}
	return NULL;
}

/*
 * Get the type / protocol of a node
 * IN:		rom_info:	pointer to the Rom_info structure of the node
 * RETURNS:	one of the defined node types, i.e. NODE_TYPE_AVC, etc.
 */
int get_node_type(Rom_info *rom_info) {
	char cpu;
	if (rom_info->unit_spec_id == 0xA02D) {
		if ((rom_info->unit_sw_version == 0x100)||
		(rom_info->unit_sw_version == 0x101) ||
		(rom_info->unit_sw_version == 0x102)) {
			return NODE_TYPE_CONF_CAM;
		} else if (rom_info->unit_sw_version == 0x10000 ||
			rom_info->unit_sw_version == 0x10001) {
			return NODE_TYPE_AVC;
		}
	} else if (rom_info->unit_spec_id == 0x609E &&
		rom_info->unit_sw_version == 0x10483) {
		return NODE_TYPE_SBP2;
	} else {
		resolv_guid(rom_info->guid_hi, rom_info->guid_lo, &cpu);
		if (cpu) return NODE_TYPE_CPU;
	}
	return NODE_TYPE_UNKNOWN;
}

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
void get_guid(raw1394handle_t handle, int phyID,
	unsigned int *hi, unsigned int *lo) {

	if (cooked1394_read(handle, 0xffC0 | phyID, CSR_REGISTER_BASE
		+ CSR_CONFIG_ROM + 0x0C, 4, hi) < 0) { *hi=0; *lo=0; return; }

	if (cooked1394_read(handle, 0xffC0 | phyID, CSR_REGISTER_BASE
		+ CSR_CONFIG_ROM + 0x10, 4, lo) < 0) { *hi=0; *lo=0; return; }
}

/*
 * Read a textual leaf into a malloced ASCII string
 * TODO: This routine should probably care about character sets, Unicode, etc.
 * IN:		phyID:	Physical ID of the node to read from
 *		offset:	Memory offset to read from
 * RETURNS:	pointer to a freshly malloced string that contains the
 *		requested text or NULL if the text could not be read.
 */
char *read_textual_leaf(raw1394handle_t handle, int phyID, octlet_t offset) {
	int i, length;
	char *s;
	quadlet_t quadlet;

	DEBUG_CSR fprintf(stderr, "Reading textual leaf: %i 0x%08x%08x\n",
		phyID, (unsigned int) (offset>>32),
		(unsigned int) offset&0xFFFFFFFF);
	QUADREADERR(handle, phyID, offset, &quadlet);
	quadlet = htonl(quadlet);
	length = (quadlet >> 16) * 4;
	DEBUG_CSR fprintf(stderr, "Textual leaf length: %i (0x%08X)\n",
		length, length);
	if (length<3*4 || length > 256) return NULL;	/* FIXME */
	QUADINC(offset);
	/* language specifier */
	QUADINC(offset);
	/* language id / character set */
	length = length - 2*4;
	if ((s = (char *) malloc(length+1)) == NULL) fatal("Out of memory");
	for (i=0; i<length; i++) {
		DEBUG_CSR fprintf(stderr,".");
		QUADINC(offset);
		QUADREADERR(handle, phyID, offset, &quadlet);
		quadlet = htonl(quadlet);
		s[i] = quadlet>>24;
		if (++i < length) s[i] = (quadlet>>16)&0xFF;
		else break;
		DEBUG_CSR fprintf(stderr,".");
		if (++i < length) s[i] = (quadlet>>8)&0xFF;
		else break;
		DEBUG_CSR fprintf(stderr,".");
		if (++i < length) s[i] = (quadlet)&0xFF;
		else break;
		DEBUG_CSR fprintf(stderr,".");
	}
	s[i] = '\0';
	DEBUG_CSR fprintf(stderr,"\nText: %s\n",s);
	return s;
}

/*
 * Read a whole bunch of textual leafes from a node into an array of ASCII
 * strings.
 * IN:		phyID:		Physical ID of the node to read from
 *		offsets:	Memory offsets to read from
 *		n:		Number of Strings to read
 * RETURNS:	pointer to a freshly malloced array of freshly malloced
 *		strings that contains the requested texts. Some of the
 *		strings might be NULL however.
 *		Returns NULL when the number of textual leafes is 0.
 */
char **read_textual_leafes(raw1394handle_t handle, int phyID,
	octlet_t offsets[], int n) {
	int i;
	char **textual_leafes;

	if (n == 0) return NULL;
	if ((textual_leafes = (char **) calloc(n,sizeof(char *))) == NULL)
		fatal("out of memory");
	for (i=0; i<n; i++) {
		textual_leafes[i] = read_textual_leaf(handle, phyID,
			offsets[i]);
	}
	return textual_leafes;
}

#if 0
/*
 * Determine the delay that is needed to communicate with "slow" devices. This
 * function was a result of a misinterpretation of the libraw1394 return codes
 * and is no lomger neccessary. I have however included it for research
 * purposes. If you get something other than delay 0 with this version (which
 * uses cooked1394_read), something spooky is going on. Please report.
 * IN:		handle:		raw1394 handle
 * 		phyID:		Physical ID of the node to read from
 * RETURNS:	delay in useconds, that should be used when communication with
 * 		this node
 */
unsigned int determine_delay(raw1394handle_t handle, int phyID) {
	unsigned int delay = 0;
	quadlet_t quadlet;
	long long offset;
	offset = CSR_REGISTER_BASE + CSR_CONFIG_ROM;
	do {
		DELAY(delay);
		if(cooked1394_read(handle, 0xffc0 | phyID, offset, 4, &quadlet)
			< 0) WARN("read failed", phyID, offset);
		DELAY(delay);
		if(cooked1394_read(handle, 0xffc0 | phyID, offset + 4, 4,
			&quadlet) < 0) WARN("read failed", phyID, offset);
		quadlet = htonl(quadlet);
		DEBUG_LOWLEVEL fprintf(stderr, "Delay: %i, Magic: 0x%08x\n",
			delay, quadlet);
		delay+=10;
	} while (quadlet != 0x31333934);
	delay-=10;
	DEBUG_CSR fprintf(stderr, "Delay: %i\n", delay);
	return delay;
}
#endif

/*
 * Read various information from the configuration ROM of a device into a
 * Rom_info struct.
 * IN:		phyID:		Physical ID of the node to read from
 *		rom_info:	Pointer to a structure to fill
 * RETURNS:	0 on success, -1 on error
 * NOTE:	Some strings may be malloced by this routine. free_rom_info
 *		should therefore be called, when the contents of this
 *		structure are no longer needed.
 */
int get_rom_info(raw1394handle_t handle, int phyID, Rom_info *rom_info) {
	int length, i, key, value, nr_textual_leafes;
	octlet_t unit_directory = 0;
	octlet_t textual_leafes[256];	/* FIXME */
	char cpu;
	quadlet_t quadlet;
	long long offset;

	init_rom_info(rom_info);
	DEBUG_CSR fprintf(stderr,"---------- PhyID: %i\n",phyID);

	/* Read Bus Info Block */
	offset = CSR_REGISTER_BASE + CSR_CONFIG_ROM;
	DEBUG_CSR fprintf(stderr, "Reading Bus Info Block: %i 0x%08x\n", phyID,
		(int) offset);
 	QUADREADERR(handle, phyID, offset, &quadlet);
 
	quadlet = htonl(quadlet);
	length = quadlet>>24;
	if (length != 4) {
		WARN("wrong bus info block length",phyID, offset);
		return -1;
	}
	QUADINC(offset);
	QUADREADERR(handle, phyID, offset, &quadlet);
	quadlet = htonl(quadlet);
	rom_info->magic = quadlet;
	DEBUG_CSR fprintf(stderr, "Magic Quadlet: 0x%08x\n", quadlet);
	if (rom_info->magic != 0x31333934) {
		WARN("wrong magic quadlet: ",phyID, offset);
		return -1;
	}
	QUADINC(offset);
	QUADREADERR(handle, phyID, offset, &quadlet);
	quadlet = htonl(quadlet);
	rom_info->irmc = quadlet>>31;
	rom_info->cmc = (quadlet>>30)&1;
	rom_info->isc = (quadlet>>29)&1;
	rom_info->bmc = (quadlet>>28)&1;
	rom_info->cyc_clk_acc = (quadlet>>16)&0xFF;
	rom_info->max_rec = (quadlet>>12)&0xF;
	QUADINC(offset);
	QUADREADERR(handle, phyID, offset, &quadlet);
	quadlet = htonl(quadlet);
	rom_info->guid_hi = quadlet;
	QUADINC(offset);
	QUADREADERR(handle, phyID, offset, &quadlet);
	quadlet = htonl(quadlet);
	rom_info->guid_lo = quadlet;

	/* Read Root Directory */
	nr_textual_leafes = 0;
	QUADINC(offset);

	if (cooked1394_read(handle, 0xffc0 | phyID, offset, 4, &quadlet) < 0) {
		WARN("read failed",phyID, offset);
		return -1;
	}
	quadlet = htonl(quadlet);
	length = quadlet>>16;
	DEBUG_CSR fprintf(stderr, "Root Directory length: %i\n",length);
	for (i=0; i<length; i++) {
		QUADINC(offset);
		QUADREADERR(handle, phyID, offset, &quadlet);
		quadlet = htonl(quadlet);
		key = quadlet>>24;
		value = quadlet&0x00FFFFFF;
		DEBUG_LOWLEVEL fprintf(stderr,"key/value: 0x%02x 0x%06x\n",
			key, value);
		switch (key) {
			case 0x0C:
				rom_info->node_capabilities = value; break;
			case 0x03:
				rom_info->vendor_id = value; break;
			case 0x81:
				textual_leafes[nr_textual_leafes++] =
					offset + value*4;
				break;
			case 0xD1:
				unit_directory = offset + value*4;
				break;
			default:
				DEBUG_CSR fprintf(stderr, "Unknown key/value pair 0x%02x 0x%06x\n", key, value);
						
		}
	}

	/* Read Unit Directory */
	if (unit_directory != 0) {
		DEBUG_CSR fprintf(stderr,
			"Reading Unit directory: %i 0x%08x%08x\n", phyID,
			(unsigned int) (unit_directory>>32),
			(unsigned int) unit_directory&0xFFFFFFFF);
		offset = unit_directory;

		if (cooked1394_read(handle, 0xffc0 | phyID, offset, 4, &quadlet)
			< 0) {
			WARN("read failed", phyID, offset);
			return -1;
		}
		quadlet = htonl(quadlet);
		length = quadlet>>16;
		DEBUG_CSR fprintf(stderr, "Unit Directory length: %i\n",
			length);
		for (i=0; i<length; i++) {
			QUADINC(offset);
			QUADREADERR(handle, phyID, offset, &quadlet);
			quadlet = htonl(quadlet);
			key = quadlet>>24;
			value = quadlet&0x00FFFFFF;
			switch (key) {
				case 0x12:
					rom_info->unit_spec_id = value;
					break;
				case 0x13:
					rom_info->unit_sw_version = value;
					break;
				case 0x17:
					rom_info->model_id = value;
					break;
				case 0xD1:
					textual_leafes[nr_textual_leafes++] =
						offset + value*4;
					break;
				default:
					DEBUG_CSR fprintf(stderr, "Unknown key/value pair 0x%02x 0x%06x\n", key, value);
						
			}
		}
	}

	/* Read textual leafes */
	rom_info->nr_textual_leafes = nr_textual_leafes;
	rom_info->textual_leafes = read_textual_leafes(handle, phyID,
		textual_leafes, nr_textual_leafes);

	/* Calculate label */
	rom_info->label = resolv_guid(rom_info->guid_hi, rom_info->guid_lo,
		&cpu);
	if (rom_info->label == NULL) {
		if (rom_info->nr_textual_leafes != 0) {
			rom_info->label = rom_info->textual_leafes[0];
		} else {
			rom_info->label = "Unknown";
		}
	}

	/* Get vendor */
	rom_info->vendor = resolv_oui(rom_info->vendor_id);
	if (rom_info->vendor == NULL) {
		rom_info->vendor = "Unknown";
	}

	/* Get node type */
	rom_info->node_type = get_node_type(rom_info);

	return 0;
}

/*
 * Free up all memory malloced by get_rom_info.
 * IN:  rom_info:	pointer to the Rom_info structure which is no longer
 * 			needed
 */
void free_rom_info(Rom_info *rom_info) {
	int i;

	return;	//FIXME
	if (rom_info == NULL || rom_info->textual_leafes == NULL) return;

	for (i=0; i<rom_info->nr_textual_leafes; i++) {
		free(rom_info->textual_leafes[i]);
	}
	if (rom_info->textual_leafes != 0) free(rom_info->textual_leafes);
	rom_info->textual_leafes = 0;
}

