/*
 *  Leafpad - GTK+ based simple text editor
 *  Copyright (C) 2004-2005 Tarot Osuji
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "leafpad.h"

#ifdef USE_HILDON

#include <hildon/hildon-program.h>
#include <hildon/hildon-defines.h>
#include <hildon/hildon-text-view.h>
#include <hildon/hildon-pannable-area.h>

static gboolean
key_press_event(GtkWidget *widget, GdkEventKey *event, MainWin *mw)
{
	GtkItemFactory *ifactory;
	gboolean state;

	switch (event->keyval) {
		case HILDON_HARDKEY_FULLSCREEN:
		case 0:
			/*ifactory = gtk_item_factory_from_widget (mw->menubar);
			state = gtk_check_menu_item_get_active (
				GTK_CHECK_MENU_ITEM (gtk_item_factory_get_item (ifactory, "/View/Fullscreen")));

			if (state) {
				gtk_window_unfullscreen (GTK_WINDOW (widget));
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (
						gtk_item_factory_get_item (ifactory, "/View/Fullscreen")), FALSE);

			} else {
				gtk_window_fullscreen (GTK_WINDOW (widget));
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (
						gtk_item_factory_get_item (ifactory, "/View/Fullscreen")), TRUE);
			}*/
			if (mw->fullscreen) {
				gtk_window_unfullscreen (GTK_WINDOW (widget));
				mw->fullscreen = FALSE;
			} else {
				gtk_window_fullscreen (GTK_WINDOW (widget));
				mw->fullscreen = TRUE;
			}
			break;
	}
	return FALSE;
}
#endif

/*
static void cb_scroll_event(GtkAdjustment *adj, GtkWidget *view)
{
	gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(view));
}
*/
MainWin *create_main_window(void)
{
	GtkWidget *window;
	GtkWidget *vbox;
#ifdef USE_HILDON
	HildonAppMenu *menubar;
#else
 	GtkWidget *menubar;
#endif
 	GtkWidget *sw;
 	GtkWidget *view;
// 	gint size;
//	GtkAdjustment *hadj, *vadj;
	
	MainWin *mw = g_malloc(sizeof(MainWin));
	
#ifdef USE_HILDON
	window = hildon_window_new();
	g_signal_connect (window, "key-press-event", G_CALLBACK (key_press_event), mw);
	mw->fullscreen = FALSE;
#else
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif

//	gtk_window_set_title(GTK_WINDOW(window), PACKAGE_NAME);
	gtk_widget_set_name(window, PACKAGE_NAME);

/*
#if GTK_CHECK_VERSION(2, 4, 0)
//	size = gtk_icon_size_lookup(GTK_ICON_SIZE_LARGE_TOOLBAR, &size, NULL);
	icon = gtk_icon_theme_load_icon(
		gtk_icon_theme_get_default(),
		PACKAGE,
		16, // size
		0,  // flags
		NULL);
	gtk_window_set_default_icon(icon);
*/
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_default_icon_name(PACKAGE);
#else
 	GdkPixbuf *icon = gdk_pixbuf_new_from_file(
		ICONDIR G_DIR_SEPARATOR_S PACKAGE ".png", NULL);
	gtk_window_set_icon(GTK_WINDOW(window), icon);
	if (icon)
		g_object_unref(icon);
#endif
	
	g_signal_connect(G_OBJECT(window), "delete-event",
		G_CALLBACK(on_file_quit), NULL);
	g_signal_connect_after(G_OBJECT(window), "delete-event",
		G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	
	menubar = create_menu_bar(window);

#ifdef USE_HILDON
	//gtk_widget_set_name(window, PACKAGE_NAME);
	hildon_window_set_app_menu (HILDON_WINDOW (window), HILDON_APP_MENU  (menubar));
	mw->find_toolbar = create_search_bar (window);
	hildon_window_add_toolbar (HILDON_WINDOW (window), GTK_TOOLBAR (mw->find_toolbar));
	mw->find_visible = FALSE;
	sw = hildon_pannable_area_new();
	g_object_set(sw, "mov-mode", HILDON_MOVEMENT_MODE_BOTH);
#else
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
		GTK_SHADOW_IN);
#endif

	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	
	view = create_text_view();
	gtk_container_add(GTK_CONTAINER(sw), view);
/*	
	hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(sw));
	vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sw));
	g_signal_connect_after(G_OBJECT(hadj), "value-changed",
		G_CALLBACK(cb_scroll_event), view);
	g_signal_connect_after(G_OBJECT(vadj), "value-changed",
		G_CALLBACK(cb_scroll_event), view);
*/		
	mw->window = window;
	mw->menubar = menubar;
	mw->view = view;
#ifdef USE_HILDON
	mw->buffer = hildon_text_view_get_buffer(HILDON_TEXT_VIEW(view));
#else
	mw->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
#endif
	
	return mw;
}

void set_main_window_title(void)
{
	gchar *title;
	
	title = get_file_basename(pub->fi->filename, TRUE);
	gtk_window_set_title(GTK_WINDOW(pub->mw->window), title);
	g_free(title);
}

