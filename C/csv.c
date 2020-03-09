#include <purim_api.h>

#define SEPARATOR ','

typedef struct
{
    String32    firstname;
    String32    surname;
    String32    groupname;
    int         groupnum;
    gboolean    free;
    int         extrashipments;
    int         extra[MAX_EXTRA_SHIPMENTS];
} db_person_struct;

typedef enum
{
    DB_S_INIT = 0,
    DB_S_SURNAME,
    DB_S_FIRSTNAME,
    DB_S_GROUPNAME,
    DB_S_FREE,
    DB_S_GROUPLIST,
    DB_S_SHIPMENTS,
    DB_S_EXTRA
} db_state;

static db_person_struct *p_person_records = NULL;
static unsigned long db_persons_count = 0;
static int db_shipments_num = 0;
static int db_groups_num = 0;
static db_state state = DB_S_INIT;
static String32 db_groupnames[MAX_GROUPS_NUM];
static int personCompare(const void* a, const void* b);
static void setGroupNumByGroupName( void );

static gboolean db_state_machine( FILE *dbfd );
static gboolean countOfLinesAndColumnsFile(char *filename, unsigned long *lines, int *columns);

/*************************************/
gboolean DB_init_purim_db(char *filename)
{
    int columns;
    FILE *dbfd = NULL;
    
    if (filename == NULL)
    { // Initialize a minimal default database
        if (p_person_records != NULL)
        {
            free(p_person_records);
        }
        p_person_records = p_person_records = malloc(sizeof(db_person_struct));
        if (p_person_records == NULL) return FALSE;
        strcpy(p_person_records[0].surname, "ישראלי" );
        strcpy(p_person_records[0].firstname, "אברהם ושרה" );
        p_person_records[0].free = FALSE;
        p_person_records[0].groupnum = 0;
        strcpy(db_groupnames[0], "שכונה א");
        strcpy(p_person_records[0].groupname, db_groupnames[0]);
        p_person_records[0].extrashipments = 0;
        db_persons_count = 1;
        db_groups_num = 1;
        db_shipments_num = 2;
        return TRUE;
    }
    if (countOfLinesAndColumnsFile(filename, &db_persons_count, &columns) == FALSE) return FALSE;
    if (db_persons_count < 1) return FALSE;
    // if memory was allocated in a previous call to this function then release it
    if (p_person_records != NULL) free(p_person_records);
    p_person_records = malloc(sizeof(db_person_struct) * db_persons_count);
    if (p_person_records == NULL) return FALSE;
    dbfd = fopen(filename, "r");
    if (dbfd == NULL) goto FAIL;
    if (db_state_machine(dbfd) == FALSE) goto FAIL;
    fclose( dbfd );
    setGroupNumByGroupName();
    
    return TRUE;
FAIL:
    if (dbfd != NULL) fclose( dbfd );
    free( p_person_records );
    p_person_records = NULL;
    return FALSE;
}

/********************************************/
gboolean DB_save_purim_db(char *filename)
{
    FILE *dbfd = NULL;
    unsigned long personNum;
    int extraNum;
    char buffer[256];
    char strShipmentsNum[8];
    char strExtra[8];
    
    if (p_person_records == NULL) return FALSE;
    if (0 >= db_persons_count) return FALSE;
    dbfd = fopen(filename, "w+");
    if (dbfd == NULL)
    {
        g_print("Failed to open file %s\n", filename);
        return FALSE;
    }
    sprintf(strShipmentsNum, "%d", db_shipments_num);
    //Run through all the records
    for (personNum=0; personNum < db_persons_count; personNum++)
    {
        sprintf(buffer, "%s%c%s%c%s%c%s%c%s%c%s%c",
                p_person_records[personNum].surname, SEPARATOR,
                p_person_records[personNum].firstname, SEPARATOR,
                p_person_records[personNum].groupname, SEPARATOR,
                (p_person_records[personNum].free==TRUE)? "+" : "", SEPARATOR,
                ((personNum < db_groups_num) && (personNum < MAX_GROUPS_NUM))?  db_groupnames[personNum] : "", SEPARATOR,
                (personNum == 0)? strShipmentsNum : "", SEPARATOR);
        for (extraNum=0; extraNum < p_person_records[personNum].extrashipments; extraNum++)
        {
            sprintf(strExtra, "%d%c", p_person_records[personNum].extra[extraNum], 
                    (extraNum < (p_person_records[personNum].extrashipments-1))? SEPARATOR : '\0');
            strcat(buffer, strExtra);
        }
        strcat(buffer, "\n");
        fputs(buffer, dbfd);        
    } // run through all the records
    
    fclose(dbfd);
    return TRUE;
}

/********************************************/
void DB_close_purim_db( void )
{
    if (p_person_records != NULL) free(p_person_records);
    p_person_records = NULL;
    db_persons_count = 0;
    db_shipments_num = 0;
    db_groups_num = 0;
    db_state state = DB_S_INIT;
}

/********************************************/
unsigned long DB_get_persons_num( void )
{
    return db_persons_count;
}

/********************************************/
unsigned long DB_get_givers_num( void )
{
    unsigned long personNum, givers;
    
    if (p_person_records == NULL) return 0;
    for (personNum = 0, givers = 0; personNum < db_persons_count; personNum++)
        if (p_person_records[personNum].free == FALSE)
            givers++;
    return givers;
}

/*******************************************/
int DB_get_shipments_num( void )
{
    return db_shipments_num;
}
/*******************************************/
void DB_set_shipments_num( int shipments)
{
    db_shipments_num = shipments;
}

/**************************************************/
char *DB_get_firstname( unsigned long personNum )
{
    if (p_person_records == NULL) return NULL;
    if (personNum > db_persons_count) return NULL;
    return p_person_records[personNum].firstname;
}

/**************************************************/
char *DB_get_surname( unsigned long personNum )
{
    if (p_person_records == NULL) return NULL;
    if (personNum > db_persons_count) return NULL;
    return p_person_records[personNum].surname;
}

/************************************************/
int DB_get_groups_number( void )
{
    if (p_person_records == NULL) return 0;
    return db_groups_num;
}
/**************************************************/
char *DB_get_person_groupname( unsigned long personNum )
{
    if (p_person_records == NULL) return NULL;
    if (personNum > db_persons_count) return NULL;
    return p_person_records[personNum].groupname;
}

/**************************************************/
char *DB_get_groupname( int groupNum )
{
    if (p_person_records == NULL) return NULL;
    if ((groupNum < 0) || (groupNum >= db_groups_num)) return NULL;
    return &(db_groupnames[ groupNum ][0]);
}

/**************************************************/
int DB_get_person_groupnumber( unsigned long personNum)
{
    if (p_person_records == NULL) return (-1);
    if (personNum >= db_persons_count) 
    {
        g_print("ERROR: DB_get_person_groupnumber(%lu) db_persons_count=%lu\n", personNum , db_persons_count);
        return (-1);
    }
    return p_person_records[personNum].groupnum;
}

/***************************************************/
gboolean DB_set_person_groupnumber( unsigned long personNum, int groupNum)
{
    char *errMsg;
    
    if (p_person_records == NULL)
    {
        errMsg = "נתוני התושבים לא טעונים";
        goto FAIL;
    }
    if (personNum >= db_persons_count)
    {
        errMsg = "מספר התושב חורג מעבר לגבול";
        goto FAIL;
    }
    if (groupNum >= MAX_GROUPS_NUM)
    {
        errMsg = "מספר השכונה חורג מעבר לגבול";
        goto FAIL;
    }
    
    p_person_records[personNum].groupnum = groupNum;
    strcpy( p_person_records[personNum].groupname , db_groupnames[groupNum] );
    return TRUE;
FAIL:
    msgBoxError( window, errMsg );
    return FALSE;
}

/**************************************************/
gboolean DB_add_group( char *groupName )
{
    int len;
    if (p_person_records == NULL) return FALSE;
    if (db_groups_num >= MAX_GROUPS_NUM) return FALSE;
    if (groupName == NULL) return FALSE;
    len = strlen(groupName);
    if ((len < 1) || (len >= sizeof(db_person_struct))) return FALSE;
    strcpy(db_groupnames[db_groups_num], groupName);
    db_groups_num++;
    return TRUE;
}

/**************************************************/
gboolean DB_del_group( int groupNum, int movePersonsToGroup )
{
    unsigned long index;
    if (p_person_records == NULL) return FALSE;
    if ((groupNum < 0) || (groupNum >= db_groups_num)) return FALSE;
    if (groupNum == movePersonsToGroup) return FALSE;
    if ((movePersonsToGroup < 0) || (movePersonsToGroup >= db_groups_num)) return FALSE;
    //erase the group from the groups list
    for (index = groupNum + 1; index < db_groups_num; index++)
    {
        memcpy( db_groupnames[index-1], db_groupnames[index], sizeof(db_groupnames[0]) );
    }
    memset(&(db_groupnames[db_groups_num-1]), 0, sizeof(db_groupnames[0]));
    // Run through all persons. For whoever is asigned a group greater than the removed group
    // numer,decrease its group number by 1. For all those with the removed group number, check
    // the replacing group number. If it is less that the removed group then asign it as is. If
    // it is greater then asign it minus 1, and also replace this person's group name accordingly. 
    for (index = 0; index < db_persons_count; index++)
    {
        if (p_person_records[index].groupnum > groupNum)
            p_person_records[index].groupnum--;
        else if (p_person_records[index].groupnum == groupNum)
        {
            if (movePersonsToGroup < groupNum)
            {
                p_person_records[index].groupnum = movePersonsToGroup;
                strcpy(p_person_records[index].groupname, db_groupnames[movePersonsToGroup]);
            }
            else
            {
                p_person_records[index].groupnum = movePersonsToGroup - 1;
                strcpy(p_person_records[index].groupname, db_groupnames[movePersonsToGroup -1]);
            }
        }
    }
    db_groups_num--;
    return TRUE;
}

/**************************************************/
gboolean DB_is_free( unsigned long personNum )
{
    if (p_person_records == NULL) return FALSE;
    if (personNum > db_persons_count) return FALSE;
    return p_person_records[personNum].free;
}

/**************************************************/
int DB_get_extra_shipments_num( unsigned long personNum )
{
    if (p_person_records == NULL) return (-1);
    if (personNum > db_persons_count) return (-1);
    return p_person_records[personNum].extrashipments;
}

/**************************************************/
int DB_get_extra_shipment( unsigned long personNum, int shipmentIndex )
{
    if (p_person_records == NULL) return (-1);
    if (personNum > db_persons_count) return (-1);
    if ((shipmentIndex < 0) || (shipmentIndex >= MAX_EXTRA_SHIPMENTS)) return (-1);
    return p_person_records[personNum].extra[shipmentIndex];
}

/*******************************************/
gboolean DB_add_extra_shipment( unsigned long personNum, unsigned long receiver )
{
    int extrashipments;
    if (p_person_records == NULL) return FALSE;
    if (personNum > db_persons_count) return FALSE;
    if (receiver > db_persons_count) return FALSE;
    extrashipments = p_person_records[personNum].extrashipments;
    if (extrashipments >= MAX_EXTRA_SHIPMENTS) return FALSE;
#ifdef DEBUG   
    g_print("DB_add_extra_shipment( %d, %d)\n", personNum, receiver);
#endif
    p_person_records[personNum].extra[extrashipments] = receiver;
    p_person_records[personNum].extrashipments++;
    return TRUE;
}

/*******************************************/
gboolean DB_del_extra_shipment( unsigned long personNum, int shipmentIndex )
{
    int extrashipments, index;
    if (p_person_records == NULL) return FALSE;
    if (personNum > db_persons_count) return FALSE;
    if (shipmentIndex > db_persons_count) return FALSE;
    extrashipments = p_person_records[personNum].extrashipments;
    if (shipmentIndex >= extrashipments) return FALSE;
    
#ifdef DEBUG 
    g_print("DB_del_extra_shipment( %d, %d)\n", personNum, shipmentIndex);
#endif
    for (index = shipmentIndex+1; index < extrashipments; index++)
        p_person_records[personNum].extra[index-1] = p_person_records[personNum].extra[index];
    p_person_records[personNum].extra[index-1] = (-1);
    p_person_records[personNum].extrashipments--;
    return TRUE;
}

/**************************************************/
void DB_debug_print_record( unsigned long personNum)
{
    int extra, i;
    
    if (p_person_records == NULL) return;
    if (personNum >= db_persons_count) return;
    printf("Firstname: %s%c Surname: %s%c Groupname: %s%c GroupNum: %02d %s Extra: ",
           DB_get_firstname(personNum), '\t',
           DB_get_surname(personNum), '\t',
           DB_get_person_groupname(personNum), '\t',
           p_person_records[personNum].groupnum,
           DB_is_free(personNum)? "free   " : " regular ");
    extra = p_person_records[personNum].extrashipments;
    for (i=0; i<extra; i++) printf("%d ", p_person_records[personNum].extra[i]);
    printf("\n");
}

/**************************************************/
void DB_debug_print_all_records( void )
{
    int extra, i;
    unsigned long personNum;
    
    if (p_person_records == NULL) return;
    g_print("Records\n======\n");
    for (personNum=0; personNum < db_persons_count; personNum++)
    {
        g_print("%03d) ",personNum);
        DB_debug_print_record( personNum );
    }
}

/*********************************/
void DB_debug_print_groups( void )
{
    int groupNum;
    
    if ((db_groups_num == 0) || (db_groups_num > MAX_GROUPS_NUM)) return;
    g_print("Groups (%02d)\n==========\n", db_groups_num);
    for (groupNum = 0; groupNum < db_groups_num; groupNum++)
        g_print("%d) %s\n", groupNum, db_groupnames[groupNum]);
}

/************************************************************************************/
gboolean DB_add_family(char *firstname, char *surname, gboolean nonSender, int groupnum)
{
    void *p_tmp;
    
    if (p_person_records == NULL) return FALSE;
    if ((firstname==NULL) || (surname==NULL)) return FALSE;
    if (groupnum >= db_groups_num) return FALSE;
    //re-allocate a bigger space for the persons table
    p_tmp = malloc(sizeof(db_person_struct) * (db_persons_count+1));
    memcpy(p_tmp, p_person_records, sizeof(db_person_struct) * db_persons_count);
    free(p_person_records);
    p_person_records = p_tmp;
    if ((groupnum < 0) || (groupnum >= MAX_GROUPS_NUM)) return FALSE;
    strcpy( p_person_records[db_persons_count].surname, surname );
    strcpy( p_person_records[db_persons_count].firstname, firstname );
    p_person_records[db_persons_count].groupnum = groupnum;
    p_person_records[db_persons_count].free = nonSender;
    p_person_records[db_persons_count].extrashipments = 0;
    strcpy(p_person_records[db_persons_count].groupname ,db_groupnames[groupnum]);
    db_persons_count++;
    qsort((void *)p_person_records, db_persons_count, sizeof(db_person_struct), personCompare);
    
    return TRUE;
}

/************************************************/
gboolean DB_del_family( unsigned long personNum )
{
    unsigned long index;
    
    if (p_person_records == NULL) return FALSE;
    if ((personNum < 0) || (personNum >= db_persons_count)) return FALSE;
    for (index = personNum + 1; index < db_persons_count; index++)
    {
        memcpy(&(p_person_records[index-1]), &(p_person_records[index]), sizeof(db_person_struct));
    }
     memset(&(p_person_records[db_persons_count-1]), 0, sizeof(db_person_struct));
    db_persons_count--;
    return TRUE;
}

/**************************************************/
long DB_find_family(char *firstname, char *surname)
{ 
    void *p_person;
    db_person_struct record;
    
    if (p_person_records == NULL) return (-1);
    if ((firstname==NULL) || (surname==NULL)) return (-1);
    if (db_persons_count < 1) return (-1);
    
    strcpy( record.surname, surname );
    strcpy( record.firstname, firstname );

    p_person = bsearch ((const void *)&record, (const void *)p_person_records, db_persons_count, sizeof(db_person_struct), personCompare);
    if (p_person == NULL) return (-1);
    return (long)(((unsigned long)p_person - (unsigned long)p_person_records) / sizeof(db_person_struct));
}

/****************************************/
gboolean DB_set_free( unsigned long personNum, gboolean nonSender)
{
    if (p_person_records == NULL) return FALSE;
    if ((personNum < 0) || (personNum >= db_persons_count)) return FALSE;
    p_person_records[personNum].free = nonSender;
    return TRUE;
}

/*********************************************************************************/
// Defining comparator function as per the requirement 
static int personCompare(const void* a, const void* b) 
{ 
    db_person_struct *person_a, *person_b;
    int result;
    
    person_a = (db_person_struct*)a;
    person_b = (db_person_struct*)b;
    result = strcmp(person_a->surname, person_b->surname);
    if (result !=0) return result;
  
    // setting up rules for comparison 
    return strcmp(person_a->firstname, person_b->firstname); 
} 


/*************************************************************************************/
static gboolean countOfLinesAndColumnsFile(char *filename, unsigned long *lines, int *columns)
{
    FILE* myfile = NULL;
    int ch;
    unsigned long number_of_lines = 0;
    int number_of_columns = 0;
    
    myfile = fopen(filename, "r");
    if (myfile == NULL)
        return FALSE;
    do
    {
        ch = fgetc(myfile);
        if(ch == '\n')
            number_of_lines++;
        if ((number_of_lines == 0) && (ch == SEPARATOR))
            number_of_columns++;
    } while (ch != EOF);
    fclose(myfile);
    *lines = number_of_lines;
    *columns = number_of_columns+1;
    
    return TRUE;
}

/**********************************************/
static void setGroupNumByGroupName( void )
{
    unsigned long personNum;
    int groupNum;
 
    if (p_person_records == NULL) return;
    if (db_groups_num < 1) return;
    for (personNum=0; personNum < db_persons_count; personNum++)
    {
        for (groupNum = 0; groupNum < db_groups_num; groupNum++)
        {
            if (strcmp(p_person_records[personNum].groupname, db_groupnames[groupNum])==0)
            {
                p_person_records[personNum].groupnum = groupNum;
                break;
            }
        }
        if (groupNum >= db_groups_num) p_person_records[personNum].groupnum = (-1);
    }
}

/**********************************************/
static gboolean db_state_machine( FILE *dbfd )
{
    unsigned long filesize, lineNum, byteNum;
    int chrIndex, newChr, extraNum;
    char buffer[sizeof(String32)];
    
    fseek(dbfd, 0L, SEEK_END);
    filesize = ftell(dbfd);
    fseek(dbfd, 0L, SEEK_SET);
    
    if (filesize <=0) return FALSE;
    lineNum = 0; chrIndex=0;
    db_groups_num=0;
    for (byteNum=0; byteNum < filesize; byteNum++)
    {
        newChr = fgetc(dbfd);
        switch (state)
        {
            case DB_S_INIT:
                if (newChr == SEPARATOR)
                {
                    g_print("Error csv db: Line %d starts with the CSV separator\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == '\n')
                {
                    g_print("Error csv db: Line %d is empty\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    // If the last line of the file ends with a NEWLINE char then we'll get here
                    if (lineNum > 0)
                        return TRUE;
                    g_print("Error csv db: End Of File at line %d\n", lineNum+1);
                    return FALSE;
                }
                else // normal char
                {
                    buffer[0]=(char)(newChr); chrIndex=1; state = DB_S_SURNAME;
                }
                break;
            case DB_S_SURNAME:
               if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    strcpy(p_person_records[lineNum].surname, buffer); state = DB_S_FIRSTNAME;
                }
                else if (newChr == '\n')
                {
                    g_print("Error csv db: End Of Line after Surname line %d\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    g_print("Error csv db: End Of File at line %d\n", lineNum+1);
                    return FALSE;
                }
                else // normal char
                {
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                break;
            case DB_S_FIRSTNAME:
               if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    strcpy(p_person_records[lineNum].firstname, buffer); state = DB_S_GROUPNAME;
                }
                else if (newChr == '\n')
                {
                    g_print("Error csv db: End Of Line after Firstname at line %d\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    g_print("Error csv db: End Of File at line %d\n", lineNum+1);
                    return FALSE;
                }
                else // normal char
                {
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                break;
            case DB_S_GROUPNAME:
               if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    strcpy(p_person_records[lineNum].groupname, buffer); state = DB_S_FREE;
                }
                else if (newChr == '\n')
                {
                    g_print("Error csv db: End Of Line after Groupname at line %d\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    g_print("Error csv db: End Of File at line %d\n", lineNum+1);
                    return FALSE;
                }
                else // normal char
                {
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                break;
            case DB_S_FREE:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    p_person_records[lineNum].free = ((buffer[0]=='+') && (buffer[1]=='\0')); state = DB_S_GROUPLIST;
                }
                else if (newChr == '\n')
                {
                    g_print("Error csv db: End Of Line after Free-column at line %d\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    g_print("Error csv db: End Of File at line %d\n", lineNum+1);
                    return FALSE;
                }
                else // normal char
                {
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                break;
            case DB_S_GROUPLIST:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0';
                    if ((db_groups_num < MAX_GROUPS_NUM) && (chrIndex > 0))
                    {
                        strcpy(db_groupnames[db_groups_num], buffer);
                        db_groups_num++;
                    }
                    chrIndex=0;
                    state = DB_S_SHIPMENTS;
                }
                else if (newChr == '\n')
                {
                    g_print("Error csv db: End Of Line after Grouplist at line %d\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    g_print("Error csv db: End Of File at line %d\n", lineNum+1);
                    return FALSE;
                }
                else // normal char
                {
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                break;
            case DB_S_SHIPMENTS:
                if (newChr == SEPARATOR)
                {
                    int shipments;
                    buffer[chrIndex]='\0'; chrIndex=0; extraNum=0;
                    shipments = my_atoi( buffer );
                    if ((shipments > 0) && (shipments <= MAX_SHIPMENTS))
                        db_shipments_num = shipments;
                    state = DB_S_EXTRA;
                }
                else if (newChr == '\n')
                {
                    g_print("Error csv db: End Of Line after Shipments at line %d\n", lineNum+1);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    g_print("Error csv db: End Of File at line %d\n", lineNum+1);
                    return FALSE;
                }
                else // normal char
                {
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                break;
            case DB_S_EXTRA:
                if (newChr == SEPARATOR)
                {
                    int tmpExtra = -1;
                    if (extraNum < MAX_EXTRA_SHIPMENTS)
                    {
                        if (chrIndex > 0) { buffer[chrIndex]='\0'; tmpExtra = my_atoi(buffer); }
                        if (tmpExtra < 0) tmpExtra = -1;
                        p_person_records[lineNum].extra[extraNum] = tmpExtra;
                        if (tmpExtra >= 0) extraNum++;
                    }
                    buffer[0]='\0'; chrIndex=0;
                }
                else if (newChr == '\n')
                {
                    int tmpExtra = -1;
                    if (extraNum < MAX_EXTRA_SHIPMENTS)
                    {
                        if (chrIndex > 0) { buffer[chrIndex]='\0'; tmpExtra = my_atoi(buffer); }
                        if (tmpExtra < 0) tmpExtra = -1;
                        p_person_records[lineNum].extra[extraNum] = tmpExtra;
                        if ((chrIndex > 0) && (tmpExtra >= 0)) extraNum++;
                    }
                    p_person_records[lineNum].extrashipments = extraNum;
                    lineNum++; buffer[0]='\0'; chrIndex=0; state = DB_S_INIT;
                }
                else if (newChr == EOF)
                {
                    int tmpExtra = -1;
                    if (extraNum < MAX_EXTRA_SHIPMENTS)
                    {
                        if (chrIndex > 0) { buffer[chrIndex]='\0'; tmpExtra = my_atoi(buffer); }
                        if (tmpExtra < 0) tmpExtra = -1;
                        p_person_records[lineNum].extra[extraNum] = tmpExtra;
                        if (tmpExtra >= 0) extraNum++;
                    }
                    p_person_records[lineNum].extrashipments = extraNum;
                    return TRUE;
                }
                else // normal char
                {
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                break;
            default:
                g_print("Error csv db: Bad state %d at line %d\n", state, lineNum+1);
                return FALSE;
        } /* switch state */
        
    } //for byteNum
    
    return TRUE;
}

