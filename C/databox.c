#include <purim_api.h>

GtkWidget *Databox_window = NULL;
/* buttons */
GtkWidget *btn_Databox_add_family, *btn_Databox_del_family, *btn_Databox_chg_family, *btn_Databox_quit;
GtkWidget *btn_Databox_save_shipments_num, *btn_Databox_add_group, *btn_Databox_del_group;
/* other widgets */
GtkWidget *entry_Databox_1, *entry_Databox_2, *chkbtn_Databox;
GtkListBox *listbox_Databox_1, *listbox_Databox_2;
GtkWidget *label_Databox_Main, *label_Databox_box1, *label_Databox_box2, *label_Databox_1, *label_Databox_2;
GtkWidget *scale_shipments;

static databoxreq request;
static void callback_destroy_row (GtkWidget *row, gpointer data);
static gint listBoxSortRows (GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer user_data);
static void row_selected_callback (GtkListBox *box, GtkListBoxRow *row, gpointer user_data);

/*** CALLBACK functions ***/
void  callback_databox_window_destroy (GtkWidget *widget, gint wiunId)
{
    gchar *title;
    
    g_print("Databox window destroy: Toggle to main window...\n");
    title = gtk_window_get_title( Databox_window );
    gtk_widget_show_all( window );
    /* recreate the Data Box window */
    create_databox_window( title );
    gtk_widget_hide ( Databox_window );
    gtk_entry_set_text( entry_Databox_1, "" );
    gtk_entry_set_text( entry_Databox_2, "" );
    go_state1();
}
/***********************************************************/

void  callback_databox_quit_button_clicked (GtkWidget *widget, gint windowId)
{
    gchar *title;
    
    g_print("Databox Quit button clicked: Toggle to main window...\n");
    gtk_widget_show_all( window );
    gtk_widget_hide ( Databox_window );
    gtk_entry_set_text( entry_Databox_1, "" );
    gtk_entry_set_text( entry_Databox_2, "" );
    go_state1();
}

/***********************************************************/
void callback_databox_add_family_button_clicked (GtkWidget *widget, gint windowId)
{
    long recordNum;
    gboolean nonsender;
    char *firstname, *surname;
    char familyname[2*sizeof(String32)+2];
    int groupnum;
    GtkWidget *row, *rowLabel;
    char errMsg[64];
    
    /* Button of second window was clicked. Hide second and show main window */
    
    firstname = gtk_entry_get_text( entry_Databox_2 );
    surname = gtk_entry_get_text( entry_Databox_1 );
    if ((strlen(firstname) > 0) && (strlen(surname) > 0))
    {
        recordNum = DB_find_family( firstname, surname );
        nonsender = gtk_toggle_button_get_active (chkbtn_Databox);
        groupnum = gtk_list_box_row_get_index(gtk_list_box_get_selected_row((GtkListBox*)listbox_Databox_2));
        if ((recordNum == (-1)) && (groupnum >= 0))
        {
            DB_add_family( firstname, surname, nonsender, groupnum);

            // add row to list_box
            if ((firstname != NULL) && (surname != NULL))
            {
                sprintf(familyname, "%s %s", surname, firstname);
                row = gtk_list_box_row_new();
                rowLabel = gtk_label_new( familyname );
                gtk_label_set_xalign( rowLabel, 1.0); // right alignment
                gtk_container_add (GTK_CONTAINER (row), rowLabel);
                gtk_container_add (GTK_CONTAINER (list_box), row);
                g_print("Added %s to listbox\n", familyname);
            }
        }
        else
        {
            printf( errMsg, "Found family at index %d\n", recordNum );
            g_print( errMsg );
            msgBoxError( Databox_window, errMsg );
        }
        g_print ("Databox Add Family button clicked: Toggle to main window...\n");
        g_print("Selected row [%d]\n", groupnum );
        if (nonsender==TRUE)
            g_print("Family is free\n");
        else
            g_print("Normal family\n");
    }
    gtk_widget_hide ( Databox_window );
    gtk_widget_show_all( window );
    go_state1();
}

/***********************************************************/
void callback_databox_chg_family_button_clicked (GtkWidget *widget, gint windowId)
{
    /* Change Family Button of second window was clicked. Hide second and show main window */
    unsigned long personNum;
    int groupNum;
    gboolean isFree;
    GtkWidget *row;
    
    personNum = gtk_list_box_row_get_index( gtk_list_box_get_selected_row((GtkListBox*)listbox_Databox_2) );
    groupNum = gtk_list_box_row_get_index( gtk_list_box_get_selected_row((GtkListBox*)listbox_Databox_1) );
    isFree = gtk_toggle_button_get_active (chkbtn_Databox);
    g_print("Changing family #%d\n", personNum);
    DB_set_person_groupnumber( personNum, groupNum );
    DB_set_free( personNum, isFree );
    gtk_widget_hide ( Databox_window );
    gtk_widget_show_all( window );    
    go_state1();
}

/***********************************************************/
void callback_databox_del_family_button_clicked (GtkWidget *widget, gint windowId)
{
    /* Delete Family Button of second window was clicked. Hide second and show main window */
    unsigned long personNum;
    GtkWidget *row;
    
    personNum = gtk_list_box_row_get_index( gtk_list_box_get_selected_row((GtkListBox*)listbox_Databox_2) );
    g_print("Deleting family #%d\n", personNum);
    DB_del_family( personNum );
    gtk_widget_hide ( Databox_window );
    gtk_widget_show_all( window );
    
    // remove row from list_box
    row = gtk_list_box_get_row_at_index( list_box, personNum );
    if (row != NULL)
        gtk_widget_destroy(row);
    
    go_state1();
}

/***********************************************************/
void callback_databox_add_group_button_clicked (GtkWidget *widget, gint windowId)
{
    int groupsNum;
    char *groupname;
    int groupnum;
    char errMsg[64];
    
    /* Button of second window was clicked. Hide second and show main window */
    
    groupsNum = DB_get_groups_number();
    if (groupsNum >= MAX_GROUPS_NUM)
    {
        msgBoxError( Databox_window, "רשימת הקבוצות מלאה. לא ניתן להוסיף קבוצה נוספת." );
        goto TAIL;
    }
    
    groupname = gtk_entry_get_text( entry_Databox_1 );
    for (groupnum = 0; groupnum < groupsNum; groupnum++)
    {
        if (strcmp( DB_get_groupname( groupnum ), groupname ) == 0)
        {
            msgBoxError( Databox_window, "קבוצה בשם זה כבר קיימת" );
            return;
        }
    }
    
    DB_add_group( groupname );

TAIL:
    gtk_widget_hide ( Databox_window );
    gtk_widget_show_all( window );
    go_state1();
}

/***********************************************************/
void callback_databox_del_group_button_clicked (GtkWidget *widget, gint windowId)
{
    int groupnum, newGroupNum, groupsNum;
    char errMsg[64];
    
    groupsNum = DB_get_groups_number();
    if (groupsNum < 2)
    {
        msgBoxError( Databox_window, "לא ניתן למחוק את הקבוצה היחידה." );
        goto TAIL;
    }
    groupnum = gtk_list_box_row_get_index( gtk_list_box_get_selected_row((GtkListBox*)listbox_Databox_2) );
    newGroupNum = gtk_list_box_row_get_index( gtk_list_box_get_selected_row((GtkListBox*)listbox_Databox_1) );
    if (groupsNum == newGroupNum)
    {
        msgBoxError( Databox_window, "לא ניתן להעביר תושבים לקבוצה העומדת להמחק" );
        goto TAIL;
    }
    
    DB_del_group( groupnum, newGroupNum );
TAIL:
    gtk_widget_hide ( Databox_window );
    gtk_widget_show_all( window );
    go_state1();
}

/********************************************************/
void callback_databox_shipments_num_button_clicked(GtkWidget *widget, gint windowId)
{
    DB_set_shipments_num( gtk_range_get_value( scale_shipments ));
    gtk_widget_hide ( Databox_window );
    gtk_widget_show_all( window );
    go_state1();
}
/********************************************************/
static void callback_destroy_row (GtkWidget *row, gpointer data)
{
    gtk_widget_destroy(row);
}

/**********************************************************/
static void row_selected_callback (GtkListBox *box, GtkListBoxRow *row, gpointer user_data)
{
    gboolean isFree;
    int groupNum;
    unsigned long personNum;
    
    personNum = gtk_list_box_row_get_index(row);
    groupNum = groupNum = DB_get_person_groupnumber( personNum );
    isFree = DB_is_free( personNum );
    g_print("Group number of person %d is %d\n", personNum, groupNum);
    /* set the group of 1st person in the groups listbox */
    gtk_list_box_select_row (listbox_Databox_1, gtk_list_box_get_row_at_index( listbox_Databox_1 , groupNum ));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbtn_Databox), isFree);    
}

/**********************************************************/
void remove_all_rows_of_listbox( GtkWidget *listbox )
{
    if (GTK_IS_CONTAINER( listbox ) != TRUE) return;
    gtk_container_foreach( (GtkContainer *)listbox, callback_destroy_row, NULL);
}

/*****************************************/
void databox_request_service( databoxreq req )
{
    request = req;
    char *title = "עדכון נתונים";
    GtkWidget *row, *label;
    String32 names[MAX_GROUPS_NUM];
    int index, groups, groupNum, shipments;
    unsigned long persons;
    char *firstname, *surname;
    gboolean isFree;
    char familyname[96];
    
    if (Databox_window == NULL) create_databox_window( title );
    remove_all_rows_of_listbox( listbox_Databox_1 );
    remove_all_rows_of_listbox( listbox_Databox_2 );
    if (request == DATABOX_REQ_ADDFAMILY)
    {
        /* add group names to listbox */
        groups = DB_get_groups_number();
        for (index = 0; index < groups; index++)
        {
            sprintf(names[index], "%s", DB_get_groupname(index));
            row = gtk_list_box_row_new();
            label = gtk_label_new(&(names[index]));
            gtk_container_add (GTK_CONTAINER (row), label);
            gtk_container_add (GTK_CONTAINER (listbox_Databox_2), row);
        }
        //select the 1st row
        gtk_list_box_select_row (listbox_Databox_2, gtk_list_box_get_row_at_index( listbox_Databox_2 , 0 ));
    }
    else if (request == DATABOX_REQ_CHGFAMILY)
    {
        /* add group names to listbox */
        groups = DB_get_groups_number();
        for (index = 0; index < groups; index++)
        {
            sprintf(names[index], "%s", DB_get_groupname(index));
            row = gtk_list_box_row_new();
            label = gtk_label_new(&(names[index]));
            gtk_label_set_xalign( label, 1.0); // right alignment
            gtk_container_add (GTK_CONTAINER (row), label);
            gtk_container_add (GTK_CONTAINER (listbox_Databox_1), row);
        }
        /* add families to listbox */
        persons = DB_get_persons_num();
        for (index = 0; index < persons; index++)
        {
            firstname = DB_get_firstname( index );
            surname = DB_get_surname( index );
            if ((firstname != NULL) && (surname != NULL))
            {
                sprintf(familyname, "%s %s", surname, firstname);
                row = gtk_list_box_row_new();
                label = gtk_label_new( familyname );
                gtk_label_set_xalign( label, 1.0); // right alignment
                gtk_container_add (GTK_CONTAINER (row), label);
                gtk_container_add (GTK_CONTAINER (listbox_Databox_2), row);
            }
        }
        //select the 1st row in familis listbox
        gtk_list_box_select_row (listbox_Databox_2, gtk_list_box_get_row_at_index( listbox_Databox_2 , 0 ));
        groupNum = DB_get_person_groupnumber( 0 );
        isFree = DB_is_free( 0 );
        g_print("Group number of person 0 is %d\n", groupNum);
        // set the group of 1st person in the groups listbox */
        gtk_list_box_select_row (listbox_Databox_1, gtk_list_box_get_row_at_index( listbox_Databox_1 , groupNum ));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbtn_Databox), isFree);
    }
    else if (request == DATABOX_REQ_DELFAMILY)
    {
        persons = DB_get_persons_num();
        for (index = 0; index < persons; index++)
        {
            firstname = DB_get_firstname( index );
            surname = DB_get_surname( index );
            if ((firstname != NULL) && (surname != NULL))
            {
                sprintf(familyname, "%s %s", surname, firstname);
                row = gtk_list_box_row_new();
                label = gtk_label_new( familyname );
                gtk_container_add (GTK_CONTAINER (row), label);
                gtk_container_add (GTK_CONTAINER (listbox_Databox_2), row);
            }
        }
        //select the 1st row
        gtk_list_box_select_row (listbox_Databox_2, gtk_list_box_get_row_at_index( listbox_Databox_2 , 0 ));
    }
    else if (request == DATABOX_REQ_ADDGROUP)
    {
        /* add group names to listbox */
        groups = DB_get_groups_number();
        for (index = 0; index < groups; index++)
        {
            sprintf(names[index], "%s", DB_get_groupname(index));
            row = gtk_list_box_row_new();
            label = gtk_label_new(&(names[index]));
            gtk_container_add (GTK_CONTAINER (row), label);
            gtk_container_add (GTK_CONTAINER (listbox_Databox_2), row);
        }
        //select the 1st row
        gtk_list_box_select_row (listbox_Databox_2, gtk_list_box_get_row_at_index( listbox_Databox_2 , 0 ));
    }
    else if (request == DATABOX_REQ_DELGROUP)
    {
        /* fill both listboxes */
        groups = DB_get_groups_number();
        for (index = 0; index < groups; index++)
        {
            sprintf(names[index], "%s", DB_get_groupname(index));
            row = gtk_list_box_row_new();
            label = gtk_label_new(&(names[index]));
            gtk_container_add (GTK_CONTAINER (row), label);
            gtk_container_add (GTK_CONTAINER (listbox_Databox_1), row);
            row = gtk_list_box_row_new();
            label = gtk_label_new(&(names[index]));
            gtk_container_add (GTK_CONTAINER (row), label);
            gtk_container_add (GTK_CONTAINER (listbox_Databox_2), row);
        }
        //select the 1st row
        gtk_list_box_select_row (listbox_Databox_2, gtk_list_box_get_row_at_index( listbox_Databox_2 , 0 ));
    }
    else if (request == DATABOX_REQ_SHIPMENTSNUM)
    {
        shipments = DB_get_shipments_num();
        gtk_range_set_value ( scale_shipments, shipments);
    }
}

/***************************************/
gboolean create_databox_window( char *title )
{
        GtkWidget *scrollwin, *main_vbox, *hbox1, *hbox2, *hbox3;
        GtkWidget *vbox2_1, *vbox2_2, *vbox2_3;
        GtkWidget *hbox2_3_1, *hbox2_3_2;
        GtkCssProvider *cssBtn;
        GdkPixbuf *pixbuf;
 
        g_print("create_databox_window\n");
        
        // Create the widgets
        Databox_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

        btn_Databox_add_family  = gtk_button_new_with_label ("הוסף משפחה");
        cssBtn = set_css_provider( 0x000000, 0x8080FF);
        css_set(cssBtn, btn_Databox_add_family);
        
        btn_Databox_chg_family = gtk_button_new_with_label ("שינוי משפחה");
        cssBtn = set_css_provider( 0x000000, 0x80A0E0);
        css_set(cssBtn, btn_Databox_chg_family);
        
        btn_Databox_del_family  = gtk_button_new_with_label ("הסר משפחה");
        cssBtn = set_css_provider( 0x000000, 0xFF8080);
        css_set(cssBtn, btn_Databox_del_family);
        
        btn_Databox_add_group  = gtk_button_new_with_label ("הוסף קבוצה");
        cssBtn = set_css_provider( 0x000000, 0x8080FF);
        css_set(cssBtn, btn_Databox_add_group);
        
        btn_Databox_del_group  = gtk_button_new_with_label ("הסר קבוצה");
        cssBtn = set_css_provider( 0x000000, 0xFF8080);
        css_set(cssBtn, btn_Databox_del_group);
        
        btn_Databox_quit = gtk_button_new_with_label("יציאה");
        cssBtn = set_css_provider( 0x000000, 0xFF0000);
        css_set(cssBtn, btn_Databox_quit);
        
        btn_Databox_save_shipments_num = gtk_button_new_with_label("שמור מספר משלוחים");
        cssBtn = set_css_provider( 0x000000, 0x009F00);
        css_set(cssBtn, btn_Databox_save_shipments_num);
        
        label_Databox_Main = gtk_label_new("הוספת משפחה");
        cssBtn = set_css_provider( 0xFF0000, 0xE0E0E0);
        if (cssBtn != NULL)
            css_set(cssBtn, label_Databox_Main);
        
        entry_Databox_1 = gtk_entry_new();
        gtk_entry_set_text( entry_Databox_1, "" );
        label_Databox_1 = gtk_label_new("");
        
        entry_Databox_2 = gtk_entry_new();
        gtk_entry_set_text( entry_Databox_2, "" );
        label_Databox_2 = gtk_label_new("");
        
        chkbtn_Databox = gtk_check_button_new_with_label("פטור ממשלוח");
        label_Databox_box1 = gtk_label_new("BOoo..");
        label_Databox_box2 = gtk_label_new("");
        
        scale_shipments = gtk_scale_new_with_range( GTK_ORIENTATION_HORIZONTAL, 1, MAX_SHIPMENTS, 1);
        gtk_scale_set_digits( scale_shipments, 0 );
        gtk_scale_set_draw_value( scale_shipments, TRUE );
        gtk_widget_set_size_request( scale_shipments, 200, 40);
        
        // set window icon
        pixbuf = gdk_pixbuf_new_from_inline(-1, ozen_haman_icon_inline, FALSE, NULL);
        gtk_window_set_icon(Databox_window, pixbuf);
        gtk_window_set_title (GTK_WINDOW (Databox_window), title);

        // Create the widgets boxes
        // Create the main container and add it to the window. This box will add its children boxes one under the other.
        main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        hbox3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        vbox2_1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        vbox2_2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        vbox2_3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        hbox2_3_1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        hbox2_3_2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        
        /* setup the windows and pack boxes into it */        
        gtk_container_set_border_width (GTK_CONTAINER (Databox_window), 25);
        // The gtk_container_add function is used to include a single widget in a container
        gtk_container_add (GTK_CONTAINER (Databox_window), main_vbox);
        gtk_box_pack_start(GTK_BOX(main_vbox), hbox1, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(main_vbox), hbox2, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(main_vbox), hbox3, FALSE, FALSE, 5);
        
        gtk_box_pack_start(GTK_BOX(hbox1), label_Databox_Main, TRUE, FALSE, 5);
        
        gtk_box_pack_start(GTK_BOX(hbox2), vbox2_1, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox2), vbox2_2, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox2), vbox2_3, FALSE, FALSE, 5);

        gtk_box_pack_start(GTK_BOX(vbox2_1), label_Databox_box1, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(vbox2_2), label_Databox_box2, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(vbox2_2), scale_shipments, FALSE, FALSE, 5);
        
        gtk_box_pack_start(GTK_BOX(vbox2_3), hbox2_3_1, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(vbox2_3), hbox2_3_2, FALSE, FALSE, 5);
        
        gtk_box_pack_start(GTK_BOX(hbox2_3_1), entry_Databox_1, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox2_3_1), label_Databox_1, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox2_3_2), entry_Databox_2, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox2_3_2), label_Databox_2, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(vbox2_3), chkbtn_Databox, FALSE, FALSE, 5);
       
        gtk_box_pack_start(GTK_BOX(hbox3), btn_Databox_add_family, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox3), btn_Databox_chg_family, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox3), btn_Databox_del_family, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox3), btn_Databox_add_group, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox3), btn_Databox_del_group, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox3), btn_Databox_save_shipments_num, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox3), btn_Databox_quit, TRUE, FALSE, 5);
 
        /* create a listbox with no rows */
        scrollwin = create_listbox_in_scrollwin( &listbox_Databox_1, 0, NULL, NULL );
        gtk_widget_set_size_request( scrollwin, 50, 200 );
        gtk_box_pack_start(GTK_BOX(vbox2_1), scrollwin, TRUE, TRUE, 5);
        
        /* create a listbox with no rows */
        scrollwin = create_listbox_in_scrollwin( &listbox_Databox_2, 0, NULL, (GCallback) row_selected_callback );
        gtk_widget_set_size_request( scrollwin, 50, 200 );
        gtk_box_pack_start(GTK_BOX(vbox2_2), scrollwin, TRUE, TRUE, 5);

        /* connect callbacks to signals */        
        g_signal_connect (G_OBJECT (Databox_window), "destroy", 
                          G_CALLBACK (callback_databox_window_destroy), 2);
        g_signal_connect (G_OBJECT (btn_Databox_quit), "clicked", 
                          G_CALLBACK (callback_databox_quit_button_clicked), 2);
        g_signal_connect (G_OBJECT (btn_Databox_add_family), "clicked", 
                          G_CALLBACK (callback_databox_add_family_button_clicked), 2);
        g_signal_connect (G_OBJECT (btn_Databox_chg_family), "clicked", 
                          G_CALLBACK (callback_databox_chg_family_button_clicked), 2);
        g_signal_connect (G_OBJECT (btn_Databox_del_family), "clicked", 
                          G_CALLBACK (callback_databox_del_family_button_clicked), 2);
        g_signal_connect (G_OBJECT (btn_Databox_add_group), "clicked", 
                          G_CALLBACK (callback_databox_add_group_button_clicked), 2);
        g_signal_connect (G_OBJECT (btn_Databox_del_group), "clicked", 
                          G_CALLBACK (callback_databox_del_group_button_clicked), 2);
        g_signal_connect (G_OBJECT (btn_Databox_save_shipments_num), "clicked", 
                          G_CALLBACK (callback_databox_shipments_num_button_clicked), 2);

        //gtk_widget_show_all( Databox_window );
    return TRUE;
}
