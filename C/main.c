/*
 * Example of setting buttons across a window. See good explenation of widget packing at:
 * https://www.cc.gatech.edu/data_files/public/doc/gtk/tutorial/gtk_tut-4.html
 * For listbox explenation see:
 * https://developer.gnome.org/gtk3/stable/GtkListBox.html
 * Compile with: 
 * FILE=main
 * gcc -o $FILE ${FILE}.c about.c -w -I . `pkg-config --cflags --libs gtk+-3.0` -export-dynamic; ./${FILE}
 */
#include <purim_api.h>
#include "main_cb.h"
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include "purim1_image.h"

typedef struct button_data { 
    char title[64];
    int colour;
    int backcolour;
    GtkWidget *p_button;
} button_data;


#define MAX_BUTTONS_IN_FRAME 12


static void set_button_attributes(button_data *btn, gint colour, gint backcolour, gchar *title);
static void add_toolbar_to_box( GtkWidget *box );

GtkWidget *list_box = NULL;
GtkWidget* window = NULL;
static GtkWidget *scrollwin = NULL;

/* Buttons */
GtkWidget *btnAddFamily, *btnDelFamily, *btnChgangeFamily, *btnShipmentsNum, *btnExtra;
GtkWidget *btnLoadDbFile, *btnNewDB, *btnAddGroup, *btnDelGroup, *btnSaveDbFile;
GtkWidget *btnCalculate, *btnSaveCalc, *btnLoadCalc, *btnManual;
GtkWidget *btnDebug, *btnNotes, *btnNote, *btnCancel;
/* other widgets */
GtkWidget *labelMain, *frameMainMsg, *frame_listbox;

/********************************************************/
static void add_toolbar_to_box( GtkWidget *box )
{
    GtkWidget *toolbar;
    GtkToolItem *newTb;
    GtkToolItem *sep;
    GtkToolItem *exitTb;
    
    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_TEXT);
    
    newTb = gtk_tool_button_new(NULL, "About");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), newTb, -1);
    
    sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1); 
    
    exitTb = gtk_tool_button_new(NULL, "Quit");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exitTb, -1);
    
    gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, FALSE, 5);
    
    g_signal_connect(G_OBJECT(newTb), "clicked", 
                     G_CALLBACK( callback_button_pressed_about ), NULL);
    g_signal_connect(G_OBJECT(exitTb), "clicked", 
                     G_CALLBACK(gtk_main_quit), NULL);
}

/********************************************************/
static GtkWidget *create_frame_with_buttons( gint  horizontal,
                                             char *title,
                                             gint  spacing,
                                             gint  buttons_num,
                                             button_data  *buttonsArray,
                                             gint  layout)
{
    GtkWidget *frame;
    GtkWidget *bbox;
    GtkWidget *button;
    GtkCssProvider *cssBtn;
    int index;
    
    frame = gtk_frame_new (title);
    
    if (horizontal)
        bbox = gtk_hbutton_box_new ();
    else
        bbox = gtk_vbutton_box_new ();
    
    gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
    gtk_container_add (GTK_CONTAINER (frame), bbox);
    
    /* Set the appearance of the Button Box */
    gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), layout);
    gtk_box_set_spacing (GTK_BOX (bbox), spacing);
    
    for (index=0; index<buttons_num; index++)
    {
        button = gtk_button_new_with_label (buttonsArray[index].title);     
        cssBtn = set_css_provider( buttonsArray[index].colour, buttonsArray[index].backcolour);
        if (cssBtn != NULL)
            css_set(cssBtn, button);
        gtk_container_add (GTK_CONTAINER (bbox), button);
        buttonsArray[index].p_button = button;
    }
    
    return frame;
}

/****************************************************************************/
GtkWidget *create_listbox_in_scrollwin( GtkWidget **p_list_box,
                                        gint entriesNum, String32 *names,
                                        GCallback row_selected_cb_function,
                                        void *user_data )
{
    GtkWidget *bbox;
    GtkWidget *row;
    GtkWidget *label;
    GtkWidget *scrollwindow;
    int index;
    
    bbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
    scrollwindow = gtk_scrolled_window_new( NULL, NULL);
    gtk_scrolled_window_set_policy( scrollwindow, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport (scrollwindow, bbox);
    
    //Create a listbox
    *p_list_box = gtk_list_box_new();
    //Set selection mode of the list
    gtk_list_box_set_selection_mode ( (GtkListBox*)*p_list_box, GTK_SELECTION_SINGLE);
    gtk_box_pack_start(GTK_BOX(bbox), *p_list_box, FALSE, FALSE, 10);

    //add rows to the listbox
    if ((names != NULL) && (entriesNum > 0))
    {
        for (index = 0; index < entriesNum; index++)
        {
            row = gtk_list_box_row_new();
            label = gtk_label_new(&(names[index]));
            gtk_label_set_xalign( label, 1.0); // right alignment
            gtk_container_add (GTK_CONTAINER (row), label);
            gtk_container_add (GTK_CONTAINER (*p_list_box), row);
        }
        //select the 1st row
        gtk_list_box_select_row (*p_list_box, gtk_list_box_get_row_at_index( *p_list_box , 0 ));
    } 

    if (row_selected_cb_function != NULL)
        g_signal_connect (*p_list_box, "row-selected", row_selected_cb_function, user_data);
    
    return scrollwindow;
}
/*******************************************************************/
static void set_button_attributes(button_data *btn, gint colour, gint backcolour, gchar *title)
{
    if (btn == NULL) return;
    if (title != NULL)
        sprintf(btn->title, "%s",title);
    else
        btn->title[0] = '\0';
    btn->colour = colour;
    btn->backcolour = backcolour;
}

/************************************************************/
static gint listBoxSortRows (GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer user_data)
{
    char *label1, *label2;
    GtkWidget *child1, *child2;
    int result;
    
    child1 = gtk_bin_get_child(GTK_BIN(row1));
    child2 = gtk_bin_get_child(GTK_BIN(row2));
    if ((GTK_IS_LABEL(child1) != TRUE) || (GTK_IS_LABEL(child2) != TRUE))
        return 0;
    label1 = gtk_label_get_text(child1);
    label2 = gtk_label_get_text(child2);
    result = strcmp(label1, label2);
    return result;
}

/*****************************************************************/
void fill_listbox_with_persons( GtkWidget *listBox )
{
    int index;
    unsigned long persons;
    char *firstname, *surname;
    char familyname[2*sizeof(String32)+2];
    GtkWidget *row, *rowLabel;
    
    persons = DB_get_persons_num();
#ifdef DEBUG
    g_print("persons=%d\n", persons);
#endif
    for (index = 0; index < persons; index++)
    {
        firstname = DB_get_firstname( index );
        surname = DB_get_surname( index );
        if ((firstname != NULL) && (surname != NULL))
        {
            sprintf(familyname, "%s %s", surname, firstname);
            row = gtk_list_box_row_new();
            rowLabel = gtk_label_new( familyname );
            gtk_label_set_xalign( rowLabel, 1.0); // right alignment
            gtk_container_add (GTK_CONTAINER (row), rowLabel);
            gtk_container_add (GTK_CONTAINER (list_box), row);
        }
    }
}

/*******************************************************************/
int main( int argc, char *argv[] )
{
    GtkWidget *main_vbox, *main_hbox, *vbox, *hbox, *image_vbox;
    GtkWidget *frame_horz, *frame_vert, *frame_image_btns2, *frame_image, *frame_image_buttons;
    GtkWidget *image, *icon;
    GdkPixbuf *pixbuf;
    GtkCssProvider *cssBtn;
    button_data buttonsArray[MAX_BUTTONS_IN_FRAME];
    char main_title[64];
    
    /* Initialize GTK */
    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    init_css_table();
    
    sprintf(main_title, "%s: משלוחי מנות טלמון", __FILE__);
    gtk_window_set_title (GTK_WINDOW (window), main_title);
    //wagtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);
    gtk_window_set_resizable (GTK_WINDOW(window), TRUE);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    // Set the inner border or the window
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    
    // Create the main container and add it to the window. This box will add its children widgets horizontally.
    main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // The gtk_container_add function is used to include a single widget in a container
    gtk_container_add (GTK_CONTAINER (window), main_vbox);
    labelMain = gtk_label_new("נתוני התושבים עדיין לא טעונים");
    
    // Add the toolbar to the main box
    add_toolbar_to_box( main_vbox );
    
    // Add the main message label
    frameMainMsg = gtk_frame_new(NULL);
    gtk_container_add (GTK_CONTAINER (frameMainMsg), labelMain);
    gtk_box_pack_start(GTK_BOX(main_vbox), frameMainMsg, FALSE, FALSE, 0);
    cssBtn = set_css_provider( 0xFF0000, 0xE0E0E0);
    if (cssBtn != NULL)
        css_set(cssBtn, labelMain);
    
    // Create the main horizontal container and add it to the main container. This box will add its children widgets horizontally.
    main_hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), main_hbox, TRUE, TRUE, 0);
    
    
    // Create the image for the main window and set its size
    #if 1
    pixbuf = gdk_pixbuf_new_from_inline(-1, purim1_inline, FALSE, NULL);
    image = gtk_image_new_from_pixbuf(pixbuf);
    pixbuf = gdk_pixbuf_new_from_inline(-1, ozen_haman_icon_inline, FALSE, NULL);
    icon = gtk_image_new_from_pixbuf(pixbuf);
    gtk_window_set_icon(window, pixbuf);
    #else  
    image = gtk_image_new_from_file("/home/mwiener/purim/Purim1.jpg");
    #endif
    gtk_image_set_from_pixbuf(
        GTK_IMAGE(image),
        gdk_pixbuf_scale_simple(gtk_image_get_pixbuf(image), 300, 300, GDK_INTERP_BILINEAR));
    
    // The main box will contain 3 frames. The 1st for only buttons put vertically,
    // the 2nd for the listbox, and the 3rd for the image and strip of horizontal buttons beneeth it.
    
    // Create a 1st frame and add it to the main container (pack the container with the frame)
    set_button_attributes( &(buttonsArray[0]) ,0xFFFFFF, 0x0000A0, "הוסף משפחה"); //dark blue
    set_button_attributes( &(buttonsArray[1]) ,0xFFFFFF, 0x0000FF, "הסר משפחה"); //blue
    set_button_attributes( &(buttonsArray[2]) ,0x000000, 0x7070FF, "שנה משפחה"); //light blue
    set_button_attributes( &(buttonsArray[3]) ,0xFFFFFF, 0x38B880, "מספר משלוחים"); //light blue / green
    set_button_attributes( &(buttonsArray[4]) ,0x000000, 0x00FF00, "משלוחים נוספים");//green
    set_button_attributes( &(buttonsArray[5]) ,0x000000, 0xFFFF00, "הוסף קבוצה");//yellow
    set_button_attributes( &(buttonsArray[6]) ,0x000000, 0xFFB000, "מחק קבוצה");//orange
    set_button_attributes( &(buttonsArray[7]) ,0x000000, 0xFFFFFF, "טען קובץ");
    set_button_attributes( &(buttonsArray[8]) ,0x000000, 0xB0B0B0, "אתחל נתונים");
    set_button_attributes( &(buttonsArray[9]) ,0xFFFFFF, 0xFF0000, " שמור נתוני תושבים");//red
    frame_vert = create_frame_with_buttons( FALSE, "", 5, 10, buttonsArray, GTK_BUTTONBOX_SPREAD);
    
    btnAddFamily =  buttonsArray[0].p_button;
    btnDelFamily =  buttonsArray[1].p_button;
    btnChgangeFamily = buttonsArray[2].p_button;
    btnShipmentsNum = buttonsArray[3].p_button;
    btnExtra = buttonsArray[4].p_button;
    btnAddGroup = buttonsArray[5].p_button;
    btnDelGroup = buttonsArray[6].p_button;
    btnLoadDbFile = buttonsArray[7].p_button;
    btnNewDB = buttonsArray[8].p_button;
    btnSaveDbFile = buttonsArray[9].p_button;
 
    // Create a frame for the calculation buttons and add it to the main container (pack the container with the frame)
    set_button_attributes( &(buttonsArray[0]) ,0x000000, 0x7070FF, "חשב משלוחים"); // light blue
    set_button_attributes( &(buttonsArray[1]) ,0xFFFFFF, 0x4040FF, "שמור משלוחים"); 
    set_button_attributes( &(buttonsArray[2]) ,0xFFFFFF, 0x600060, "טען משלוחים");
    set_button_attributes( &(buttonsArray[3]) ,0xFFFFFF, 0x008080, "שינוי ידני");
    frame_image_buttons = create_frame_with_buttons( TRUE, "Shipments Buttons Frame", 5, 4, buttonsArray, GTK_BUTTONBOX_SPREAD);
    
    btnCalculate =  buttonsArray[0].p_button;
    btnSaveCalc =  buttonsArray[1].p_button;
    btnLoadCalc = buttonsArray[2].p_button;
    btnManual = buttonsArray[3].p_button;
    
    // Create a frame for the calculation buttons and add it to the main container (pack the container with the frame)
    set_button_attributes( &(buttonsArray[0]) ,0xFFFF00, 0x000000, "DEBUG"); // black
    set_button_attributes( &(buttonsArray[1]) ,0x0000A0, 0x40C0FF, "הפקת פתקים");
    set_button_attributes( &(buttonsArray[2]) ,0x000070, 0x60E0FF, "הפקת פתק אחד");
    set_button_attributes( &(buttonsArray[3]) ,0x000000, 0xFF3030, "ביטול");
    frame_image_btns2 = create_frame_with_buttons( TRUE, "לחיצי פתקים", 5, 4, buttonsArray, GTK_BUTTONBOX_SPREAD);
    
    btnDebug =  buttonsArray[0].p_button;
    btnNotes =  buttonsArray[1].p_button;
    btnNote  =  buttonsArray[2].p_button;
    btnCancel  =  buttonsArray[3].p_button;
    
    // Set button callback functions
    g_signal_connect (btnNewDB, "button-press-event", G_CALLBACK (newDB_button_pressed_callback), NULL);
    g_signal_connect (btnLoadDbFile, "button-press-event", G_CALLBACK (loadDB_button_pressed_callback), NULL);
    g_signal_connect (btnAddFamily, "button-press-event", G_CALLBACK (add_family_button_pressed_callback), NULL);
    g_signal_connect (btnChgangeFamily, "button-press-event", G_CALLBACK (chg_family_button_pressed_callback), NULL);
    g_signal_connect (btnDelFamily, "button-press-event", G_CALLBACK (del_family_button_pressed_callback), NULL);
    g_signal_connect (btnAddGroup, "button-press-event", G_CALLBACK (add_group_button_pressed_callback), NULL);
    g_signal_connect (btnDelGroup, "button-press-event", G_CALLBACK (del_group_button_pressed_callback), NULL);
    g_signal_connect (btnExtra, "button-press-event", G_CALLBACK (extra_button_pressed_callback), NULL);
    g_signal_connect (btnShipmentsNum, "button-press-event", G_CALLBACK (shipmentsnum_button_pressed_callback), NULL);
    g_signal_connect (btnSaveDbFile, "button-press-event", G_CALLBACK (saveDB_button_pressed_callback), NULL);
    g_signal_connect (btnCalculate, "button-press-event", G_CALLBACK (calculate_button_pressed_callback), NULL);
    g_signal_connect (btnSaveCalc, "button-press-event", G_CALLBACK (save_shipments_button_pressed_callback), NULL);
    g_signal_connect (btnLoadCalc, "button-press-event", G_CALLBACK (load_shipments_button_pressed_callback), NULL);
    g_signal_connect (btnManual, "button-press-event", G_CALLBACK (manual_chg_button_pressed_callback), NULL);
    g_signal_connect (btnNotes, "button-press-event", G_CALLBACK (make_notes_button_pressed_callback), NULL);
    g_signal_connect (btnNote, "button-press-event", G_CALLBACK (make_note_button_pressed_callback), NULL);
    g_signal_connect (btnCancel, "button-press-event", G_CALLBACK (cancel_button_pressed_callback), NULL);
    g_signal_connect (btnDebug, "button-press-event", G_CALLBACK (help_button_pressed_callback), NULL);
    // Expand argument is true, so fill in all the extra space in the box, and the
    //fill argument is also true, so he extra space is allocated to the objects themselves 
    gtk_box_pack_start (GTK_BOX (main_hbox), frame_vert, TRUE, TRUE, 10);
    
    // Create the 2nd frame and add it to the main container
    scrollwin = create_listbox_in_scrollwin( &list_box, 0, NULL, (GCallback) row_selected_callback, NULL );
    gtk_list_box_set_sort_func( list_box, listBoxSortRows, NULL, NULL);
    frame_listbox = gtk_frame_new("רשימת משפחות");
    gtk_frame_set_label_align( frame_listbox, 0.5, 1.0);
    gtk_box_pack_start (GTK_BOX (main_hbox), frame_listbox, TRUE, TRUE, 5); 
    gtk_container_add (GTK_CONTAINER (frame_listbox), scrollwin);
    
    
    // Create the 3rd frame and add it to the main container
    frame_image = gtk_frame_new ("");
    gtk_box_pack_start (GTK_BOX (main_hbox), frame_image, TRUE, TRUE, 10);
    image_vbox =gtk_vbox_new (FALSE,0);
    gtk_container_add (GTK_CONTAINER (frame_image), image_vbox);
    gtk_container_add (GTK_CONTAINER (image_vbox), image);
    gtk_container_add (GTK_CONTAINER (image_vbox), frame_image_buttons);
    gtk_container_add (GTK_CONTAINER (image_vbox), frame_image_btns2);
    
    gtk_widget_show_all (window);
    go_state0();
    
    /* Enter the event loop */
    gtk_main ();
    
    return 0;
}

