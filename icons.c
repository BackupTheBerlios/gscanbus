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

GdkPixmap *xpm_unknown = NULL;
GdkPixmap *xpm_dvcr = NULL;
GdkPixmap *xpm_disk = NULL;
GdkPixmap *xpm_cpu = NULL;
GdkPixmap *xpm_cpu_linux = NULL;
GdkPixmap *xpm_cpu_apple = NULL;
GdkPixmap *xpm_cpu_windows = NULL;
GdkBitmap *xpm_unknown_mask;
GdkBitmap *xpm_dvcr_mask;
GdkPixmap *xpm_disk_mask;
GdkBitmap *xpm_cpu_mask;
GdkBitmap *xpm_cpu_linux_mask;
GdkBitmap *xpm_cpu_apple_mask;
GdkBitmap *xpm_cpu_windows_mask;
GdkWindow *xpm_window = NULL;

static int chooseIconVendor(Rom_info *, GdkBitmap **, GdkBitmap **, char **);
static int contains(char *, char *);

/*
 * Initialize a GdkPixmap and corresponding mask from xpm data. If the
 * pixmap was already loaded and the window has not changed, nothing is done.
 * If the pixmap was already loaded and the window has changed, the pixmap
 * will be reloaded.
 * IN:	xpm:		previously initialized pixmap if any
 *	xpm_mask:	previously initialized mask if any
 *	xpm_window:	the window previously used for pixmaps
 *	window:		the window to use now for pixmaps
 *	data:		a pointer to the xpm data
 * OUT:	xpm:		the loaded pixmap
 *	xpm_mask:	the loaded mask
 */
static void init_xpm_d(GdkPixmap **xpm, GdkBitmap **xpm_mask, GdkWindow *xpm_window,
	GdkWindow *window, char **xpm_data) 
{
	if (xpm_window != window) {
		if (*xpm != NULL) gdk_pixmap_unref(*xpm);
		if (*xpm_mask != NULL) gdk_bitmap_unref(*xpm_mask);
	}
	if (*xpm == NULL || xpm_window != window) {
		*xpm = gdk_pixmap_create_from_xpm_d(
			window, xpm_mask, NULL, xpm_data);
	}
}

void initIcons(GdkWindow *window) {

	init_xpm_d(&xpm_unknown, &xpm_unknown_mask, xpm_window, window, 
		gnome_question_xpm);
	init_xpm_d(&xpm_dvcr, &xpm_dvcr_mask, xpm_window, window,
		gnome_qeye_xpm);
	init_xpm_d(&xpm_disk, &xpm_disk_mask, xpm_window, window,
		gtcd_xpm);
	init_xpm_d(&xpm_cpu, &xpm_cpu_mask, xpm_window, window,
		gnome_term_xpm);
	init_xpm_d(&xpm_cpu_linux, &xpm_cpu_linux_mask, xpm_window, window,
		gnome_term_linux_xpm);
	init_xpm_d(&xpm_cpu_apple, &xpm_cpu_apple_mask, xpm_window, window,
		gnome_term_apple_xpm);
	init_xpm_d(&xpm_cpu_windows, &xpm_cpu_windows_mask, xpm_window, window,
		gnome_term_windows_xpm);

	xpm_window = window;

}

void chooseIcon(Rom_info *rom_info, GdkBitmap **xpm_node,
	GdkBitmap **xpm_node_mask, char **label) {

	switch(rom_info->node_type) {
		case NODE_TYPE_CONF_CAM:
		case NODE_TYPE_AVC:
			*xpm_node = xpm_dvcr;
			*xpm_node_mask = xpm_dvcr_mask;
			*label = "AV/C Device";
			break;
		case NODE_TYPE_SBP2:
			*xpm_node = xpm_disk;
			*xpm_node_mask = xpm_disk_mask;
			*label = "SBP2 Device";
			break;
		case NODE_TYPE_CPU:
			if (chooseIconVendor(rom_info,
				xpm_node, xpm_node_mask, label) < 0) {
				*xpm_node = xpm_cpu;
				*xpm_node_mask = xpm_cpu_mask;
			}
			break;
		default:
			if (chooseIconVendor(rom_info,
				xpm_node, xpm_node_mask, label) < 0) {
				*xpm_node = xpm_unknown;
				*xpm_node_mask = xpm_unknown_mask;
			}
			break;
	}

}

int chooseIconVendor(Rom_info *rom_info, GdkBitmap **xpm_node,
	GdkBitmap **xpm_node_mask, char **label) {

	if (contains(rom_info->vendor, "apple")) {
		*xpm_node = xpm_cpu_apple;
		*xpm_node_mask = xpm_cpu_apple_mask;
		*label = "MacOS";
	} else if (contains(rom_info->vendor, "microsoft")) {
		*xpm_node = xpm_cpu_windows;
		*xpm_node_mask = xpm_cpu_windows_mask;
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

