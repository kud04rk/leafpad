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
#ifdef HAVE_MAEMO
#include <hildon/hildon-font-selection-dialog.h>
#endif
#endif

void set_text_font_by_name(GtkWidget *widget, gchar *fontname)
{
	PangoFontDescription *font_desc;
	
	font_desc = pango_font_description_from_string(fontname);
	gtk_widget_modify_font(widget, font_desc);
	pango_font_description_free(font_desc);
}

gchar *get_font_name_from_widget(GtkWidget *widget) /* MUST BE FREED */
{
	GtkStyle *style;
	
	style = gtk_widget_get_style(widget);
	return pango_font_description_to_string(style->font_desc);
}

static gchar *get_font_name_by_selector(GtkWidget *window, gchar *current_fontname)
{
#ifdef USE_HILDON
	HildonFontSelectionDialog *dialog;
	PangoFontDescription *font = NULL;
	gint size, weight;
	gboolean bold, italic, style;
	const gchar *family = NULL;
#else
	GtkWidget *dialog;
#endif
	gchar *fontname;
	
#ifdef USE_HILDON
	dialog = HILDON_FONT_SELECTION_DIALOG (hildon_font_selection_dialog_new (NULL, _("Font")));
       	font = pango_font_description_new ();
	font = pango_font_description_from_string (current_fontname);
	family = pango_font_description_get_family (font);
	size = pango_font_description_get_size (font) / PANGO_SCALE;
	weight = pango_font_description_get_weight (font);
	style = pango_font_description_get_style (font);
        g_object_set (G_OBJECT (dialog),
                      "bold", weight == PANGO_WEIGHT_BOLD ? TRUE : FALSE,
                      "italic", style == PANGO_WEIGHT_NORMAL ? TRUE : FALSE,
                      "size", size,
                      "family", family,
		      "color-set", FALSE,
		      "underline", FALSE,
		      "strikethrough", FALSE,
		      "position", -1,
                      NULL);

#else
	dialog = gtk_font_selection_dialog_new(_("Font"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), current_fontname);
#endif
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
#ifdef USE_HILDON
	{
		g_object_get (G_OBJECT(dialog),
                    "family", &family, "size", &size, "bold", &bold, "italic", &italic, NULL);
		pango_font_description_set_family (font, family);
		pango_font_description_set_size (font, size * PANGO_SCALE);
		if (bold) {
			pango_font_description_set_weight (font, PANGO_WEIGHT_BOLD);
		} else {
			pango_font_description_set_weight (font, PANGO_WEIGHT_NORMAL);
		}
		if (italic) {
			pango_font_description_set_style (font, PANGO_STYLE_ITALIC);
		} else {
			pango_font_description_set_style (font, PANGO_STYLE_NORMAL);
		}

		fontname = pango_font_description_to_string (font);
	}
#else
		fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
#endif
	else
		fontname = NULL;
	gtk_widget_destroy (GTK_WIDGET (dialog));
		
	return fontname;
}

void change_text_font_by_selector(GtkWidget *widget)
{
	gchar *current_fontname, *fontname;
	
	current_fontname = get_font_name_from_widget(widget);
	fontname = get_font_name_by_selector(
		gtk_widget_get_toplevel(widget), current_fontname);
	if (fontname) {
		set_text_font_by_name(widget, fontname);
		indent_refresh_tab_width(widget);
	}
	
	g_free(fontname);
	g_free(current_fontname);
}

