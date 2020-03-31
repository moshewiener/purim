#include <purim_api.h>
#include "msg_icons.h"


/**********************************************************************
 * Open a dialog box with Yes/No options
 * Returns GTK_RESPONSE_YES, GTK_RESPONSE_NO, GTK_RESPONSE_DELETE_EVENT
 ***********************************************************************/
GtkResponseType msgBoxYesNo ( GtkWindow *window, char *msg )
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *err_image;
    GdkPixbuf *pixbuf;
    gint response_id;
    
    /*Create the dialog window. Modal windows prevent interaction with other 
     * windows in the same application*/
    dialog = gtk_dialog_new_with_buttons ("Yes-No",
                                          window, 
                                          GTK_DIALOG_MODAL, 
                                          GTK_STOCK_YES,
                                          GTK_RESPONSE_YES,
                                          GTK_STOCK_NO,
                                          GTK_RESPONSE_NO,
                                          NULL);
    
    /*Create a label and attach it to the content area of the dialog*/
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    label = gtk_label_new ( msg );
    gtk_container_add (GTK_CONTAINER (content_area), label);
    gtk_widget_show( label );
    pixbuf = gdk_pixbuf_new_from_inline(-1, questionicon_inline, FALSE, NULL);
    err_image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_container_add (GTK_CONTAINER (content_area), err_image);
    gtk_window_set_default_size( dialog, 200, 100 );
    gtk_widget_show( err_image );
    
    response_id = gtk_dialog_run (dialog);
    gtk_widget_destroy (dialog);
    return response_id;
}
/**********************************************************************
 * Open a dialog box with OK option
 * Returns GTK_RESPONSE_OK, GTK_RESPONSE_DELETE_EVENT
 ***********************************************************************/
GtkResponseType msgBoxError ( GtkWindow *window, char *msg )
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *err_image;
    GdkPixbuf *pixbuf;
    gint response_id;
    
    /*Create the dialog window. Modal windows prevent interaction with other 
     * windows in the same application*/
    dialog = gtk_dialog_new_with_buttons ("Error",
                                          window, 
                                          GTK_DIALOG_MODAL, 
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_OK,
                                          NULL);
    
    /*Create a label and attach it to the content area of the dialog*/
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    label = gtk_label_new ( msg );
    gtk_container_add (GTK_CONTAINER (content_area), label);
    gtk_widget_show( label );
    pixbuf = gdk_pixbuf_new_from_inline(-1, erricon48_inline, FALSE, NULL);
    err_image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_container_add (GTK_CONTAINER (content_area), err_image);
    gtk_window_set_default_size( dialog, 200, 100 );
    gtk_widget_show( err_image );
    
    response_id = gtk_dialog_run (dialog);
    gtk_widget_destroy (dialog);
    return response_id;
}

/**********************************************************************
 * Open a dialog box with OK option
 * Returns GTK_RESPONSE_OK, GTK_RESPONSE_DELETE_EVENT
 ***********************************************************************/
GtkResponseType msgBoxSuccess ( GtkWindow *window, char *msg )
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *image;
    GdkPixbuf *pixbuf;
    gint response_id;
    
    /*Create the dialog window. Modal windows prevent interaction with other 
     * windows in the same application*/
    dialog = gtk_dialog_new_with_buttons ("Success",
                                          window, 
                                          GTK_DIALOG_MODAL, 
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_OK,
                                          NULL);
    
    /*Create a label and attach it to the content area of the dialog*/
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    label = gtk_label_new ( msg );
    gtk_container_add (GTK_CONTAINER (content_area), label);
    gtk_widget_show( label );
    pixbuf = gdk_pixbuf_new_from_inline(-1, successionicon_inline, FALSE, NULL);
    image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_container_add (GTK_CONTAINER (content_area), image);
    gtk_window_set_default_size( dialog, 200, 100 );
    gtk_widget_show( image );
    
    response_id = gtk_dialog_run (dialog);
    gtk_widget_destroy (dialog);
    return response_id;
}

/****************************************************************
 * Caller of this function needs to free the returned file name *
 ****************************************************************/
char *msgBoxOpenfile( char *fileFilter, char *title )
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    GtkFileFilter *filter;
    char *filename = NULL;
    gint res;
    
    dialog = gtk_file_chooser_dialog_new ((title == NULL)? "Open File" : title,
                                          window,
                                          action,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, (fileFilter==NULL)? "*.*" : fileFilter);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    gtk_window_set_default_size( dialog, 800, 600 );
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        gtk_file_chooser_set_create_folders( chooser, FALSE );
        filename = gtk_file_chooser_get_filename (chooser);
    }
    
    gtk_widget_destroy (dialog);
    return filename;
}

/****************************************************************
 * Caller of this function needs to free the returned file name *
 ****************************************************************/
char *msgBoxSavefile( char *fileFilter, char *title )
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    GtkFileFilter *filter;
    char *filename = NULL;
    gint res;
    
    dialog = gtk_file_chooser_dialog_new ((title == NULL)? "Open File" : title,
                                          window,
                                          action,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, (fileFilter==NULL)? "*.*" : fileFilter);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    gtk_window_set_default_size( dialog, 800, 600 );
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        gtk_file_chooser_set_create_folders( chooser, FALSE );
        filename = gtk_file_chooser_get_filename (chooser);
    }
    
    gtk_widget_hide (dialog);
    gtk_widget_destroy (dialog);
    return filename;
}
