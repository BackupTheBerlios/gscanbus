/*
 * This file is part of the gscanbus project.
 * Copyright (C) 2001  Andreas Micklei  <nurgle@gmx.de>
 * Copyright (C) 2010  Gareth McMullin  <gareth@blacksphere.co.nz>
 *
 * menues.c - Menues and Transaction Dialogs for gscanbus
 * 		written 11.2.2000 to 6.4.2000 by Andreas Micklei
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

#include <ctype.h>		// isprint()
#include <string.h>		// strlen()
#include "menues.h"

extern raw1394handle_t handle;	// From gscanbus.c

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

typedef struct {
	GtkWidget *entries[3];
	GtkWidget *text;
	GtkWidget *dialog;
} TransactionDialog;

void ClosingTransactionDialog(GtkWidget *widget, gpointer data) {
	free(data);
	gtk_grab_remove(GTK_WIDGET(widget));
}

/*
 * Callback for the about menu item from the menu bar.
 */
static void aboutApp(gpointer callback_data, guint callback_action,
	GtkWidget *widget) {
	GtkWidget *button, *dialog_window, *headline, *text;
	char *sheadline = "gscanbus 0.7.1\nA utility to access the IEEE1394 bus.";
	char *s = "\nwritten 11.07.2001 by Andreas Micklei\n\nMany ideas taken from gnome1394\nby Emanuel Pirker\n\nContributors:\nJim Harkins (ASCII dump, compilation fixes)\nManfred Weihs (Major bugfixes)\nSimon Vogl (endianess fixes)\nMark Knecht (bug reports)\nPK Chen of VIA Technologies, Inc (hardware)";

	dialog_window = gtk_dialog_new();
	g_signal_connect(GTK_OBJECT(dialog_window), "destroy",
		GTK_SIGNAL_FUNC(ClosingDialog),
		&dialog_window);
	gtk_window_set_title(GTK_WINDOW(dialog_window), "About");
	gtk_container_set_border_width(GTK_CONTAINER(dialog_window), 5);
	gtk_window_set_default_size(GTK_WINDOW(dialog_window), 300, 200);

	headline = gtk_label_new(sheadline);
	gtk_widget_show(headline);

	text = gtk_label_new(s);
	gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
	gtk_widget_show(text);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox),
		headline, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox),
		text, TRUE, TRUE, 0);

	button = gtk_button_new_with_label("OK");
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(CloseDialog),
		dialog_window);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area),
		button, TRUE, TRUE, 0);
	gtk_widget_grab_default(button);
	gtk_widget_show(button);

	gtk_widget_show(dialog_window);
}

GtkWidget *makeDialogWindow(char *title) {
	GtkWidget *dialog_window;

	dialog_window = gtk_dialog_new();
	g_signal_connect(GTK_OBJECT(dialog_window), "destroy",
		GTK_SIGNAL_FUNC(ClosingDialog),
		&dialog_window);
	gtk_window_set_title(GTK_WINDOW(dialog_window), title);
	gtk_container_set_border_width(GTK_CONTAINER(dialog_window), 5);
	gtk_window_set_default_size(GTK_WINDOW(dialog_window), 300, 200);
	return dialog_window;
}

TransactionDialog *makeTransactionDialog(char *title) {
	TransactionDialog *transactionDialog;

	transactionDialog = malloc(sizeof(TransactionDialog));
	if (!transactionDialog) fatal("out of memory!");

	transactionDialog->dialog = gtk_dialog_new();
	g_signal_connect(GTK_OBJECT(transactionDialog->dialog), "destroy",
		GTK_SIGNAL_FUNC(ClosingTransactionDialog),
		transactionDialog);
	gtk_window_set_title(GTK_WINDOW(transactionDialog->dialog), title);
	gtk_container_set_border_width(GTK_CONTAINER(transactionDialog->dialog), 5);
	gtk_window_set_default_size(GTK_WINDOW(transactionDialog->dialog),
		300, 200);
	return transactionDialog;
}

GtkWidget *makeLabel(char *s) {
	GtkWidget *label;

	label = gtk_label_new(s);
	gtk_widget_show(label);
	return label;
}

GtkWidget *makeEntry(char *s) {
	GtkWidget *entry;

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), s);
	gtk_widget_show(entry);
	return entry;
}

GtkWidget *makeButton(char *s, GtkSignalFunc func, gpointer data) {
	GtkWidget *button;

	button = gtk_button_new_with_label(s);
	g_signal_connect(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(func), data);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_show(button);
	return button;
}

int isEmptyString(char *s) {
	while (*s == ' ' || *s == '\t') s++;
	if (*s == '\0') return -1;
	else return 0;
}

void scanEditable(GtkWidget *editable, char *format, void *result) {
	char *s = gtk_editable_get_chars(GTK_EDITABLE(editable), 0, -1);
	if (s != NULL) {
		if (!isEmptyString(s)) {
			sscanf(s, format, result);
		}
		g_free(s);
	}
}

GtkWidget *tableAttachEntry(GtkWidget *table, char *slable, char *sentry,
	int pos) {
	GtkWidget *entry = makeEntry(sentry);

	gtk_table_attach_defaults(GTK_TABLE(table),
		makeLabel(slable), 0, 1, pos, pos+1);
	gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 3, pos, pos+1);
	return entry;
}

GtkWidget *tableAttachScrollText(GtkWidget *table, char *slable, char *stext,
	int pos) {
	GtkWidget *hbox, *sw, *text;

	gtk_table_attach_defaults(GTK_TABLE(table),
		makeLabel(slable), 0, 1, pos, pos+1);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	text = gtk_text_view_new();
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(sw), text);
	gtk_widget_set_size_request(text, 230, 200);
	gtk_box_pack_start(GTK_BOX(hbox), sw, FALSE, FALSE, 0);
	gtk_widget_show_all(sw);

	gtk_table_attach_defaults(GTK_TABLE(table), hbox,
		1, 3, pos, pos+1);
	return text;
}

void dialogAddOkClose(GtkWidget *dialog_window, GtkSignalFunc func) {
	GtkWidget *button_ok, *button_cancel;

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area),
		button_ok = makeButton("OK", GTK_SIGNAL_FUNC(func),
		dialog_window), TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area),
		button_cancel = makeButton("Close",
		GTK_SIGNAL_FUNC(CloseDialog), dialog_window), TRUE, TRUE, 0);

	gtk_widget_grab_default(button_ok);
}

void transactionDialogAddOkClose(TransactionDialog *dialog,
	GtkSignalFunc func) {
	GtkWidget *button_ok, *button_cancel;

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog->dialog)->action_area),
		button_ok = makeButton("OK", GTK_SIGNAL_FUNC(func),
		dialog), TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog->dialog)->action_area),
		button_cancel = makeButton("Close",
		GTK_SIGNAL_FUNC(CloseDialog), dialog->dialog), TRUE, TRUE, 0);

	gtk_widget_grab_default(button_ok);
}

/*
 * Performs a read quadlet transaction from the read quadlet dialog
 * IN:		widget:	not used
 * 		data:	the dialog
 */
void readQuadletAppOk(GtkWidget *widget, gpointer data) {
	int destId = 0;
	octlet_t offset = 0;
	quadlet_t result;
	gchar sresult[11];
	GtkWidget *entry_destid, *entry_offset, *entry_result;
	TransactionDialog *dialog;

	dialog = (TransactionDialog *) data;
	entry_destid = dialog->entries[0];
	entry_offset = dialog->entries[1];
	entry_result = dialog->entries[2];

	scanEditable(entry_destid, "%i", &destId);
	scanEditable(entry_offset, "%Li", &offset);

	if (cooked1394_read(handle, 0xffc0 | destId, offset, 4, &result) < 0) {
		gtk_entry_set_text(GTK_ENTRY(entry_result), "Error");
	} else {
		snprintf(sresult, 11, "0x%08X", ntohl(result));
		gtk_entry_set_text(GTK_ENTRY(entry_result), sresult);
	}
}

/*
 * Callback for the read quadlet menu item from the menu bar.
 */
static void readQuadletApp(gpointer callback_data, guint callback_action,
	GtkWidget *widget) {
	GtkWidget *table;
	TransactionDialog *dialog;

	dialog = makeTransactionDialog("Read Quadlet");

	table = gtk_table_new(3, 3, FALSE);

	dialog->entries[0] = tableAttachEntry(table, "Destination ID:", "0", 0);
	dialog->entries[1] = tableAttachEntry(table,
		"Memory Offset:", "0xFFFFF0000000", 1);
	dialog->entries[2] = tableAttachEntry(table, "Result:", "", 2);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog->dialog)->vbox),
		table, TRUE, TRUE, 0);

	transactionDialogAddOkClose(dialog, readQuadletAppOk);

	gtk_widget_show_all(dialog->dialog);
}

/*
 * Performs a write quadlet transaction from the write quadlet dialog
 * IN:		widget:	not used
 * 		data:	the dialog
 */
void writeQuadletAppOk(GtkWidget *widget, gpointer data) {
	int destId = 0;
	octlet_t offset = 0;
	quadlet_t writedata;
	gchar swritedata[11];
	GtkWidget *entry_destid, *entry_offset, *entry_data;
	TransactionDialog *dialog;

	dialog = (TransactionDialog *) data;
	entry_destid = dialog->entries[0];
	entry_offset = dialog->entries[1];
	entry_data = dialog->entries[2];

	scanEditable(entry_destid, "%i", &destId);
	scanEditable(entry_offset, "%Li", &offset);
	scanEditable(entry_data, "%i", &writedata);
	writedata = htonl(writedata);

	if (cooked1394_write(handle, 0xffc0 | destId, offset, 4, &writedata) < 0) {
		gtk_entry_set_text(GTK_ENTRY(entry_data), "Error");
	} else {
		snprintf(swritedata, 11, "0x%08X", ntohl(writedata));
		gtk_entry_set_text(GTK_ENTRY(entry_data), swritedata);
	}
}

/*
 * Callback for the write quadlet menu item from the menu bar.
 */
static void writeQuadletApp(gpointer callback_data, guint callback_action,
	GtkWidget *widget) {
	GtkWidget *table;
	TransactionDialog *dialog;

	dialog = makeTransactionDialog("Write Quadlet");

	table = gtk_table_new(3, 3, FALSE);

	dialog->entries[0] = tableAttachEntry(table, "Destination ID:", "0", 0);
	dialog->entries[1] = tableAttachEntry(table, "Memory Offset:",
		"0xFFFFF0000000", 1);
	dialog->entries[2] = tableAttachEntry(table, "Data:", "", 2);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog->dialog)->vbox),
		table, TRUE, TRUE, 0);

	transactionDialogAddOkClose(dialog, writeQuadletAppOk);

	gtk_widget_show_all(dialog->dialog);
}

/*
 * Performs a read block transaction from the read block dialog
 * IN:		widget:	not used
 * 		data:	the dialog
 */
void readBlockAppOk(GtkWidget *widget, gpointer data) {
	int destId, length, i, j, pos = 0, nread;
	octlet_t offset = 0;
	quadlet_t *result, datum;
	gchar sresult[30], text[5];
	GtkWidget *entry_destid, *entry_offset, *entry_length, *text_result;
	TransactionDialog *dialog;

	dialog = (TransactionDialog *) data;
	entry_destid = dialog->entries[0];
	entry_offset = dialog->entries[1];
	entry_length = dialog->entries[2];
	text_result = dialog->text;

	scanEditable(entry_destid, "%i", &destId);
	scanEditable(entry_offset, "%Li", &offset);
	scanEditable(entry_length, "%i", &length);

	result = malloc(length);
	if (!result) fatal("out of memory!");

	nread = cooked1394_read(handle, 0xffc0 | destId, offset, length,result);
 	if (nread < 0) {
		gtk_entry_set_text(GTK_ENTRY(text_result), "Error");
	} else {
		gtk_editable_delete_text(GTK_EDITABLE(text_result), 0, -1);
		pos = 0;
		//for (p=result; p<result+length/4; p++) {
		for (i=0; i<length/4; i++) {
			//printf("%i\n",i);
			for(j = 0, datum = result[i]; j < 4; j++) {
				text[j] =
				    isprint(datum & 0xff) ?  datum & 0xff : '.';
				datum >>= 8;
			}
			text[4] = 0;
			snprintf(sresult, 30, "0x%08X:\t0x%08X  %s\n",
				((quadlet_t)offset)+i*4, ntohl(result[i]), (char *) text);
			gtk_text_buffer_insert_at_cursor(
				gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_result)),
				sresult, strlen(sresult));
		}
	}

	free(result);
}

/*
 * Callback for the read block menu item from the menu bar.
 */
static void readBlockApp(gpointer callback_data, guint callback_action,
	GtkWidget *widget) {
	//GtkWidget *dialog_window, *table;
	GtkWidget *table;
	TransactionDialog *dialog;

	//dialog_window = makeDialogWindow("Read Block");
	dialog = makeTransactionDialog("Read Block");

	table = gtk_table_new(3, 4, FALSE);

	dialog->entries[0] = tableAttachEntry(table, "Destination ID:", "0", 0);
	dialog->entries[1] = tableAttachEntry(table, "Memory Offset:",
		"0xFFFFF0000000", 1);
	dialog->entries[2] = tableAttachEntry(table, "Length:", "4", 2);
	dialog->text = tableAttachScrollText(table, "Result:", "", 3);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog->dialog)->vbox),
		table, TRUE, TRUE, 0);

	//dialogAddOkClose(dialog->dialog, readBlockAppOk);
	transactionDialogAddOkClose(dialog, readBlockAppOk);

	gtk_widget_show_all(dialog->dialog);
}

/*
 * Callback for the force bus reset menu item from the menu bar.
 */
static void forceBusResetApp(gpointer callback_data, guint callback_action,
	GtkWidget *widget) {

	raw1394_reset_bus(handle);

}

/*
 * The data for the GtkItemFactory for the menu bar. This is the easy way to
 * create a menu bar in GTK+. Hopefully it is flexible enough for future
 * versions of this program.
 */
static GtkItemFactoryEntry menu_items[] = {
	{"/_File",		NULL,		0,	0,	"<Branch>" },
	{"/File/tearoff1",	NULL,		0,	0,	"<Tearoff>" },
	{"/File/_Quit",		"<control>Q",	gtk_main_quit,0, },

	{"/_Control",		NULL,		0,	0,	"<Branch>" },
	{"/Control/tearoff1",	NULL,		0,	0,	"<Tearoff>" },
	/*{"/Control/Show _Bus Information...",	0,	0, },
	{"/Control/Show _CSR Space...",		0,	0, },*/
	{"/Control/Force Bus _Reset",		0,	forceBusResetApp, },

	{"/_Transactions",	NULL,		0,	0,	"<Branch>" },
	{"/Transactions/tearoff1",	NULL,	0,	0,	"<Tearoff>" },
	{"/Transactions/Read Quadlet...",	0,	readQuadletApp, },
	{"/Transactions/Write Quadlet...",	0,	writeQuadletApp, },
	{"/Transactions/_Read Block...",	0,	readBlockApp, },
	/*{"/Transactions/_Write Block...",	0,	0, },
	{"/Transactions/_Lock...",		0,	0, },*/

	{"/_Help",		NULL,		0,	0,	"<LastBranch>"},
	{"/Help/_About",	NULL,		aboutApp,0 },
};

static int nmenu_items = sizeof(menu_items)/sizeof(menu_items[0]);

/*
 * build the menu bar
 * IN:		window: pointer to the window. This is needed for adding
 *			keyboard accelerators
 * RETURNS:	pointer to the feshly created and visible menu bar
 */
GtkWidget *makeMenuBar(GtkWidget *window) {
	GtkAccelGroup *accel_group;
	GtkItemFactory *item_factory;
	GtkWidget *menu_bar;

	accel_group = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<blah>",
		accel_group);
	gtk_item_factory_create_items(item_factory, nmenu_items, menu_items,
		NULL);
	//gtk_accel_group_attach(accel_group, GTK_OBJECT(window));
	menu_bar = gtk_item_factory_get_widget(item_factory, "<blah>");
	gtk_widget_show(menu_bar);
	return menu_bar;
}

