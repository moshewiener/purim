#include <purim_api.h>

/********************************************************
states:
    0-no file loaded            1-flie loaded
    2-asked to add family       3-asked to del family
    4-asked to add group        5-asked to del group
    6-define num of shipments   7-asked to change family
    8-manual shipment           9-calculate
*********************************************************/ 
static gint state = 0;

int my_atoi( char *strNum )
{
    int index;
    int result = 0;
    int len = 0;
    char ch;
    
    if (strNum == NULL) return (-1);
    len = strlen(strNum);
    if ((len < 1) || (len > 10)) return (-1);
    for (index=0; index<len; index++)
    {
        ch = strNum[index];
        if ((ch < '0') || (ch > '9')) return (-1);
        result = result * 10 + (ch - '0');
    } /* for index */
    return result; 
}

/**************************************************************************************************/
void pointerssort(void **array, size_t nitems, size_t size, int (*compar)(const void *, const void*))
{
    int i, j;
    void *temp;
    
    temp = malloc(size);
    for (i = 0; i < nitems; i++)
    {
        for (j = i; j < nitems; j++)
        {
            if (compar(array[i] , array[j]) > 0)
            {
                memcpy(temp ,array[i], size);
                memcpy(array[i] , array[j], size);
                memcpy(array[j] , temp, size);
            }
        }
    }
    free( temp );
    return;
}



void hideAll( void )
{
    gtk_widget_hide( btnAddFamily );
    gtk_widget_hide( btnDelFamily );
    gtk_widget_hide( btnChgangeFamily );
    gtk_widget_hide( btnShipmentsNum );
    gtk_widget_hide( btnLoadDbFile );
    gtk_widget_hide( btnNewDB );
    gtk_widget_hide( btnAddGroup );
    gtk_widget_hide( btnDelGroup );
    gtk_widget_hide( btnSaveDbFile );
    gtk_widget_hide( btnExtra );
    gtk_widget_hide( btnManual );
    gtk_widget_hide( frame_listbox );
}

void hideDataboxAll( void )
{
    gtk_widget_hide( entry_Databox_1 );
    gtk_widget_hide( label_Databox_1 );
    gtk_widget_hide( entry_Databox_2 );
    gtk_widget_hide( label_Databox_2 );
    gtk_widget_hide( label_Databox_Main );
    gtk_widget_hide( chkbtn_Databox );
    gtk_widget_hide( btn_Databox_add_family );
    gtk_widget_hide( btn_Databox_del_family );
    gtk_widget_hide( btn_Databox_add_group );
    gtk_widget_hide( btn_Databox_del_group );
    gtk_widget_hide( btn_Databox_quit );
    gtk_widget_hide( listbox_Databox_1 );
    gtk_widget_hide( listbox_Databox_2 );
    gtk_widget_hide( label_Databox_box1 );
    gtk_widget_hide( label_Databox_box2 );
    gtk_widget_hide( scale_shipments );
    gtk_widget_hide( btn_Databox_save_shipments_num );
}
void go_state0( void )
{
    /* Data not yet loaded */
    hideAll();
    gtk_widget_show( btnLoadDbFile );
    gtk_widget_show ( btnNewDB );
}

void go_state1 ( void )
{
    /* After loading data. Main state */
    if (Databox_window != NULL) gtk_widget_hide( Databox_window );
    gtk_widget_show_all( window );
    hideAll();
    gtk_widget_show( btnAddFamily );
    gtk_widget_show( btnDelFamily );
    gtk_widget_show( btnChgangeFamily );
    gtk_widget_show( btnShipmentsNum );
    gtk_widget_show( btnAddGroup );
    gtk_widget_show( btnDelGroup );
    gtk_widget_show( btnSaveDbFile );
    gtk_widget_show( frame_listbox );
    gtk_widget_show( btnExtra );
    gtk_widget_show( btnManual );
    gtk_label_set_text(labelMain, "נתוני התושבים נטענו בהצלחה. בחר פעולה");
}

void go_state2 ( void )
/* Add family */
{
    databox_request_service( DATABOX_REQ_ADDFAMILY );
    gtk_widget_hide( window );
    gtk_widget_show_all( Databox_window );
    hideDataboxAll();
    gtk_label_set_text( label_Databox_Main, "הוספת משפחה");
    gtk_label_set_text( label_Databox_1, "שם משפחה");
    gtk_label_set_text( label_Databox_2, "שמות פרטיים");
    gtk_label_set_text( label_Databox_box2, "רשימת השכונות" );
    gtk_entry_set_text( entry_Databox_1, "" );
    gtk_entry_set_text( entry_Databox_2, "" );
    gtk_widget_show ( Databox_window );
    gtk_widget_show ( label_Databox_Main );
    gtk_widget_show( entry_Databox_1 );
    gtk_widget_show( label_Databox_1 );
    gtk_widget_show( entry_Databox_2 );
    gtk_widget_show( label_Databox_2 );
    gtk_widget_show( chkbtn_Databox );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbtn_Databox), FALSE);
    gtk_widget_show( btn_Databox_add_family );
    gtk_widget_show( btn_Databox_quit );
    gtk_widget_show( listbox_Databox_2 );
    gtk_widget_show( label_Databox_box2 );
}

void go_state3 ( void )
/* Delete family */
{
    databox_request_service( DATABOX_REQ_DELFAMILY );
    gtk_widget_hide( window );
    gtk_widget_show_all( Databox_window );
    hideDataboxAll();
    gtk_label_set_text( label_Databox_Main, "הסרת משפחה");
    gtk_label_set_text( label_Databox_box2, "רשימת התושבים" );
    gtk_widget_show ( label_Databox_Main );
    gtk_widget_show( btn_Databox_del_family );
    gtk_widget_show( btn_Databox_quit );
    gtk_widget_show( listbox_Databox_2 );
    gtk_widget_show( label_Databox_box2 );
}

void go_state4 ( void )
/* Add group */
{
    databox_request_service( DATABOX_REQ_ADDGROUP );
    gtk_widget_hide( window );
    gtk_widget_show_all( Databox_window );
    hideDataboxAll();
    gtk_label_set_text( label_Databox_Main, "הוספת קבוצה");
    gtk_label_set_text( label_Databox_1, "שם הקבוצה החדשה");
    gtk_label_set_text( label_Databox_box2, "רשימת השכונות" );
    gtk_entry_set_text( entry_Databox_1, "" );
    gtk_widget_show ( Databox_window );
    gtk_widget_show ( label_Databox_Main );
    gtk_widget_show( entry_Databox_1 );
    gtk_widget_show( label_Databox_1 );
    gtk_widget_show( btn_Databox_add_group );
    gtk_widget_show( btn_Databox_quit );
    gtk_widget_show( listbox_Databox_2 );
    gtk_widget_show( label_Databox_box2 );
}

void go_state5 ( void )
/* Delete group */
{
    databox_request_service( DATABOX_REQ_DELGROUP );
    gtk_widget_hide( window );
    gtk_widget_show_all( Databox_window );
    hideDataboxAll();
    gtk_label_set_text( label_Databox_Main, "הסרת קבוצה");
    gtk_label_set_text( label_Databox_box1, "הקבוצה אליה יעברו התושבים שהיו בקבוצה שנמחקה");
    gtk_label_set_text( label_Databox_box2, "רשימת השכונות" );
    gtk_widget_show ( Databox_window );
    gtk_widget_show ( label_Databox_Main );
    gtk_widget_show( btn_Databox_del_group );
    gtk_widget_show( btn_Databox_quit );
    gtk_widget_show( listbox_Databox_1 );
    gtk_widget_show( listbox_Databox_2 );
    gtk_widget_show( label_Databox_box1 );
    gtk_widget_show( label_Databox_box2 );
}

void go_state6 ( void )
/* Set shipments number */
{
    databox_request_service( DATABOX_REQ_SHIPMENTSNUM );
    gtk_widget_hide( window );
    gtk_widget_show_all( Databox_window );
    hideDataboxAll();
    gtk_label_set_text( label_Databox_Main, "מספר משלוחים");
    gtk_widget_show ( Databox_window );
    gtk_widget_show ( label_Databox_Main );
    gtk_widget_show( btn_Databox_save_shipments_num );
    gtk_widget_show( btn_Databox_quit );
    gtk_widget_show( scale_shipments );
}
