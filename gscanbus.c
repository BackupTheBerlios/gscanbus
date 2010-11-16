/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 * Copyright (C) 2010  Gareth McMullin  <gareth@blacksphere.co.nz>
 *
 * gscanbus.c - Linux IEEE-1394 Subsystem GDK Topology Viewing utility
 * 		written 18.11.1999 to 05.06.2001 by Andreas Micklei 
 * 		updated for GTK+ 2 by Gareth McMullin
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

#include "raw1394util.h"
#include "simpleavc.h"
#include "topologyMap.h"
#include "rominfo.h"
#include "topologyTree.h"
#include "decodeselfid.h"
#include "menues.h"
#include "debug.h"
#include "icons.h"
#include <sys/types.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define FONTNAME "-*-helvetica-*-*-*-*-12-*-*-*-*-*-*-*"
#define FONTHEIGHT 12

#define NODEWIDTH 48	/* Pixmaps should not exceed this size */
#define NODEHEIGHT 48

#define SETLINEWIDTH(gc, width) gdk_gc_set_line_attributes(gc, width, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

raw1394handle_t handle;		/* Global = dangerous (threading issues) */
TopologyTree *topologyTree;	/* Global for mouse click detection */
GtkWidget *drawing_area;	/* Global for use by bus reset handler */
int repaintCountdown = 0;

static GdkPixmap *pixmap = NULL;

const char not_compatible[] = "\
This libraw1394 does not work with your version of Linux. You need a different\
version that matches your kernel (see kernel help text for the raw1394 option to\
find out which is the correct version).\n";

const char not_loaded[] = "\
This probably means that you don't have raw1394 support in the kernel or that\
you haven't loaded the raw1394 module.\n";

/*---------------------------------------------------------------------------
 * Drawing routines
 *---------------------------------------------------------------------------*/

/*
 * Draw a line for the topology Tree.
 * IN:	drawable:	The GDK drawing Area to draw into
 * 	gc:		The GDK Graphics Context to use
 * 	x1:		x position of beginning of line
 * 	y1:		y position of beginning of line
 * 	x2:		x position of end of line
 * 	y2:		y position of end of line
 * 	node:		The current node
 * 	child:		The child node to draw the line to
 * 	col_new:	The color to use for the line
 * 	col_old:	The color to restore when finished
 */
void drawTopologyLine(GdkDrawable *drawable, GdkGC *gc,
	int x1, int y1, int x2, int y2, TopologyTree *node,
	TopologyTree *child, GdkColor *col_new, GdkColor *col_old) {
	SETLINEWIDTH(gc,(MIN(node->selfid[0].packetZero.phySpeed,
		child->selfid[0].packetZero.phySpeed)+1)*2);
	gdk_gc_set_foreground(gc, col_new);
	gdk_draw_line(drawable, gc, x1, y1, x2, y2);
	gdk_gc_set_foreground(gc, col_old);
}

void chooseLabel(Rom_info *rom_info, TopologyTree *node, char *label) {

	/* Use rom_info->label if it contains something meaningful */
	if (rom_info->label != NULL && strcmp(rom_info->label, "Unknown")) {
		strcpy(node->label, rom_info->label);
	/* Use calculated label otherwise, if it exists */
	} else if (label != NULL) {
		strcpy(node->label, label);
	} else {
		strcpy(node->label, "Unknown");
	}

}

/*
 * Draw a topology Tree.
 * IN:	drawable:	The GDK drawing Area to draw into
 * 	window:		The window to get various default values from
 * 	gc:		The GDK Graphics Context to use
 * 	node:		The topologyTree or subTree
 * 	myPhyID:	Physical ID of the host, for highlighting
 * 	left:		left offset to begin drawing
 * 	width:		width of drawing area
 * 	level:		depth of the current subTree in respect to the root
 */
void drawTopologyTree(GdkDrawable *drawable, GdkWindow *window, GdkGC *gc,
    	TopologyTree *node, int myPhyID, int left, int width, int level) {
    	int nodewidth = NODEWIDTH;
    	int nodeheight = NODEHEIGHT;
	int xpmwidth;
	int xpmheight;
    	GdkPixmap *xpm_node;
    	GdkBitmap *xpm_node_mask;
    	TopologyTree *child;
	Rom_info rom_info;
	GdkGCValues gc_values;
	char *label = NULL;

	/* All static variables are only loaded once and then reused */
    	static GdkFont *font;
    	static GdkColormap *colormap;
    	static GdkColor *col_arc;
    	static GdkColor *col_lines;

	/* Remember default from gc */
	gdk_gc_get_values(gc, &gc_values);

	/* Load resources (font, colours, pixmaps) */
    	if (font == NULL) {
		font = gdk_font_load(FONTNAME);
		gdk_font_ref (font);	/* is this necessary? */
	}
	if (colormap == NULL) {
		colormap = gdk_colormap_get_system();
		gdk_colormap_ref(colormap);	/* is this necessary? */
	}
	if (col_arc == NULL) {
		col_arc = malloc(sizeof(GdkColor));
		gdk_color_parse("cornflower blue", col_arc);
		gdk_colormap_alloc_color(colormap, col_arc, FALSE, TRUE);
	}
	if (col_lines == NULL) {
		/*col_lines = malloc(sizeof(GdkColor));
		gdk_color_parse("gray", col_lines);
		gdk_colormap_alloc_color(colormap, col_lines, FALSE, TRUE);*/
		col_lines = col_arc;
	}

	initIcons(window);

	rom_info = node->rom_info;

	/* Choose label and icon */
	chooseIcon(&rom_info, &xpm_node, &xpm_node_mask, &label);
	chooseLabel(&rom_info, node, label);

	/* Recursively draw rest of tree */
	if (numberOfChilds(node) == 0) {
		/* We are a leaf */
	} else if (numberOfChilds(node) == 1) {
		/* Draw one child directly beneath us */
		child = getNthChild(node,1);
		drawTopologyLine(drawable, gc,
			left+width/2, level*nodeheight*2+nodeheight/2,
			left+width/2, (level+1)*nodeheight*2+nodeheight/2,
			node, child, col_lines, &gc_values.foreground);
		drawTopologyTree(drawable, window, gc, child, myPhyID,
			left, width, level+1);
	} else if (numberOfChilds(node) == 2) {
		/* Draw two childs left and right beneath us */
		child = getNthChild(node,1);
		drawTopologyLine(drawable, gc,
			left+width/2, level*nodeheight*2+nodeheight/2,
			left+width/4, (level+1)*nodeheight*2+nodeheight/2,
			node, child, col_lines, &gc_values.foreground);
		drawTopologyTree(drawable, window, gc, child, myPhyID,
			left, width/2, level+1);
		child = getNthChild(node,2);
		drawTopologyLine(drawable, gc,
			left+width/2, level*nodeheight*2+nodeheight/2,
			left+width/2+width/4,
			(level+1)*nodeheight*2+nodeheight/2,
			node, child, col_lines, &gc_values.foreground);
		drawTopologyTree(drawable, window, gc, child, myPhyID,
			left+width/2, width/2, level+1);
	} else if (numberOfChilds(node) == 3) {
		/* Draw three childs left, right and directly beneath us */
		child = getNthChild(node,1);
		drawTopologyLine(drawable, gc,
			left+width/2, level*nodeheight*2+nodeheight/2,
			left+width/6, (level+1)*nodeheight*2+nodeheight/2,
			node, child, col_lines, &gc_values.foreground);
		drawTopologyTree(drawable, window, gc, child, myPhyID,
			left, width/3, level+1);
		child = getNthChild(node,2);
		drawTopologyLine(drawable, gc,
			left+width/2, level*nodeheight*2+nodeheight/2,
			left+width/2, (level+1)*nodeheight*2+nodeheight/2,
			node, child, col_lines, &gc_values.foreground);
		drawTopologyTree(drawable, window, gc, child, myPhyID,
			left, width, level+1);
		child = getNthChild(node,3);
		drawTopologyLine(drawable, gc,
			left+width/2, level*nodeheight*2+nodeheight/2,
			left+2*(width/3)+width/6,
			(level+1)*nodeheight*2+nodeheight/2,
			node, child, col_lines, &gc_values.foreground);
		drawTopologyTree(drawable, window, gc, child, myPhyID,
			left+2*(width/3), width/3, level+1);
	}

	SETLINEWIDTH(gc, 1);

	/* Highlight Host controller and give it a Linux pixmap */
	if (node->selfid[0].packetZero.phyID == myPhyID) {
		if (strcmp(node->label, "Unknown") == 0)
			strcpy(node->label, "Localhost");
		xpm_node = xpm_cpu_linux;	/* Host controller */
		xpm_node_mask = xpm_cpu_linux_mask;
		gdk_gc_set_foreground(gc, col_arc);
		gdk_draw_arc(drawable, gc, 1,
			left + (width/2 - nodewidth/2),
			level*nodeheight*2, nodewidth, nodeheight,
			0, 360*64);
	}

	/* Draw icon */
	gdk_gc_set_foreground(gc, &gc_values.foreground);

	gdk_drawable_get_size(GDK_DRAWABLE(xpm_node), &xpmwidth, &xpmheight);

	gdk_gc_set_clip_origin(gc, left + (width/2 - xpmwidth/2),
		level*nodeheight*2 + (nodeheight-xpmheight)/2);
	gdk_gc_set_clip_mask(gc, xpm_node_mask);
	gdk_draw_drawable(drawable, gc, xpm_node, 0, 0,
		left + (width/2 - xpmwidth/2),
		level*nodeheight*2 + (nodeheight-xpmheight)/2,
		xpmwidth, xpmheight);
	gdk_gc_set_clip_mask(gc, gc_values.clip_mask);

	/* Draw speed string */
	gdk_draw_string(drawable, font, gc,
		left + (width/2 - nodewidth/2),
		level*nodeheight*2+nodeheight+FONTHEIGHT,
		decode_speed(node->selfid[0].packetZero.phySpeed));

	/* Draw label */
	gdk_draw_string(drawable, font, gc,
		left + (width/2 - nodewidth/2),
		level*nodeheight*2+nodeheight+FONTHEIGHT*2,
		node->label);
}

/*
 * Repaint the main window.
 * IN:		data:	A pointer to the drawable of the main window
 * RESULT:	TRUE on success, FALSE otherwise
 */
gint Repaint (gpointer data) {
	RAW1394topologyMap* topologyMap;
	int nodeCount, depth;
	GtkWidget* drawing_area = (GtkWidget *) data;
	GdkRectangle update_rect;
	int width, height;
	GdkDrawable *drawable;
	GdkGC *gc;

	nodeCount = raw1394_get_nodecount(handle);
	topologyMap = raw1394GetTopologyMap(handle);
	/*topologyMap = generateTestTopologyMap(7);*/
	if (topologyMap == NULL) {
		fprintf(stderr, "Could not read topologyMap\n");
		return (TRUE);
	}
	if (topologyTree != NULL) freeTopologyTree(topologyTree);
	topologyTree = spawnTopologyTree(handle, topologyMap);

	DEBUG_GENERAL fprintf(stderr, "Root id: %d\n",
		topologyTreeRoot(topologyTree)->selfid[0].packetZero.phyID);

	depth = topologyTreeDepth(topologyTree);
	DEBUG_GENERAL fprintf(stderr, "\nTree depth: %d\n", depth);

	drawable = drawing_area->window;
	gc = gdk_gc_new(drawing_area->window);

	width = drawing_area->allocation.width;
	height = drawing_area->allocation.height;

	gdk_draw_rectangle(pixmap,
		drawing_area->style->white_gc,
		TRUE,
		0, 0, width, height);

	if (depth != 0)
		drawTopologyTree(pixmap, drawable, gc, topologyTree,
			raw1394_get_local_id(handle) & 0x3f, 0, width, 0);

	gdk_gc_unref(gc);

	update_rect.x = 0;
	update_rect.y = 0;
	update_rect.width = width;
	update_rect.height = height;
	gtk_widget_draw(drawing_area, &update_rect);

	return (TRUE);
}

/*
 * Detect which node was clicked.
 * IN:		node:	The topologyTree or SubTree
 * 		left:	left offset of drawing area
 * 		width:	width of drawing area
 * 		level:	depth of the current subTree in respect to the root
 * 		x:	x position of mouse click
 * 		y:	y position of mouse click
 * RESULT:	clicked node or NULL if no node was clicked
 */
TopologyTree *detectClick(TopologyTree *node, int left, int width, int level,
	int x, int y) {
    	TopologyTree *child;
	TopologyTree *click;

	if (y > level*NODEHEIGHT*2 && y < (level*NODEHEIGHT*2)+NODEHEIGHT) {
		if (x > (left+width/2)-NODEWIDTH/2
			&& x < (left+width/2)+NODEWIDTH/2) {
				/* Click detected, return phyID */
				return node;
		} else {
			/* No need to go beyond this level, truncate search */
			return NULL;
		}
	}
	if (numberOfChilds(node) == 0) {
		/* We are a leaf */
		return NULL;
	} else if (numberOfChilds(node) == 1) {
		/* Process one child directly beneath us */
		child = getNthChild(node,1);
		return detectClick(child, left, width, level+1, x, y);
	} else if (numberOfChilds(node) == 2) {
		/* Process two childs left and right beneath us */
		child = getNthChild(node,1);
		click = detectClick(child, left, width/2, level+1, x, y);
		if (click != NULL) return click;
		child = getNthChild(node,2);
		return detectClick(child, left+width/2, width/2, level+1, x, y);
	} else if (numberOfChilds(node) == 3) {
		/* Process three childs left, right and directly beneath us */
		child = getNthChild(node,1);
		click = detectClick(child, left, width/3, level+1, x, y);
		if (click != NULL) return click;
		child = getNthChild(node,2);
		click = detectClick(child, left, width, level+1, x, y);
		if (click != NULL) return click;
		child = getNthChild(node,3);
		return detectClick(child, left+2*(width/3), width/3, level+1,
			x, y);
	}
	return NULL;
}

/*
 * Called whenever the window is made visible. Copys the pixmap to the window.
 * IN:		widget:	the drawing area
 * 		event:	the expose event
 * RESULT:	always FALSE
 */
static gint expose_event(GtkWidget *widget, GdkEventExpose *event) {
	gdk_draw_drawable(widget->window,
		widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		GDK_DRAWABLE(pixmap),
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);

	return FALSE;
}

/*
 * Called when the window is created or resized.
 * IN:		widget:	the drawing area
 * 		event:	the configure event
 * RESULT:	always TRUE
 */
static gint configure_event(GtkWidget *widget, GdkEventConfigure *event) {
	if (pixmap) {
		g_object_unref(pixmap);
	}

	pixmap = gdk_pixmap_new(widget->window,
		widget->allocation.width,
		widget->allocation.height,
		-1);

	Repaint((gpointer) drawing_area);

	return TRUE;
}

/* The following are now defined in menues.c */
#if 0
/*
 * Closes a dialog window.
 * IN:		widget:	not used
 * 		data:	the dialog
 */
void CloseDialog(GtkWidget *widget, gpointer data) {
	gtk_widget_destroy(GTK_WIDGET(data));
}

/*
 * Called when a dialog is closing. Releases the input focus.
 * IN:		widget:	the dialog
 */
void ClosingDialog(GtkWidget *widget, gpointer data) {
	gtk_grab_remove(GTK_WIDGET(widget));
}
#endif

#define AVC_RETRY 4
#define CTLVCR0 AVC_CTYPE_CONTROL | AVC_SUBUNIT_TYPE_TAPE_RECORDER | AVC_SUBUNIT_ID_0
#define STATVCR0 AVC_CTYPE_STATUS | AVC_SUBUNIT_TYPE_TAPE_RECORDER | AVC_SUBUNIT_ID_0
#define CTLTUNER0 AVC_CTYPE_CONTROL | AVC_SUBUNIT_TYPE_TUNER | AVC_SUBUNIT_ID_0
#define STATTUNER0 AVC_CTYPE_STATUS | AVC_SUBUNIT_TYPE_TUNER | AVC_SUBUNIT_ID_0
#define TUNER0 AVC_SUBUNIT_TYPE_TUNER | AVC_SUBUNIT_ID_0
#define CTLUNIT AVC_CTYPE_CONTROL | AVC_SUBUNIT_TYPE_UNIT | AVC_SUBUNIT_ID_IGNORE
#define STATUNIT AVC_CTYPE_STATUS | AVC_SUBUNIT_TYPE_UNIT | AVC_SUBUNIT_ID_IGNORE
/*
 * Find out if an AVC device is currently playing
 * IN:		handle: the libraw1394 handle
 * 		phyID:	the phyisical ID of the node
 * RETURNS:	OPERAND if device is playing
 * 		0 otherwise
 */
int isPlaying(raw1394handle_t handle, int phyID) {
	quadlet_t response = avc_transaction(handle, phyID, STATVCR0
		| VCR_COMMAND_TRANSPORT_STATE | VCR_OPERAND_TRANSPORT_STATE,
		AVC_RETRY);
	if (AVC_MASK_OPCODE(response)
		== VCR_RESPONSE_TRANSPORT_STATE_PLAY)
		return AVC_GET_OPERAND0(response);
	else
		return 0;
}

/*
 * Find out if an AVC device is currently recording
 * IN:		handle: the libraw1394 handle
 * 		phyID:	the phyisical ID of the node
 * RETURNS:	OPERAND if device is recording
 * 		0 otherwise
 */
int isRecording(raw1394handle_t handle, int phyID) {
	quadlet_t response = avc_transaction(handle, phyID, STATVCR0
		| VCR_COMMAND_TRANSPORT_STATE | VCR_OPERAND_TRANSPORT_STATE,
                AVC_RETRY);
	if (AVC_MASK_OPCODE(response)
		== VCR_RESPONSE_TRANSPORT_STATE_RECORD)
		return AVC_GET_OPERAND0(response);
	else
		return 0;
}

/*
 * Called when the play button is clicked. Send a play command to a node
 * normally, or a play slow motion command if the node is already playing at
 * normal speed.
 * IN:		widget:	the button
 * 		data:	The node
 */
void avc_play(GtkWidget *widget, gpointer data) {
	int phyID;

	phyID = ((TopologyTree *)data)->selfid[0].packetZero.phyID;
	if (isPlaying(handle, phyID) == VCR_OPERAND_PLAY_FORWARD) {
		send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_PLAY | VCR_OPERAND_PLAY_SLOWEST_FORWARD);
	} else {
		send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_PLAY | VCR_OPERAND_PLAY_FORWARD);
	}
}

/*
 * Called when the stop button is clicked. Send a stop command to a node.
 * IN:		widget:	the button
 * 		data:	The node
 */
void avc_stop(GtkWidget *widget, gpointer data) {
	int phyID;

	phyID = ((TopologyTree *)data)->selfid[0].packetZero.phyID;
	send_avc_command(handle, phyID, CTLVCR0
		| VCR_COMMAND_WIND | VCR_OPERAND_WIND_STOP);

}

/*
 * Called when the rewind button is clicked. Send a wind_rewind when the
 * node is stopped or a play_rewind command when the node is playing or
 * paused to a node.
 * IN:		widget:	the button
 * 		data:	The node
 */
void avc_rewind(GtkWidget *widget, gpointer data) {
	int phyID;

	phyID = ((TopologyTree *)data)->selfid[0].packetZero.phyID;
	if (isPlaying(handle, phyID)) {
		send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_PLAY | VCR_OPERAND_PLAY_FASTEST_REVERSE);

	} else {
		send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_WIND | VCR_OPERAND_WIND_REWIND);

	}
}

/*
 * Called when the pause button is clicked. Send a pause command to a node.
 * IN:		widget:	the button
 * 		data:	The node
 */
void avc_pause(GtkWidget *widget, gpointer data) {
	int phyID, mode;

	phyID = ((TopologyTree *)data)->selfid[0].packetZero.phyID;
	if ((mode = isRecording(handle, phyID))) {
		if (mode == VCR_OPERAND_RECORD_PAUSE) {
			send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_RECORD | VCR_OPERAND_RECORD_RECORD);
		} else {
			send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_RECORD | VCR_OPERAND_RECORD_PAUSE);
		}
	} else {
		if (isPlaying(handle, phyID)
			==VCR_OPERAND_PLAY_FORWARD_PAUSE) {
			send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_PLAY | VCR_OPERAND_PLAY_FORWARD);
		} else {
			send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_PLAY | VCR_OPERAND_PLAY_FORWARD_PAUSE);
		}
	}

}

/*
 * Called when the forward button is clicked. Send a wind_forward when the
 * node is stopped or a play_forward command when the node is playing or
 * paused to a node.
 * IN:		widget:	the button
 * 		data:	The node
 */
void avc_forward(GtkWidget *widget, gpointer data) {
	int phyID;

	phyID = ((TopologyTree *)data)->selfid[0].packetZero.phyID;
	if (isPlaying(handle, phyID)) {
		send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_PLAY | VCR_OPERAND_PLAY_FASTEST_FORWARD);
	} else {
		send_avc_command(handle, phyID, CTLVCR0
			| VCR_COMMAND_WIND | VCR_OPERAND_WIND_FAST_FORWARD);

	}
}

/*
 * Called when the eject button is clicked. Send an eject command to a node.
 * IN:		widget:	the button
 * 		data:	The node
 */
void avc_eject(GtkWidget *widget, gpointer data) {
	int phyID;

	phyID = ((TopologyTree *)data)->selfid[0].packetZero.phyID;
	send_avc_command(handle, phyID, CTLVCR0
		| VCR_COMMAND_LOAD_MEDIUM | VCR_OPERAND_LOAD_MEDIUM_EJECT);
}

/*
 * Called when the record button is clicked. Send a record command to a node.
 * IN:		widget:	the button
 * 		data:	The node
 */
void avc_record(GtkWidget *widget, gpointer data) {
	int phyID;

	phyID = ((TopologyTree *)data)->selfid[0].packetZero.phyID;
	send_avc_command(handle, phyID, CTLVCR0
		| VCR_COMMAND_RECORD | VCR_OPERAND_RECORD_RECORD);
}

#define MAX_DESCRIPTORS 0x100
void avc_test_descriptors(int phyID, quadlet_t subunit_type,
	quadlet_t subunit_id) {
	int i;
	quadlet_t response;
	char descriptors[MAX_DESCRIPTORS];

	fprintf(stderr, "*** Test descriptors ***\n");

	memset(descriptors,0,MAX_DESCRIPTORS);
	for (i=0; i<MAX_DESCRIPTORS; i++) {
		response = avc_transaction(handle, phyID,
			AVC_CTYPE_SPECIFIC_INQUIRY
			| subunit_type | subunit_id
			| AVC_COMMAND_OPEN_DESCRIPTOR | i, AVC_RETRY);
		if (AVC_MASK_RESPONSE(response) == AVC_RESPONSE_IMPLEMENTED) {
			descriptors[i] = -1;
		}
	}
	fprintf(stderr, "Unit Descriptors:");
	for (i=0; i<MAX_DESCRIPTORS; i++)
		if (descriptors[i])fprintf(stderr, " 0x02%X", i);
	fprintf(stderr, "\n");
}

struct status_entry {
	TopologyTree *node;
	GtkWidget *entry;
};

gint update_avc_status(gpointer data) {
	struct status_entry *status_entry = (struct status_entry *) data;
	char *status;
	int phyID;

	if (data == NULL) return FALSE;

	DEBUG_AVC fprintf(stderr, "Getting AV/C status\n");
	if (!GTK_IS_WIDGET(status_entry->entry)) return FALSE;
	phyID = status_entry->node->selfid[0].packetZero.phyID;

	status = avc_decode_vcr_response(avc_transaction(handle, phyID,
			STATVCR0 | VCR_COMMAND_TRANSPORT_STATE
			| VCR_OPERAND_TRANSPORT_STATE, AVC_RETRY));

	gtk_entry_set_text(GTK_ENTRY(status_entry->entry), status);
	return TRUE;
}

GtkWidget *make_avc_buttons(TopologyTree *node) {
	//GtkWidget *table, *button;
	GtkWidget *hbox1, *hbox2, *hbox3, *vbox, *button, *label, *entry;
	struct status_entry *status_entry;

	status_entry = malloc(sizeof(struct status_entry));
	if (status_entry == NULL) fatal("out of memory");

	//table = gtk_table_new(8, 2, FALSE);
	hbox1 = gtk_hbox_new(TRUE, 0);
	hbox2 = gtk_hbox_new(TRUE, 0);
	hbox3 = gtk_hbox_new(FALSE, 0);
	button = gtk_button_new_with_label("<<");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(avc_rewind), node);
	gtk_widget_show(button);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 3, 0, 1);
	gtk_box_pack_start(GTK_BOX(hbox1), button, TRUE, TRUE, 0);
	button = gtk_button_new_with_label("PLAY");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(avc_play), node);
	gtk_widget_show(button);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 3, 5, 0, 1);
	gtk_box_pack_start(GTK_BOX(hbox1), button, TRUE, TRUE, 0);
	button = gtk_button_new_with_label(">>");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(avc_forward), node);
	gtk_widget_show(button);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 5, 7, 0, 1);
	gtk_box_pack_start(GTK_BOX(hbox1), button, TRUE, TRUE, 0);
	button = gtk_button_new_with_label("STOP");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(avc_stop), node);
	gtk_widget_show(button);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 1, 2);
	gtk_box_pack_start(GTK_BOX(hbox2), button, TRUE, TRUE, 0);
	button = gtk_button_new_with_label("||");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(avc_pause), node);
	gtk_widget_show(button);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 2, 4, 1, 2);
	gtk_box_pack_start(GTK_BOX(hbox2), button, TRUE, TRUE, 0);
	button = gtk_button_new_with_label("Eject");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(avc_eject), node);
	gtk_widget_show(button);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 4, 6, 1, 2);
	gtk_box_pack_start(GTK_BOX(hbox2), button, TRUE, TRUE, 0);
	button = gtk_button_new_with_label("Record");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(avc_record), node);
	gtk_widget_show(button);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 6, 8, 1, 2);
	gtk_box_pack_start(GTK_BOX(hbox2), button, TRUE, TRUE, 0);
	//gtk_widget_show(table);
	//return table;

	label = gtk_label_new("Status: ");
	gtk_widget_show(label);
	entry = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	//g_signal_connect(GTK_OBJECT(button), "clicked",
		//GTK_SIGNAL_FUNC(avc_record), node);
	gtk_widget_show(entry);
	//gtk_table_attach_defaults(GTK_TABLE(table), button, 6, 8, 1, 2);
	gtk_box_pack_start(GTK_BOX(hbox3), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox3), entry, TRUE, TRUE, 0);

	gtk_widget_show(hbox1);
	gtk_widget_show(hbox2);
	gtk_widget_show(hbox3);
	vbox = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, FALSE, 0);
	gtk_widget_show(vbox);

	status_entry->node = node;
	status_entry->entry = entry;

	g_timeout_add(500, update_avc_status, status_entry);

	return vbox;
}

#define MAXTEXTLEAFCHARS 1000
#define VIDEO_MONITOR (AVC_SUBUNIT_TYPE_VIDEO_MONITOR>>19)
#define DISC_RECORDER (AVC_SUBUNIT_TYPE_DISC_RECORDER>>19)
#define TAPE_RECORDER (AVC_SUBUNIT_TYPE_TAPE_RECORDER>>19)
#define TUNER (AVC_SUBUNIT_TYPE_TUNER>>19)
#define VIDEO_CAMERA (AVC_SUBUNIT_TYPE_VIDEO_CAMERA>>19)
#define VENDOR_UNIQUE (AVC_SUBUNIT_TYPE_VENDOR_UNIQUE>>19)
#define NOTHING 0xFF
#define VIDEO_MONITOR_S "Video Monitor"
#define DISC_RECORDER_S "Disc Recorder"
#define TAPE_RECORDER_S "Tape Recorder"
#define TUNER_S "Tuner"
#define VIDEO_CAMERA_S "Video Camera"
#define VENDOR_UNIQUE_S "Vendor Unique"
#define MAXAVCSTRINGCHARS 200

void append_subunit_strings(char *buf, quadlet_t table[8]) {
	int i, j;
	int entry;
	int id;
	int count;
	char *s;
	for (i=0; i<8; i++) {
		for (j=3; j>=0; j--) {
			entry = (table[i] >> (j * 8)) & 0xFF;
			if (entry == NOTHING) continue;
			id = entry >> 3;
			count = (entry & 7) + 1;
			switch (id) {
				case VIDEO_MONITOR:
					s = VIDEO_MONITOR_S; break;
				case DISC_RECORDER:
					s = DISC_RECORDER_S; break;
				case TAPE_RECORDER:
					s = TAPE_RECORDER_S; break;
				case TUNER:
					s = TUNER_S; break;
				case VIDEO_CAMERA:
					s = VIDEO_CAMERA_S; break;
				case VENDOR_UNIQUE:
					s = VENDOR_UNIQUE_S; break;
				default:
					s = "Unknown";
			}
			strcpy(buf, s);
			buf += strlen(s);
			strcpy(buf, ": ");
			buf += 2;
			*buf++ = count + '0';
			*buf++ = '\n';
		}
	}
	*buf++ = 0;
}

/*
 * Popup a dialog displaying various detailed information about a particular
 * node.
 * IN:		node: The node to display information about
 */
void popup_nodeinfo(TopologyTree *node) {
	GtkWidget *button, *dialog_window, *hbox, *text, *sw;
	char *s;
	char textualleafes[MAXTEXTLEAFCHARS];
	int nchars, nleafes, i;
	quadlet_t table[8];
	char avcstring[MAXAVCSTRINGCHARS];

	dialog_window = gtk_dialog_new();
	g_signal_connect(GTK_OBJECT(dialog_window), "destroy",
		GTK_SIGNAL_FUNC(ClosingDialog),
		&dialog_window);
	if (node->label != NULL && node->label[0] != '\0') {
		gtk_window_set_title(GTK_WINDOW(dialog_window),
		node->label);
	} else
		gtk_window_set_title(GTK_WINDOW(dialog_window), "Node Info");
	gtk_container_set_border_width(GTK_CONTAINER(dialog_window), 5);
	/*gtk_window_set_default_size(GTK_WINDOW(dialog_window), 300, 250);*/
	gtk_window_set_default_size(GTK_WINDOW(dialog_window), 300, 300);

	nleafes = node->rom_info.nr_textual_leafes;
	nchars = (nleafes != 0) ? MAXTEXTLEAFCHARS / nleafes : 0;
	textualleafes[0] = '\0';
	if (nchars != 0) {
		for(i=0; i<nleafes; i++) {
			if (node->rom_info.textual_leafes[i] != NULL) {
				strncat(textualleafes, "\n", 1);
				strncat(textualleafes, node->rom_info
					.textual_leafes[i], nchars);
			}
		}
	}

	DEBUG_GENERAL fprintf(stderr,"Getting AVC subunit info\n");
	if (get_node_type(&node->rom_info) == NODE_TYPE_AVC) {
		if (avc_subunit_info(handle, node->selfid[0].packetZero.phyID,table) < 0) {
			strcpy(avcstring, "Error getting subunit info\n");
		} else {
			append_subunit_strings(avcstring, table);
		}
	} else {
		strcpy(avcstring, "N/A\n");
	}
	DEBUG_GENERAL fprintf(stderr,"Got AVC subunit info\n");

	//sprintf(s, "SelfID Info\n-----------\nPhysical ID: %i\nLink active: %s\nGap Count: %i\nPHY Speed: %s\nPHY Delay: %s\nIRM Capable: %s\nPower Class: %s\nPort 0: %s\nPort 1: %s\nPort 2: %s\nInit. reset: %s\n\nCSR ROM Info\n------------\nGUID: 0x%08X%08X\nNode Capabilities: 0x%08X\nVendor ID: 0x%08X\nUnit Spec ID: 0x%08X\nUnit SW Version: 0x%08X\nModel ID: 0x%08X\nNr. Textual Leafes: %i\n\nTextual Leafes: %s\n\nAV/C Subunits\n-------------\n%s",
	s = g_strdup_printf("SelfID Info\n-----------\nPhysical ID: %i\nLink active: %s\nGap Count: %i\nPHY Speed: %s\nPHY Delay: %s\nIRM Capable: %s\nPower Class: %s\n%sInit. reset: %s\n\nCSR ROM Info\n------------\nGUID: 0x%08X%08X\nNode Capabilities: 0x%08X\nVendor ID: 0x%08X\nUnit Spec ID: 0x%08X\nUnit SW Version: 0x%08X\nModel ID: 0x%08X\nNr. Textual Leafes: %i\n\nVendor: %s\nTextual Leafes: %s\n\nAV/C Subunits\n-------------\n%s",
		node->selfid[0].packetZero.phyID,
		yes_no(node->selfid[0].packetZero.linkActive),
		node->selfid[0].packetZero.gapCount,
		decode_speed(node->selfid[0].packetZero.phySpeed),
		decode_delay(node->selfid[0].packetZero.phyDelay),
		yes_no(node->selfid[0].packetZero.contender),
		decode_pwr(node->selfid[0].packetZero.powerClass),
		//decode_port_status(node->selfid[0].packetZero.port0),
		//decode_port_status(node->selfid[0].packetZero.port1),
		//decode_port_status(node->selfid[0].packetZero.port2),
		decode_all_ports_status(node->selfid),
		yes_no(node->selfid[0].packetZero.initiatedReset),
		node->rom_info.guid_hi, node->rom_info.guid_lo,
		node->rom_info.node_capabilities,
		node->rom_info.vendor_id,
		node->rom_info.unit_spec_id,
		node->rom_info.unit_sw_version,
		node->rom_info.model_id,
		node->rom_info.nr_textual_leafes,
		node->rom_info.vendor,
		textualleafes,
		avcstring);

	text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
	gtk_text_buffer_insert_at_cursor(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)), s, -1);
	g_free(s);

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(sw), text);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox),
		sw, TRUE, TRUE, 0);

	if (get_node_type(&node->rom_info) == NODE_TYPE_AVC) {
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox),
			make_avc_buttons(node), FALSE, TRUE, 0);
	}

	button = gtk_button_new_with_label("OK");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(CloseDialog),
		dialog_window);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area),
		button, TRUE, TRUE, 0);
	gtk_widget_grab_default(button);
	gtk_widget_show(button);

	gtk_widget_show_all(dialog_window);
}

/*
 * Called when the mouse button is pressed in the main window. Check if a node
 * was clicked, and pop up an information dialog if so.
 * IN:		widget:	not used
 * 		event:	the button press event
 * RESULT:	TRUE
 */
static gint button_press_event(GtkWidget *widget, GdkEventButton *event) {
	int x, y, width, height;
	GdkModifierType state;
	TopologyTree *node;

	if (event->type == GDK_BUTTON_PRESS) {
		x = event->x;
		y = event->y;
		state = event->state;
		gdk_drawable_get_size(GDK_DRAWABLE(event->window), &width, &height);
		node = detectClick(topologyTree, 0, width, 0, x, y);
		if (node != NULL) {
			popup_nodeinfo(node);
		}
	}
	return TRUE;
}

/*
 * Called whenever a bus reset has occured before the last read was startet.
 * Calls Repaint().
 */
int bus_reset_handler(raw1394handle_t handle, unsigned int generation) {
	DEBUG_GENERAL fprintf(stderr,
		"Bus reset - current generation number: %d\n", generation);
	raw1394_update_generation(handle, generation);
	//Repaint((gpointer) drawing_area);
	repaintCountdown = 10;	/* Repaint 10 times until reset is finished */
	return 0;
}

/*
 * Called periodically by the timer.
 * Performs a dummy read to the cycle time register on the local node to
 * trigger the bus_reset_handler callback whenever a bus reset has occured.
 */
gint dummy_read(gpointer data) {
	quadlet_t quadlet;

	/* Repaint if repaintCountdown is running */
	if (repaintCountdown) {
		repaintCountdown--;
		Repaint((gpointer) drawing_area);
	}
	cooked1394_read(handle, 0xffc0 | raw1394_get_local_id(handle),
		CSR_REGISTER_BASE + CSR_CYCLE_TIME, 4,
		(quadlet_t *) &quadlet);
	return TRUE;
}

/*
 * build the drawing area for the topology tree.
 * IN:		x: horizontal size of drawing area
 * 		y: vertical size of drawing area
 * RETURNS:	handle of the drawing area widget
 */
GtkWidget *makeDrawingArea(int x, int y) {
	GtkWidget *drawing_area;

	drawing_area = gtk_drawing_area_new ();
	gtk_widget_set_size_request(GTK_WIDGET(drawing_area), x, y);
	gtk_widget_show (drawing_area);
	g_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
		GTK_SIGNAL_FUNC (expose_event), NULL);
	g_signal_connect (GTK_OBJECT (drawing_area), "configure_event",
		GTK_SIGNAL_FUNC (configure_event), NULL);
	g_signal_connect (GTK_OBJECT (drawing_area), "button_press_event",
		GTK_SIGNAL_FUNC (button_press_event), NULL);
	gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK);
	return drawing_area;
}

/*
 * The main Method. Get a handle from the 1394 subsystem, initialize the GTK
 * GUI, then enter the GTK event loop.
 */
int main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *menu_bar;
	quadlet_t quadlet;
	int c, level;
	int port = 0;
	/* Parse command line options */
	const char *optstring = "p:v::";

	do {
		c = getopt(argc, argv, optstring);
		switch(c) {
			case 'v':
				if (optarg == NULL) level = 1;
				else level = atoi(optarg);
				fprintf(stderr, "Debug level: %i\n", level);
				set_debug_level(level);
				break;
 		        case 'p':
			        if (optarg == NULL) port = 0;
				else port = atoi(optarg);
	 
		}
	} while (c != -1);

	/* Initialize 1394, check if we have access */
	handle = raw1394_new_handle();

        if (!handle) {
                if (!errno) {
                        fprintf(stderr, not_compatible);
                } else {
                        perror("couldn't get handle");
                        fprintf(stderr, not_loaded);
                }
                exit(1);
        }

        DEBUG_GENERAL fprintf(stderr, "successfully got handle\n");
        DEBUG_GENERAL fprintf(stderr, "current generation number: %d\n",
		raw1394_get_generation(handle));
	if (raw1394_set_port(handle, port) < 0) {
		perror("couldn't set port");
		exit(1);
	}

	DEBUG_GENERAL fprintf(stderr,"using first card found: %d nodes on bus, local ID is %d\n",
		raw1394_get_nodecount(handle),
		raw1394_get_local_id(handle) & 0x3f);

	if (cooked1394_read(handle, 0xffc0 | raw1394_get_local_id(handle),
		CSR_REGISTER_BASE + CSR_CYCLE_TIME, 4,
		(quadlet_t *) &quadlet) < 0) {
		fprintf(stderr, "something is wrong here\n");
	}

	raw1394_set_bus_reset_handler(handle, bus_reset_handler);

	gtk_init (&argc, &argv);
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	g_signal_connect (GTK_OBJECT (window), "destroy",
		GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	menu_bar = makeMenuBar(window);
	drawing_area = makeDrawingArea(640, 480);

	gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), drawing_area, TRUE, TRUE, 0);

	gtk_widget_show_all (window);
	Repaint((gpointer) drawing_area);
	g_timeout_add(100, dummy_read, NULL);

	gtk_main ();	/* Should never return */

	DEBUG_GENERAL fprintf(stderr, "Closing down.");

	return 0;
}

