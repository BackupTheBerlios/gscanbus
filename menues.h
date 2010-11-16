/* 
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 *
 * menues.h - Menues and Transaction Dialogs for gscanbus
 * written 10.2.2000 by Andreas Micklei
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

#include <libraw1394/raw1394.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "fatal.h"
#include "raw1394util.h"

/*
 * Closes a dialog window.
 * IN:		widget:	not used
 * 		data:	the dialog
 */
void CloseDialog(GtkWidget *widget, gpointer data);

/*
 * Called when a dialog is closing. Releases the input focus.
 * IN:		widget:	the dialog
 */
void ClosingDialog(GtkWidget *widget, gpointer data);

/*
 * build the menu bar
 * IN:		window: pointer to the window. This is needed for adding
 *			keyboard accelerators
 * RETURNS:	pointer to the feshly created and visible menu bar
 */
GtkWidget *makeMenuBar(GtkWidget *window);

