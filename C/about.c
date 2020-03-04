/*
 * See good explanation of dialog boxes at:
 * https://developer.gnome.org/gtk3/stable/GtkDialog.html
 */
#include <purim_api.h>
#include "ozneyHaman_image.h"


/****************************************************************/
/* Open a dialog box with the About message                     */
/****************************************************************/
void show_about_window (GtkWindow *parent, gchar *message)
{
 GtkWidget *dialog, *label, *content_area, *purim_image;
 GtkDialogFlags flags;
 GdkPixbuf *pixbuf;
 
 // Create the widgets
 flags = GTK_DIALOG_MODAL;
 dialog = gtk_dialog_new_with_buttons ("אודות",
                                       parent,
                                       flags,
                                       "_OK",
                                       GTK_RESPONSE_NONE,
                                       NULL);
 content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
 label = gtk_label_new (message);
#if 1
 pixbuf = gdk_pixbuf_new_from_inline(-1, ozneyHaman_inline, FALSE, NULL);
 purim_image = gtk_image_new_from_pixbuf(pixbuf);
#else 
 purim_image = gtk_image_new_from_file("../ozneyHaman.jpg");
#endif
 
 // Ensure that the dialog box is destroyed when the user responds

 g_signal_connect_swapped (dialog,
                           "response",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog);

 // Add the label, and show everything we’ve added

 gtk_container_add (GTK_CONTAINER (content_area), label);
 gtk_container_add (GTK_CONTAINER (content_area), purim_image);
 gtk_widget_show_all (dialog);
}
