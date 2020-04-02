#ifndef _MAIN_CB_H_
#define _MAIN_CB_H_

#include <purim_api.h>

gboolean callback_button_pressed_about(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
void row_selected_callback (GtkListBox *box, GtkListBoxRow *row, gpointer user_data);
gint CloseAppWindow_callback (GtkWidget *widget, gpointer *data);
gboolean newDB_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean help_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean add_family_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean del_family_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean chg_family_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean add_group_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean del_group_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean extra_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean shipmentsnum_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean calculate_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean saveDB_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean loadDB_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean save_shipments_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean load_shipments_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean manual_chg_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean make_notes_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gboolean make_note_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
gint timeout_make_notes_callback (gpointer user_data);
#endif /* _MAIN_CB_H_ */
