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
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_HILDON
#include <hildon/hildon.h>
//#include "font-dialog.h"

#endif

static void set_selection_bound(GtkTextBuffer *buffer, gint start, gint end)
{
	GtkTextIter start_iter, end_iter;
	
	gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
	if (end < 0)
		gtk_text_buffer_get_end_iter(buffer, &end_iter);
	else
		gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
	gtk_text_buffer_place_cursor(buffer, &end_iter);
	gtk_text_buffer_move_mark_by_name(buffer, "selection_bound", &start_iter);
}

void on_file_new(void)
{
	gchar *comline;
	gchar *option;

	save_config_file();
	option = pub->fi->charset_flag ?
		g_strdup_printf("%s%s", " --codeset=", pub->fi->charset) : "";
	comline = g_strdup_printf("%s%s", PACKAGE, option);
	if (pub->fi->charset_flag)
		g_free(option);
	g_spawn_command_line_async(comline, NULL);
	g_free(comline);
}

void on_file_open(void)
#ifdef ENABLE_CSDI
{ // too slow...
	FileInfo *fi;
	gchar *comline;
	gchar *option = NULL;
	
	fi = get_fileinfo_from_selector(pub->fi, OPEN);
	if (fi) {
		save_config_file();
		option = g_strdup_printf("--codeset=%s ", fi->charset);
		comline = g_strdup_printf("%s %s%s", PACKAGE,
			fi->charset ? option : "",
			fi->filename);
		g_spawn_command_line_async(comline, NULL);
		g_free(option);
		g_free(comline);
		g_free(fi);
	}
}
#else
{
	FileInfo *fi;
	
	if (check_text_modification())
		return;
	fi = get_fileinfo_from_selector(pub->fi, OPEN);
	if (fi) {
		if (file_open_real(pub->mw->view, fi))
			g_free(fi);
		else {
			g_free(pub->fi);
			pub->fi = fi;
			undo_clear_all(pub->mw->buffer);
//			set_main_window_title();
			force_call_cb_modified_changed(pub->mw->view);
//			undo_init(sd->mainwin->textview, sd->mainwin->textbuffer, sd->mainwin->menubar);
		}
	}
}
#endif

gint on_file_save(void)
{
	if (pub->fi->filename == NULL)
		return on_file_save_as();
	if (check_file_writable(pub->fi->filename) == FALSE)
		return on_file_save_as();
	if (file_save_real(pub->mw->view, pub->fi))
		return -1;
//	set_main_window_title();
	force_call_cb_modified_changed(pub->mw->view);
//	undo_reset_step_modif();
	return 0;
}

gint on_file_save_as(void)
{
	FileInfo *fi;
	
	fi = get_fileinfo_from_selector(pub->fi, SAVE);
	if (fi == NULL)
		return -1;
	if (file_save_real(pub->mw->view, fi)) {
		g_free(fi);
		return -1;
	}
	g_free(pub->fi);
	pub->fi = fi;
	undo_clear_all(pub->mw->buffer);
//	set_main_window_title();
	force_call_cb_modified_changed(pub->mw->view);
//	undo_init(sd->mainwin->textview, sd->mainwin->textbuffer, sd->mainwin->menubar);
	return 0;
}

void on_file_stats(void)
{
	gchar * stats = file_stats( pub->mw->view, pub->fi );

	GtkMessageDialog * msg = (GtkMessageDialog *) 
		gtk_message_dialog_new_with_markup( NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO,
#ifndef USE_HILDON
			GTK_BUTTONS_OK,
#else
			GTK_BUTTONS_NONE,
#endif
			_("<b>Statistics</b>")
	);

 	gtk_message_dialog_format_secondary_markup(
		msg,
        	"<i>%s</i>",
		stats
	);
 
	gtk_window_set_title( GTK_WINDOW(msg),
		pub->fi->filename );
	gtk_window_set_transient_for(GTK_WINDOW(msg),
		GTK_WINDOW(pub->mw->window));
	gtk_dialog_run( (GtkDialog *) msg );
	gtk_widget_destroy( (GtkWidget *) msg );

	g_free( stats );
}

#ifdef ENABLE_PRINT
#	if GTK_CHECK_VERSION(2, 10, 0)
void on_file_print_preview(void)
{
	create_gtkprint_preview_session(GTK_TEXT_VIEW(pub->mw->view),
		get_file_basename(pub->fi->filename, FALSE));
}

void on_file_print(void)
{
	create_gtkprint_session(GTK_TEXT_VIEW(pub->mw->view),
		get_file_basename(pub->fi->filename, FALSE));
}
#	else
void on_file_print(void)
{
	create_gnomeprint_session();
}
#	endif
#endif
void on_file_close(void)
{
	if (!check_text_modification()) {
		force_block_cb_modified_changed(pub->mw->view);
//		undo_block_signal(textbuffer);
		gtk_text_buffer_set_text(pub->mw->buffer, "", 0);
		gtk_text_buffer_set_modified(pub->mw->buffer, FALSE);
		if (pub->fi->filename)
			g_free(pub->fi->filename);
		pub->fi->filename = NULL;
		if (pub->fi->charset)
			g_free(pub->fi->charset);
		pub->fi->charset = NULL;
		pub->fi->charset_flag = FALSE;
		pub->fi->lineend = LF;
		undo_clear_all(pub->mw->buffer);
//		set_main_window_title();
		force_call_cb_modified_changed(pub->mw->view);
		force_unblock_cb_modified_changed(pub->mw->view);
//		undo_unblock_signal(textbuffer);
//		undo_init(sd->mainwin->textview, textbuffer, sd->mainwin->menubar);
	}
}

void on_file_quit(void)
{
	if (!check_text_modification()) {
		save_config_file();
		gtk_main_quit();
	}
}

void on_edit_undo(void)
{
	undo_undo(pub->mw->buffer);
}

void on_edit_redo(void)
{
	undo_redo(pub->mw->buffer);
}

void on_edit_cut(void)
{
	g_signal_emit_by_name(G_OBJECT(pub->mw->view), "cut-clipboard");
}

void on_edit_copy(void)
{
	g_signal_emit_by_name(G_OBJECT(pub->mw->view), "copy-clipboard");
}

void on_edit_paste(void)
{
	g_signal_emit_by_name(G_OBJECT(pub->mw->view), "paste-clipboard");
//	TODO: Use modify signal!!
/*	gtk_text_view_scroll_mark_onscreen(
		GTK_TEXT_VIEW(pub->mw->view),
		gtk_text_buffer_get_insert(pub->mw->buffer));
*/}

void on_edit_delete(void)
{
	gtk_text_buffer_delete_selection(pub->mw->buffer, TRUE, TRUE);
}

void on_edit_select_all(void)
{
	set_selection_bound(pub->mw->buffer, 0, -1);
//	g_signal_emit_by_name(G_OBJECT(pub->mw->view), "select-all");
}

void on_view_fullscreen(void)
{
	GtkItemFactory *ifactory;
	gboolean state;

	ifactory = gtk_item_factory_from_widget(pub->mw->menubar);
	state = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(ifactory, "/View/Fullscreen")));

	if (state)
		gtk_window_fullscreen(GTK_WINDOW(pub->mw->window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(pub->mw->window));
}

static void activate_quick_find(void)
{
	GtkItemFactory *ifactory;
	static gboolean flag = FALSE;
	
	if (!flag) {
		ifactory = gtk_item_factory_from_widget(pub->mw->menubar);
#ifndef USE_HILDON
		gtk_widget_set_sensitive(
			gtk_item_factory_get_widget(ifactory, "/Search/Find Next"),
			TRUE);
		gtk_widget_set_sensitive(
			gtk_item_factory_get_widget(ifactory, "/Search/Find Previous"),
			TRUE);
#endif
		flag = TRUE;
	}
}

void on_search_find(void)
{
	if (run_dialog_search(pub->mw->view, 0) == GTK_RESPONSE_OK)
		activate_quick_find();
}

void on_search_find_next(void)
{
	document_search_real(pub->mw->view, 1);
}

void on_search_find_previous(void)
{
	document_search_real(pub->mw->view, -1);
}

void on_search_replace(void)
{
	if (run_dialog_search(pub->mw->view, 1) == GTK_RESPONSE_OK)
		activate_quick_find();
}

void on_search_jump_to(void)
{
	run_dialog_jump_to(pub->mw->view);
}

void on_option_font(void)
{
	change_text_font_by_selector(pub->mw->view);
}

#ifdef USE_HILDON
void on_option_word_wrap(HildonCheckButton *button, gpointer user_data)
{
	gboolean active;
	active = hildon_check_button_get_active (HILDON_CHECK_BUTTON(button));
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pub->mw->view),
		active ? GTK_WRAP_WORD : GTK_WRAP_NONE);
#else
void on_option_word_wrap(void)
{
	GtkItemFactory *ifactory;
	gboolean state;
	
	ifactory = gtk_item_factory_from_widget(pub->mw->menubar);
	state = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(ifactory, "/Options/Word Wrap")));
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pub->mw->view),
		state ? GTK_WRAP_WORD : GTK_WRAP_NONE);
#endif
}

#ifdef USE_HILDON
void on_option_line_numbers(HildonCheckButton *button, gpointer user_data)
{
	gboolean active;
	active = hildon_check_button_get_active (HILDON_CHECK_BUTTON(button));
	show_line_numbers(pub->mw->view, active);
#else
void on_option_line_numbers(void)
{
	GtkItemFactory *ifactory;
	gboolean state;
	
	ifactory = gtk_item_factory_from_widget(pub->mw->menubar);
	state = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(ifactory, "/Options/Line Numbers")));
	show_line_numbers(pub->mw->view, state);
#endif
}

void on_option_always_on_top(void)
{
#if GTK_CHECK_VERSION(2, 4, 0)
	static gboolean flag = FALSE;
	
	flag =! flag;
	gtk_window_set_keep_above(GTK_WINDOW(pub->mw->window), flag);
#endif
}

#ifdef USE_HILDON
void on_option_auto_indent(HildonCheckButton *button, gpointer user_data)
{
	gboolean active;
	active = hildon_check_button_get_active (HILDON_CHECK_BUTTON(button));
	indent_set_state(active);
#else
void on_option_auto_indent(void)
{
	GtkItemFactory *ifactory;
	gboolean state;
	
	ifactory = gtk_item_factory_from_widget(pub->mw->menubar);
	state = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(ifactory, "/Options/Auto Indent")));
	indent_set_state(state);
#endif
}

void on_help_about(void)
{
	const gchar *copyright = "Copyright \xc2\xa9 2004-2008 Tarot Osuji";
	const gchar *comments = _("GTK+ based simple text editor");
	const gchar *authors[] = {
		"Tarot Osuji <tarot@sdf.lonestar.org>\nEduardo Lima <eblima@gmail.com>",
		NULL
	};
	const gchar *translator_credits = _("translator-credits");
	
	translator_credits = strcmp(translator_credits, "translator-credits")
		? translator_credits : NULL;
	
#if GTK_CHECK_VERSION(2, 6, 0)
	const gchar *artists[] = {
		"Lapo Calamandrei <calamandrei@gmail.com>",
		NULL
	};
	gtk_show_about_dialog(GTK_WINDOW(pub->mw->window),
//		"name", PACKAGE_NAME,
		"version", PACKAGE_VERSION,
		"copyright", copyright,
		"comments", comments,
		"authors", authors,
		"artists", artists,
		"translator-credits", translator_credits,
		"logo-icon-name", PACKAGE,
		NULL);
#else
	static GtkWidget *about = NULL;

	if (about != NULL) {
		gtk_window_present(GTK_WINDOW(about));
		return;
	}
	
	const gchar *documenters[] = {
		NULL
	};
	GdkPixbuf *logo = gdk_pixbuf_new_from_file(
		ICONDIR G_DIR_SEPARATOR_S PACKAGE ".png", NULL);
	about = create_about_dialog(
		PACKAGE_NAME,
		PACKAGE_VERSION,
		copyright,
		comments,
		authors,
		documenters,
		translator_credits,
		logo);
	if (logo)
		g_object_unref(logo);
	gtk_window_set_transient_for(GTK_WINDOW(about),
		GTK_WINDOW(pub->mw->window));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(about), TRUE);
	g_signal_connect(G_OBJECT(about), "destroy",
		G_CALLBACK(gtk_widget_destroyed), &about);

	gtk_widget_show(about);
#endif
}

#ifdef USE_HILDON
void on_find_close(void)
{
    gtk_widget_hide (pub->mw->find_toolbar);
}

void on_preferences(void)
{
	GtkWidget *font_button;
	GtkWidget *word_wrap_button;
	GtkWidget *line_numbers_button;
	GtkWidget *auto_indent_button;

	GtkDialog *dialog = GTK_DIALOG(
	gtk_dialog_new_with_buttons(_("Settings"),
                                  GTK_WINDOW(pub->mw->window),
                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                  dgettext("hildon-libs", "wdgt_bd_done"),
                                  GTK_RESPONSE_OK,
                                  NULL));
	word_wrap_button = hildon_check_button_new (HILDON_SIZE_THUMB_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
	gtk_button_set_label (GTK_BUTTON (word_wrap_button), _("Word Wrap"));
	hildon_check_button_set_active (HILDON_CHECK_BUTTON (word_wrap_button), gtk_text_view_get_wrap_mode(GTK_TEXT_VIEW(pub->mw->view)) );
	g_signal_connect (word_wrap_button, "toggled", G_CALLBACK (on_option_word_wrap), NULL);
	gtk_container_add(GTK_CONTAINER(dialog->vbox), word_wrap_button);

	line_numbers_button = hildon_check_button_new (HILDON_SIZE_THUMB_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
	gtk_button_set_label (GTK_BUTTON (line_numbers_button), _("Line Numbers"));
	hildon_check_button_set_active (HILDON_CHECK_BUTTON (line_numbers_button), get_line_numbers_visible());
	g_signal_connect (line_numbers_button, "toggled", G_CALLBACK (on_option_line_numbers), NULL);
	gtk_container_add(GTK_CONTAINER(dialog->vbox), line_numbers_button);

	auto_indent_button = hildon_check_button_new (HILDON_SIZE_THUMB_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
	gtk_button_set_label (GTK_BUTTON (auto_indent_button), _("Auto Indent"));
	hildon_check_button_set_active (HILDON_CHECK_BUTTON (auto_indent_button), indent_get_state());
	g_signal_connect (auto_indent_button, "toggled", G_CALLBACK (on_option_auto_indent), NULL);
	gtk_container_add(GTK_CONTAINER(dialog->vbox), auto_indent_button);

	font_button = hildon_button_new (HILDON_SIZE_THUMB_HEIGHT | HILDON_SIZE_AUTO_WIDTH,HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
	g_signal_connect (font_button, "clicked", G_CALLBACK (on_option_font), NULL);
	hildon_button_set_title (HILDON_BUTTON (font_button), _("Font"));
	gtk_container_add(GTK_CONTAINER(dialog->vbox), font_button);

	gtk_widget_show_all (GTK_WIDGET (dialog));

	gtk_dialog_run(dialog);

	/* close the dialog */
	gtk_widget_destroy(GTK_WIDGET(dialog));

}
#endif

