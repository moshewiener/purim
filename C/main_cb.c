#include "main_cb.h"

#ifdef PROJ_VERSION
char *Version = PROJ_VERSION;
#else
char *Version = "0.0";
#endif

static long user_data;
static time_t sec_start, sec_end;
static long ms_start, ms_end;
static gint timer_tag;
/*********** CALLBACK functions ********************************/

gboolean callback_button_pressed_about(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    char msg[128];
    
    g_print("About button pressed\n");
    sprintf(msg,"%s\n%s : %s\n%s %s","פרויקט משלוחי מנות טלמון", " גירסא ", Version, "תאריך בניה ", __DATE__);
    show_about_window(window, msg);
#ifdef DEBUG
    g_print ("back from About dialog\n");
#endif
    return TRUE;
}

/*---------------------------------------------------------*/
void row_selected_callback (GtkListBox *box, GtkListBoxRow *row, gpointer user_data)
{
    GtkWidget *child;
    char *labeltext = NULL;
    
    child = gtk_bin_get_child(GTK_BIN(row));
    if (GTK_IS_LABEL(child) != TRUE)
        g_print("%s: ERROR!\n", __FUNCTION__);
    else
    {
        labeltext = gtk_label_get_text((GtkLabel*)child);
#ifdef DEBUG        
        printf("Row selected %d: Text=%s\n", gtk_list_box_row_get_index(row), labeltext);
        g_print("box %p list_box %p...\n", (void*)box, (void*)list_box);
#endif
    }
    
}

/*---------------------------------------------------------*/
gint CloseAppWindow_callback (GtkWidget *widget, gpointer *data)
{
    DB_close_purim_db();
    gtk_main_quit ();
    
    return (FALSE);
}

/*---------------------------------------------------------*/
gboolean newDB_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    DB_init_purim_db( NULL );
    fill_listbox_with_persons( list_box );
    go_main_state();
    return TRUE;
}

/*---------------------------------------------------------*/
/* DEBUG function */
gboolean help_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    char *firstname, *surname, *groupname;
    long family;
    GtkResponseType answer;
    char *responseName;
#if 0    
    answer =  msgBoxYesNo ( window, "Did you have breakfast?" );
    if (answer == GTK_RESPONSE_NO) 
    {
        msgBoxError ( window, "How sad..." );
        responseName = "NO";
    }
    else if (answer == GTK_RESPONSE_YES)
    {
        msgBoxSuccess ( window , "That is wonderful!");
        responseName = "YES";
    }
    else if (answer == GTK_RESPONSE_DELETE_EVENT) responseName = "DELETE_EVENT";
    else responseName = "??";
    g_print("Response = %s %d\n", responseName, answer);
#endif
    
    DB_debug_print_groups();
    DB_debug_print_all_records();
    printf("Number of persons %lu. Number of givers %d. Number of shipments: %d\n",
           DB_get_persons_num(), DB_get_givers_num(), DB_get_shipments_num());

    CALC_debug_print_shipments();

    return TRUE;
}

/*---------------------------------------------------------*/
gboolean add_family_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state2();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean del_family_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state3();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean chg_family_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state7();
    return TRUE;
}


/*---------------------------------------------------------*/
gboolean add_group_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state4();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean del_group_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state5();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean extra_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state8();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean shipmentsnum_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state6();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean calculate_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    gboolean result;
    
#ifdef DEBUG
    g_print("Calling calculate_shipments...\n");
#endif
    result = CALC_calculate_shipments();
    go_main_state();
    return result;
}

/*-----------------------------------------------------------------------------*/
gboolean saveDB_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    GtkFileFilter *filter;
    gint res;
    
    dialog = gtk_file_chooser_dialog_new ("Save DB File",
                                          window,
                                          action,
                                          "_ביטול",
                                          GTK_RESPONSE_CANCEL,
                                          "_שמור",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.csv");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        char command[64];
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        gtk_file_chooser_set_create_folders( chooser, FALSE );
        filename = gtk_file_chooser_get_filename (chooser);
        g_print("Saving DB in file: %s\n", filename);
        DB_save_purim_db( filename );
        g_free( filename );
    }
    
    gtk_widget_destroy (dialog);
    go_main_state();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean loadDB_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    char *filename = NULL;
    gboolean result;
    
    filename = msgBoxOpenfile( "*.csv" , "בחר קובץ נתוני תושבים");
    result = DB_init_purim_db( (filename==NULL)? "../data/smalldb.csv" : filename );
    if (filename != NULL) free(filename);
    if (result == FALSE)
    {
        msgBoxError( window, "טעינת קובץ הנתונים נכשלה");
    }
    else
    {
        fill_listbox_with_persons( list_box );
        go_main_state();
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------*/
gboolean save_shipments_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    GtkFileFilter *filter;
    gint res;
    gboolean result;
    char *errmsg = NULL;
    
    dialog = gtk_file_chooser_dialog_new ("Save Shipments File",
                                          window,
                                          action, "_ביטול",
                                          GTK_RESPONSE_CANCEL, "_שמור",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.csv");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        char command[64];
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        gtk_file_chooser_set_create_folders( chooser, FALSE );
        filename = gtk_file_chooser_get_filename (chooser);
        g_print("Saving Shipments in file: %s\n", filename);
        result = CALC_save_shipments( filename, &errmsg );
        g_free( filename );
    }
    
    gtk_widget_destroy (dialog);
    if (result == TRUE)
        msgBoxSuccess( window, "רשימות המשלוחים נשמרו בהצלחה");
    else
        msgBoxError( window, (errmsg == NULL)? "נכשל נסיון לשמור משלוחים" : errmsg );
    if (errmsg != NULL) free( errmsg );
    go_main_state();
    return TRUE;
}

/*-----------------------------------------------------------------------------*/
gboolean load_shipments_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    GtkFileFilter *filter;
    gint res;
    gboolean result;
    char *errmsg = NULL;
    
    dialog = gtk_file_chooser_dialog_new ("טען קובץ פתקים",
                                          window,
                                          action, "_ביטול",
                                          GTK_RESPONSE_CANCEL, "_שמור",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.csv");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        char command[64];
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        gtk_file_chooser_set_create_folders( chooser, FALSE );
        filename = gtk_file_chooser_get_filename (chooser);
        g_print("Loading Shipments from file: %s\n", filename);
        result = CALC_load_shipments( filename, &errmsg );
        g_free( filename );
    }
    
    gtk_widget_destroy (dialog);
    if (result == TRUE)
        msgBoxSuccess( window, "רשימות המשלוחים נטענו בהצלחה");
    else
        msgBoxError( window, (errmsg == NULL)? "נכשל נסיון לטעון משלוחים" : errmsg );
    if (errmsg != NULL) free( errmsg );
    go_main_state();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean manual_chg_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    go_state9();
    return TRUE;
}

/*---------------------------------------------------------*/
gboolean NEW_make_notes_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    long giversnum, receiversnum, giverIndex, receiver, personNum;
    char *command = NULL;
    char *destFilename = NULL;
    char *templateFilename = NULL;
    int shipments, shipmentIndex;
    char receiverName[2*sizeof(String32) + 4];
    char title[64];
    struct timespec spec;
    
    
    clock_gettime(CLOCK_REALTIME, &spec);
    sec_start = spec.tv_sec;
    ms_start = round(spec.tv_nsec / 1.0e6);
    if (ms_start > 999) { sec_start++; ms_start = 0; }
       
    if (CALC_is_data_loaded() == FALSE)
        goto NO_SHIPMENTS_DATA;
    giversnum = CALC_get_givers_num();
    receiversnum = CALC_get_receivers_num();
    if ((giversnum < 1) || (receiversnum < 1))
        goto NO_SHIPMENTS_DATA;
    
    if ((templateFilename = msgBoxOpenfile( "*.odt" , "בחר קובץ תבנית")) != NULL)
        destFilename = msgBoxSavefile( "*.odt" , "בחר קובץ לשמירת הפתקים");
    if ((templateFilename == NULL) || (destFilename == NULL))
        goto FINISH;
    // Open communication channel with Libreoffice
    if (COMM_build_comm_libreoffice() == FALSE)
    {
        msgBoxError( window, "נכשל נסיון הקמת תקשורת עם השרת");
        goto FINISH;
    }
    // Tell Libreoffice server the template file name
    command = malloc(strlen(templateFilename) + 1);
    strcpy(command, templateFilename);
    COMM_send_command( COMM_CMD_TEMPLATE_FILE, command );
    // Tell Libreoffice server the notes file name
    if (strlen(destFilename) > strlen(templateFilename))
    {
        free(command);
        command = malloc(strlen(destFilename) + 1);
    }
    strcpy(command, destFilename);
    COMM_send_command( COMM_CMD_NOTES_FILE, command );
    // make the initializing call to the timer callback
    user_data = 0;
    timeout_make_notes_callback(&user_data);
    user_data = 1;
    // send all shipments notes, each one every second
    timer_tag = g_timeout_add (3000, timeout_make_notes_callback, &user_data);
    
FINISH:
    if (command != NULL) free(command);
    if (templateFilename != NULL) free(templateFilename);
    if (destFilename != NULL) free(destFilename);
    return TRUE;
    
NO_SHIPMENTS_DATA:
    msgBoxError(window, "אין נתוני משלוחים");
    return FALSE;
}

/*---------------------------------------------------------*/
gboolean make_notes_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    long giversnum, receiversnum, giverIndex, receiver, personNum;
    char *command = NULL;
    char *destFilename = NULL;
    char *templateFilename = NULL;
    int shipments, shipmentIndex;
    char receiverName[2*sizeof(String32) + 4];
    char title[128];
    struct timespec spec;
    int minutes, seconds;
    
    if (CALC_is_data_loaded() == FALSE)
        goto NO_SHIPMENTS_DATA;
    giversnum = CALC_get_givers_num();
    receiversnum = CALC_get_receivers_num();
    if ((giversnum < 1) || (receiversnum < 1))
        goto NO_SHIPMENTS_DATA;
    
    if ((templateFilename = msgBoxOpenfile( "*.odt" , "בחר קובץ תבנית")) != NULL)
        destFilename = msgBoxSavefile( "*.odt" , "בחר קובץ לשמירת הפתקים");
    if ((templateFilename == NULL) || (destFilename == NULL))
        goto FINISH;
    clock_gettime(CLOCK_REALTIME, &spec);
    sec_start = spec.tv_sec;
    ms_start = round(spec.tv_nsec / 1.0e6);
    if (ms_start > 999) { sec_start++; ms_start = 0; }
    // Open communication channel with Libreoffice
    if (COMM_build_comm_libreoffice() == FALSE)
    {
        msgBoxError( window, "נכשל נסיון הקמת תקשורת עם השרת");
        goto FINISH;
    }
    // Tell Libreoffice server the template file name
    command = malloc(strlen(templateFilename) + 1);
    strcpy(command, templateFilename);
    COMM_send_command( COMM_CMD_TEMPLATE_FILE, command );
    // Tell Libreoffice server the notes file name
    if (strlen(destFilename) > strlen(templateFilename))
    {
        free(command);
        command = malloc(strlen(destFilename) + 1);
    }
    strcpy(command, destFilename);
    COMM_send_command( COMM_CMD_NOTES_FILE, command );
    // send all shipments notes
    free(command);
    command = malloc((sizeof(String32)+1)*(MAX_SHIPMENTS+MAX_EXTRA_SHIPMENTS));
    for (giverIndex = 0; giverIndex < giversnum; giverIndex++)
    {
        personNum = CALC_get_person_by_giver( giverIndex );
        shipments = CALC_get_shipments_num( personNum );
#ifdef DEBUG
        g_print("Giver[%d]: ",personNum);
#endif
        sprintf(command, "%s %s",DB_get_surname(personNum),DB_get_firstname(personNum));
        for (shipmentIndex=0; shipmentIndex < shipments; shipmentIndex++)
        {
            receiver = CALC_get_giver_shipment( personNum, shipmentIndex );
#ifdef DEBUG
            g_print("[%d] ", receiver);
#endif
            if (receiver >= 0)
            {
                sprintf(receiverName, ",%s %s",DB_get_surname(receiver),DB_get_firstname(receiver));
                strcat(command, receiverName);
            }
        } // for shipmentIndex
#ifdef DEBUG
        g_print("\n");
#endif
        sprintf(title, "%s %d %s %d", "מייצר פתק מספר", giverIndex, "מתוך", giversnum);
        gtk_label_set_text(labelMain, title);
#ifdef DEBUG
        g_print("%s: sending record: %s\n", __FUNCTION__, command);
#endif
        COMM_send_command(COMM_CMD_MAKE_NOTE, command);
        while (gtk_events_pending())
            gtk_main_iteration();
    } // for giverIndex
    // Tell Libreoffice server to close communication and shut down
    COMM_send_command( COMM_CMD_DONE, "Done");
    
    clock_gettime(CLOCK_REALTIME, &spec);
    sec_end = spec.tv_sec;
    ms_end = round(spec.tv_nsec / 1.0e6);
    if (ms_end > 999) { sec_end++; ms_end = 0; }
    seconds = sec_end - sec_start;
    minutes = seconds / 60;
    seconds = seconds % 60;
    sprintf(title, "%s.\n%s %d %s %d% s" , "הסתיים תהליך כתיבת הפתקים לקובץ", "התהליך ארך ",minutes ,"דקות ו ",seconds, "שניות");
    msgBoxSuccess(window, title);
FINISH:
    if (command != NULL) free(command);
    if (templateFilename != NULL) free(templateFilename);
    if (destFilename != NULL) free(destFilename);
    return TRUE;
    
NO_SHIPMENTS_DATA:
    msgBoxError(window, "אין נתוני משלוחים");
    return FALSE;
}


/*---------------------------------------------------------*/
gboolean make_note_button_pressed_callback(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    char msg[128];
    GtkWidget *row;
    long giversnum, receiversnum, giverIndex, receiver, personNum;
    char *command = NULL;
    char *destFilename = NULL;
    char *templateFilename = NULL;
    int shipments, shipmentIndex, len;
    char receiverName[2*sizeof(String32) + 4];
    char title[64];
    
    row = gtk_list_box_get_selected_row( list_box );
    if (row == NULL)
    {
        sprintf(msg, "יש לבחור משפחה מהרשימה");
        msgBoxError( window, msg );
        return FALSE;
    }
    personNum = gtk_list_box_row_get_index( row );
    if (personNum < 0)
    {
        sprintf(msg, "יש לבחור משפחה מהרשימה");
        msgBoxError( window, msg );
        return FALSE;
    }
    if (DB_is_free(personNum) == TRUE)
    {
        sprintf(msg, "המשפחה שבחרת פטורה ממשלוחים");
        msgBoxError( window, msg );
        return FALSE;
    }
    if (CALC_is_data_loaded() == FALSE)
        goto NO_SHIPMENTS_DATA;
    giversnum = CALC_get_givers_num();
    receiversnum = CALC_get_receivers_num();
    if ((giversnum < 1) || (receiversnum < 1))
        goto NO_SHIPMENTS_DATA;
    
    if ((templateFilename = msgBoxOpenfile( "*.odt" , "בחר קובץ תבנית")) != NULL)
        destFilename = msgBoxSavefile( "*.odt" , "בחר קובץ לשמירת הפתק");
    if ((templateFilename == NULL) || (destFilename == NULL))
        goto FINISH;
    // Open communication channel with Libreoffice
    if (COMM_build_comm_libreoffice() == FALSE)
    {
        msgBoxError( window, "נכשל נסיון הקמת תקשורת עם השרת");
        goto FINISH;
    }
    // Tell Libreoffice server the template file name
    command = malloc(strlen(templateFilename) + 1);
    strcpy(command, templateFilename);
    COMM_send_command( COMM_CMD_TEMPLATE_FILE, command );
    // Tell Libreoffice server the notes file name
    if (strlen(destFilename) > strlen(templateFilename))
    {
        if (command != NULL)
        {
            free(command);
            command = NULL;
        }
        command = malloc(strlen(destFilename) + 1);
    }
    strcpy(command, destFilename);
    COMM_send_command( COMM_CMD_NOTES_FILE, command );
    // send the note create command
    shipments = CALC_get_shipments_num( personNum );
    len = (sizeof(String32)+1)*(MAX_SHIPMENTS+MAX_EXTRA_SHIPMENTS);
    if (strlen(destFilename) < len)
    {
        free(command);
        command = malloc(len);
    }
    sprintf(command, "%s %s",DB_get_surname(personNum),DB_get_firstname(personNum));
    for (shipmentIndex=0; shipmentIndex < shipments; shipmentIndex++)
    {
        receiver = CALC_get_giver_shipment( personNum, shipmentIndex );
        if (receiver >= 0)
        {
            sprintf(receiverName, ",%s %s",DB_get_surname(receiver),DB_get_firstname(receiver));
            strcat(command, receiverName);
        }
    } // for shipmentIndex
    sprintf(title, "%s %d %s %d", "מייצר פתק עבור משפחה מספר", personNum, "מתוך", giversnum);
    gtk_label_set_text(labelMain, title);
    #ifdef DEBUG
    g_print("%s %d : %s %s\n", "שולח רשומת משלוחים מספר ", giverIndex, DB_get_surname(personNum),DB_get_firstname(personNum));
    #endif
    COMM_send_command(COMM_CMD_MAKE_NOTE, command);
    // Tell Libreoffice server to close communication and shut down
#ifdef DEBUG
     g_print("%s: sending Done.\n", __FUNCTION__);
#endif
    COMM_send_command( COMM_CMD_DONE, "Done");
    sprintf(msg, "%s %s %s %s", "הפתק למשפחת ", DB_get_surname(personNum),DB_get_firstname(personNum), "נוצר בהצלחה");
    msgBoxSuccess(window, msg);
    
FINISH:
    if (command != NULL) free(command);
    if (templateFilename != NULL) free(templateFilename);
    if (destFilename != NULL) free(destFilename);
    return TRUE;
    
NO_SHIPMENTS_DATA:
    msgBoxError(window, "אין נתוני משלוחים");
    return FALSE;
}

/*---------------------------------------------------------*/
gint timeout_make_notes_callback (gpointer user_data)
{
    static long giversnum, receiversnum, giverIndex;
    long receiver, personNum;
    static char *command = NULL;
    int shipments, shipmentIndex;
    char receiverName[2*sizeof(String32) + 4];
    char title[64];
    struct timespec spec;
    int minutes, seconds;

    if (*(long*)(user_data) == (-1)) return FALSE;
    if (*(long*)(user_data) == 0)
    { // initializing call
        if (CALC_is_data_loaded() == FALSE)
            {
            msgBoxError(window, "אין נתוני משלוחים");
            return FALSE;
        }
        giversnum = CALC_get_givers_num();
        receiversnum = CALC_get_receivers_num();
        if ((giversnum < 1) || (receiversnum < 1)) 
        {
            msgBoxError(window, "אין נתוני משלוחים");
            return FALSE;
        }
        if (command != NULL) free(command);
        command = malloc((sizeof(String32)+1)*(MAX_SHIPMENTS+MAX_EXTRA_SHIPMENTS));
        giverIndex = 0;
    }
    
    // send all shipments notes
    if (giverIndex < giversnum)
    {
        personNum = CALC_get_person_by_giver( giverIndex );
        shipments = CALC_get_shipments_num( personNum );
        sprintf(command, "%s %s",DB_get_surname(personNum),DB_get_firstname(personNum));
        for (shipmentIndex=0; shipmentIndex < shipments; shipmentIndex++)
        {
            receiver = CALC_get_giver_shipment( personNum, shipmentIndex );
            if (receiver >= 0)
            {
                sprintf(receiverName, ",%s %s",DB_get_surname(receiver),DB_get_firstname(receiver));
                strcat(command, receiverName);
            }
        } // for shipmentIndex
        sprintf(title, "%s %d %s %d", "מייצר פתק עבור משפחה מספר", personNum, "מתוך", giversnum);
        gtk_label_set_text(labelMain, title);
#ifdef DEBUG
        g_print("%s %d : %s %s\n", "שולח רשומת משלוחים מספר ", giverIndex, DB_get_surname(personNum),DB_get_firstname(personNum));
#endif
        COMM_send_command(COMM_CMD_MAKE_NOTE, command);
        giverIndex++;
        while (gtk_events_pending())
            gtk_main_iteration();
        return TRUE;
    } // if giverIndex
    else
    {
        // Tell Libreoffice server to close communication and shut down
#ifdef DEBUG
        g_print("%s: sending Done.\n", __FUNCTION__);
#endif
        COMM_send_command( COMM_CMD_DONE, "Done");
        if (command != NULL)
            { free(command); command = NULL; }
        clock_gettime(CLOCK_REALTIME, &spec);
        sec_end = spec.tv_sec;
        ms_end = round(spec.tv_nsec / 1.0e6);
        if (ms_end > 999) { sec_end++; ms_end = 0; }
        seconds = sec_end - sec_start;
        minutes = seconds / 60;
        seconds = seconds % 60;
        sprintf(title, "%s\n%s %d %s %d%s" , "הסתיים תהליך כתיבת הפתקים לקובץ", "התהליך ארך ",minutes ,"דקות ו ",seconds, "שניות");
        msgBoxSuccess(window, title);    
        return FALSE;
    }
}

