/* 
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
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

#include "icons.h"

#include "gnome-question.xpm"
#include "gnome-qeye.xpm"
#include "gnome-term.xpm"
#include "gnome-term-linux.xpm"
#include "gnome-term-apple.xpm"
#include "gnome-term-windows.xpm"
#include "gtcd.xpm"
#include "apple-green.xpm"

GdkPixbuf *xpm_unknown = NULL;
GdkPixbuf *xpm_dvcr = NULL;
GdkPixbuf *xpm_disk = NULL;
GdkPixbuf *xpm_cpu = NULL;
GdkPixbuf *xpm_cpu_linux = NULL;
GdkPixbuf *xpm_cpu_apple = NULL;
GdkPixbuf *xpm_cpu_windows = NULL;

static int chooseIconVendor(Rom_info *, GdkPixbuf **, char **);
static int contains(char *, char *);

void initIcons(void) 
{
	xpm_unknown = gdk_pixbuf_new_from_xpm_data(gnome_question_xpm);
	xpm_dvcr = gdk_pixbuf_new_from_xpm_data(gnome_qeye_xpm);
	xpm_disk = gdk_pixbuf_new_from_xpm_data(gtcd_xpm);
	xpm_cpu = gdk_pixbuf_new_from_xpm_data(gnome_term_xpm);
	xpm_cpu_linux = gdk_pixbuf_new_from_xpm_data(gnome_term_linux_xpm);
	xpm_cpu_apple = gdk_pixbuf_new_from_xpm_data(gnome_term_apple_xpm);
	xpm_cpu_windows = gdk_pixbuf_new_from_xpm_data(gnome_term_windows_xpm);
}

void chooseIcon(Rom_info *rom_info, GdkPixbuf **xpm_node, char **label) 
{
	switch(rom_info->node_type) {
		case NODE_TYPE_CONF_CAM:
		case NODE_TYPE_AVC:
			*xpm_node = xpm_dvcr;
			*label = "AV/C Device";
			break;
		case NODE_TYPE_SBP2:
			*xpm_node = xpm_disk;
			*label = "SBP2 Device";
			break;
		case NODE_TYPE_CPU:
			if (chooseIconVendor(rom_info, xpm_node, label) < 0) {
				*xpm_node = xpm_cpu;
			}
			break;
		default:
			if (chooseIconVendor(rom_info, xpm_node, label) < 0) {
				*xpm_node = xpm_unknown;
			}
			break;
	}

}

int chooseIconVendor(Rom_info *rom_info, GdkPixbuf **xpm_node, char **label) 
{
	if (contains(rom_info->vendor, "apple")) {
		*xpm_node = xpm_cpu_apple;
		*label = "MacOS";
	} else if (contains(rom_info->vendor, "microsoft")) {
		*xpm_node = xpm_cpu_windows;
		*label = "Windows";
	} else return -1;
	return 0;

}

int contains(char *s1, char *s2) {

	int i;
	int ls1, ls2;
	char *ps1 = s1;

	if (s1 == NULL || s2 == NULL) return 0;
	ls1 = strlen(s1);
	ls2 = strlen(s2);

	for(i=0; i<ls1-ls2; i++) {
		if (strncasecmp(ps1++,s2,ls2) == 0) return -1;
	}

	return 0;

}

