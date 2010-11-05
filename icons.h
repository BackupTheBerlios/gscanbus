/* $Id: icons.h,v 1.2 2001/05/11 17:54:21 ami Exp $
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
#include <gtk/gtk.h>

#include "gnome-question.xpm"
#include "gnome-qeye.xpm"
#include "gnome-term.xpm"
#include "gnome-term-linux.xpm"
#include "gnome-term-apple.xpm"
#include "gnome-term-windows.xpm"
#include "gtcd.xpm"
#include "apple-green.xpm"

extern GdkPixmap *xpm_unknown;
extern GdkPixmap *xpm_dvcr;
extern GdkPixmap *xpm_disk;
extern GdkPixmap *xpm_cpu;
extern GdkPixmap *xpm_cpu_linux;
extern GdkPixmap *xpm_cpu_apple;
extern GdkPixmap *xpm_cpu_windows;
extern GdkBitmap *xpm_unknown_mask;
extern GdkBitmap *xpm_dvcr_mask;
extern GdkPixmap *xpm_disk_mask;
extern GdkBitmap *xpm_cpu_mask;
extern GdkBitmap *xpm_cpu_linux_mask;
extern GdkBitmap *xpm_cpu_apple_mask;
extern GdkBitmap *xpm_cpu_windows_mask;
extern GdkWindow *xpm_window;

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
void init_xpm_d(GdkPixmap **xpm, GdkBitmap **xpm_mask, GdkWindow *xpm_window,
	GdkWindow *window, char **xpm_data);

void initIcons(GdkWindow *window);

void chooseIcon(Rom_info *rom_info, GdkBitmap **xpm_node,
	GdkBitmap **xpm_node_mask, char **label);

