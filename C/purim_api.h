
#ifndef __PURIM_API_H__
#define __PURIM_API_H__
#include <gtk/gtk.h>
#include "purimicon.h"

typedef char String32[32];
typedef enum {
    DATABOX_REQ_ADDFAMILY,
    DATABOX_REQ_CHGFAMILY,
    DATABOX_REQ_DELFAMILY,
    DATABOX_REQ_ADDGROUP,
    DATABOX_REQ_DELGROUP,
    DATABOX_REQ_SHIPMENTSNUM,
    DATABOX_REQ_EXTRA,
    DATABOX_REQ_MANUAL
} databoxreq;

#define MAX_GROUPS_NUM      12
#define MAX_SHIPMENTS       12
#define MAX_EXTRA_SHIPMENTS 5
#define SEPARATOR ','

/* Main window Buttons */
extern GtkWidget *btnAddFamily, *btnDelFamily, *btnChgangeFamily, *btnShipmentsNum, *btnExtra;
extern GtkWidget *btnLoadDbFile, *btnNewDB, *btnAddGroup, *btnDelGroup, *btnSaveDbFile;
extern GtkWidget *btnCalculate, *btnSaveCalc, *btnLoadCalc, *btnManual;
extern GtkWidget *btnDebug, *btnNotes;

/* Other Main window widgets */
extern GtkWidget *labelMain, *frame_listbox;
extern GtkWidget* window, *list_box;

/* Databox widgets */
extern GtkWidget *btn_Databox_add_family, *btn_Databox_del_family, *btn_Databox_save_changes, *btn_Databox_quit;
extern GtkWidget *btn_Databox_add_group, *btn_Databox_del_group, *btn_Databox_save_shipments_num;
extern GtkWidget *btn_Databox_add_to_list, *btn_Databox_remove_from_list;
extern GtkWidget *Databox_window, *label1_Databox_hbox1, *label1_Databox_vbox2_1, *label1_Databox_vbox2_2, *label1_Databox_vbox2_3;
extern GtkWidget *label1_Databox_hbox2_3_1, *label1_Databox_hbox2_3_2;
extern GtkListBox *listbox_Databox_1, *listbox_Databox_2, *listbox_Databox_3;
extern GtkWidget *entry_Databox_1, *entry_Databox_2, *chkbtn_Databox, *scale_Databox_shipments;

/* General functions */
int my_atoi( char *strNum );
void pointerssort(void **array, size_t nitems, size_t size, int (*compar)(const void *, const void*));
GtkWidget *create_listbox_in_scrollwin( GtkWidget **p_list_box,
                                        gint entriesNum, String32 *names,
                                        GCallback row_selected_cb_function,
                                        void *user_data );
void remove_all_rows_of_listbox( GtkWidget *listbox );
void fill_listbox_with_persons( GtkWidget *listBox );
gboolean countOfLinesAndColumnsFile(char *filename, unsigned long *lines, int *columns);

/* Message boxes */
void show_about_window (GtkWindow *parent, gchar *message);
GtkResponseType msgBoxYesNo ( GtkWindow *window, char *msg );
GtkResponseType msgBoxError ( GtkWindow *window, char *msg );
GtkResponseType msgBoxSuccess ( GtkWindow *window, char *msg );
char *msgBoxOpenfile( char *fileFilter );

/* CSS colouring widget functions */
void init_css_table ( void );
GtkCssProvider *set_css_provider( gint colour, gint backcolour );
void css_set(GtkCssProvider *cssProvider, GtkWidget *g_widget);

/* CSV database functions */
gboolean DB_init_purim_db(char *filename);
gboolean DB_save_purim_db(char *filename);
void DB_close_purim_db( void );
gboolean DB_is_data_loaded( void );
unsigned long DB_get_persons_num( void );
unsigned long DB_get_givers_num( void );
int DB_get_shipments_num( void );
void DB_set_shipments_num( int shipments);
char *DB_get_firstname( unsigned long personNum );
char *DB_get_surname( unsigned long personNum );
int DB_get_groups_number( void );
char *DB_get_person_groupname( unsigned long personNum);
int DB_get_person_groupnumber( unsigned long personNum);
gboolean DB_set_person_groupnumber( unsigned long personNum, int groupNum);
char *DB_get_groupname( int groupNum );
gboolean DB_add_group( char *groupName );
gboolean DB_del_group( int groupNum, int movePersonsToGroup );
gboolean DB_is_free( unsigned long personNum );
gboolean DB_set_free( unsigned long personNum, gboolean nonSender);
int DB_get_extra_shipments_num( unsigned long personNum );
int DB_get_extra_shipment( unsigned long personNum, int shipmentIndex );
gboolean DB_add_extra_shipment( unsigned long personNum, unsigned long receiver );
gboolean DB_del_extra_shipment( unsigned long personNum, int shipmentIndex );
gboolean DB_add_family(char *firstname, char *surname, gboolean nonSender, int groupnum);
gboolean DB_del_family( unsigned long personNum );
long DB_find_family(char *firstname, char *surname);
/* CSV database debug functions */
void DB_debug_print_record( unsigned long personNum);
void DB_debug_print_all_records( void );
void DB_debug_print_groups( void );

/* Databox functions */
gboolean create_databox_window( char *title );
void databox_request_service( databoxreq req );

gboolean CALC_calculate_shipments( void );
gboolean CALC_save_shipments(char *filename, char **errmsg);
gboolean CALC_load_shipments(char *filename, char **errmsg);
long CALC_get_receivers_num( void );
long CALC_get_giver_shipment( unsigned long personNum, int shipmentNum );
gboolean CALC_manual_change_shipments( unsigned long personNum, int shipmentsNum, unsigned long *shipmentsArray );
gboolean CALC_is_data_loaded( void );
void CALC_debug_print_shipments( void );

/* widgets show states functions */
void hideAll( void );
void go_state0( void ); // no population nor shipments data is yet loaded
void go_state1( void ); // main screen after data was loaded but shipments not yet calculated
void go_state2( void ); //add family
void go_state3( void ); //delete family
void go_state4( void ); //add group
void go_state5( void ); //delete group
void go_state6( void ); // set shipments number
void go_state7( void ); // changfe family
void go_state8( void ); // extra shipments
void go_state9( void ); // manually change shipments
void go_state10( void ); //population data not loaded, shipments data loaded
void go_state11( void ); //both population data and shipments data are loaded
void go_main_state( void ); //choose between states 0,1,10,11
#endif /* __PURIM_API_H__ */
