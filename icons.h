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

#include "rominfo.h"
#include <gtk/gtk.h>

extern GdkPixbuf *xpm_cpu_linux;

void initIcons(GdkWindow *window);

void chooseIcon(Rom_info *rom_info, GdkPixbuf **xpm_node, char **label);

