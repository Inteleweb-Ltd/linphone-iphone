/***************************************************************************
 *            gtk/conference.c
 *
 *  Mon Sep 12, 2011
 *  Copyright  2011  Belledonne Communications
 *  Author: Simon Morlat
 *  Email simon dot morlat at linphone dot org
 ****************************************************************************/

/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "linphone.h"

static GtkWidget *create_conference_label(void){
	GtkWidget *box=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),gtk_image_new_from_stock(GTK_STOCK_ADD,GTK_ICON_SIZE_MENU),FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(box),gtk_label_new(_("Conference")),TRUE,FALSE,0);
	gtk_widget_show_all(box);
	return box;
}

static void init_local_participant(GtkWidget *participant){
	GtkWidget *sound_meter;
	gtk_label_set_markup(GTK_LABEL(linphone_gtk_get_widget(participant,"callee_name_label")),_("Me"));
	sound_meter=linphone_gtk_get_widget(participant,"sound_indicator");
	gtk_widget_hide(linphone_gtk_get_widget(participant,"hangup_button"));
	linphone_gtk_init_audio_meter(sound_meter, (get_volume_t) linphone_core_get_conference_local_input_volume, linphone_gtk_get_core());
}

static GtkWidget *get_conference_tab(GtkWidget *mw){
	GtkWidget *box=(GtkWidget*)g_object_get_data(G_OBJECT(mw),"conference_tab");
	if (box==NULL){
		GtkWidget *box=gtk_vbox_new(FALSE,0);
		GtkWidget *participant=linphone_gtk_create_widget("main","callee_frame");
		init_local_participant(participant);
		gtk_box_pack_start(GTK_BOX(box),participant,FALSE,FALSE,0);
		gtk_widget_show(box);
		g_object_set_data(G_OBJECT(mw),"conference_tab",box);
		gtk_notebook_append_page(GTK_NOTEBOOK(linphone_gtk_get_widget(mw,"viewswitch")),box,
		                         create_conference_label());
	}
	return box;
}

void linphone_gtk_add_to_conference(LinphoneCall *call){
	GtkWidget *mw=linphone_gtk_get_main_window();
	GtkWidget *tab=get_conference_tab(mw);
	GtkWidget *participant=linphone_gtk_create_widget("main","callee_frame");
	const char *hangup_image=linphone_gtk_get_ui_config("stop_call_icon","stopcall-red.png");
	const LinphoneAddress *addr=linphone_call_get_remote_address(call);
	GtkWidget *sound_meter;
	GtkWidget *viewswitch=linphone_gtk_get_widget(mw,"viewswitch");
	gchar *markup;
	if (linphone_address_get_display_name(addr)!=NULL){
		markup=g_strdup_printf("<b>%s</b>",linphone_address_get_display_name(addr));
	}else{
		char *tmp=linphone_address_as_string_uri_only(addr);
		markup=g_strdup_printf("%s",tmp);
		ms_free(tmp);
	}
	gtk_label_set_markup(GTK_LABEL(linphone_gtk_get_widget(participant,"callee_name_label")),markup);
	g_free(markup);
	gtk_button_set_image(GTK_BUTTON(linphone_gtk_get_widget(participant,"hangup_button")),create_pixmap(hangup_image));
	sound_meter=linphone_gtk_get_widget(participant,"sound_indicator");
	linphone_gtk_init_audio_meter(sound_meter, (get_volume_t) linphone_call_get_play_volume, call);
	gtk_box_pack_end(GTK_BOX(tab),participant,FALSE,FALSE,0);
	g_object_set_data_full(G_OBJECT(participant),"call",linphone_call_ref(call),(GDestroyNotify)linphone_call_unref);
	gtk_widget_show(participant);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(viewswitch),
	                              gtk_notebook_page_num(GTK_NOTEBOOK(viewswitch),tab));
	linphone_core_add_to_conference(linphone_gtk_get_core(),call);
}

static GtkWidget *find_conferencee_from_call(LinphoneCall *call){
	GtkWidget *mw=linphone_gtk_get_main_window();
	GtkWidget *tab=get_conference_tab(mw);
	GList *elem;
	GtkWidget *ret=NULL;
	GList *l=gtk_container_get_children(GTK_CONTAINER(tab));
	for(elem=l;elem!=NULL;elem=elem->next){
		GtkWidget *frame=(GtkWidget*)elem->data;
		if (call==g_object_get_data(G_OBJECT(frame),"call")){
			ret=frame;
			break;
		}
	}
	g_list_free(l);
	return ret;
}

void linphone_gtk_remove_from_conference(LinphoneCall *call){
	GtkWidget *frame=find_conferencee_from_call(call);
	if (frame){
		gtk_widget_destroy(frame);
	}
}

