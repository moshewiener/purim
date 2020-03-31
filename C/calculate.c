#include <purim_api.h>
#include<time.h>

#ifndef MAX_PUPULATION
#define MAX_PUPULATION  1000
#endif

typedef struct
{
    unsigned long   familynum;
    int             shipmentsnum;
    unsigned long   shipment[MAX_SHIPMENTS + MAX_EXTRA_SHIPMENTS];
} giverstruct;

typedef struct
{
    int             shipmentsnum;
    unsigned long   shipment[MAX_SHIPMENTS];
} receiverstruct;

typedef enum
{
    CALC_S_INIT = 0,
    CALC_S_GIVERSNUM,
    CALC_S_RECEIVERSNUM,
    CALC_S_1STLINETAIL,
    CALC_S_INIT_GIVERLINE,
    CALC_S_GIVERFAMILYNUM,
    CALC_S_GIVERSURNAME,
    CALC_S_GIVERFIRSTNAME,
    CALC_S_GIVERSHIPMENT,
    CALC_S_INITRECEIVERLINE,
    CALC_S_RECEIVERSURNAME,
    CALC_S_RECEIVERFIRSTNAME,
    CALC_S_RECIEVERSHIPMENT
} calc_state;

static giverstruct *givingToArray[MAX_PUPULATION] = { NULL };
static receiverstruct *receivingFromArray[MAX_PUPULATION] = { NULL };
static unsigned long giversNum = 0;
static unsigned long familiesNum = 0;
static gboolean is_shipments_data_loaded = FALSE;

static void free_calculations( void );
static int groupnumCompare(const void* p_index_a, const void* p_index_b);
static int familyNumberCompare(const void* a, const void* b);
static long find_giver_index( unsigned long personNum );
static gboolean remove_shipment_from_receiver( unsigned long sender, unsigned long receiver );
static gboolean remove_giver( unsigned long personNum );
static gboolean add_giver( unsigned long personNum );
static gboolean clear_giver_shipments( unsigned long personNum );
static gboolean add_giver_shipment( unsigned long personNum, unsigned long receiver );
static unsigned long maxMembersOfGroup( int *giversGroupArray, unsigned long giversNum);
static gboolean calc_state_machine( FILE *dbfd, char *p_msg );

/************************************/
gboolean CALC_is_data_loaded( void )
{
    if ((givingToArray[0] == NULL)      ||
        (receivingFromArray[0] == NULL) ||
        (giversNum == 0)                ||
        (familiesNum == 0)              ||
        (is_shipments_data_loaded == FALSE))
        return FALSE;
    return TRUE;
}

/************************************/
gboolean CALC_calculate_shipments( void )
{
    /*
     * Find how many families are giving shipments. Store in giversNum.
     * Allocate integer array of size giversNum in giversArray and fill it with record number of each giving family.
     * Allocate integer array of size giversNum in giversGroupArray and fill it with givers group number.
     * Now we wish to randomally blend the order of the giving families. Allocate integer arrays freeLocations and
     * randomLocations, both of size of giversNum, fill freeLocations with numbers from 0 to (giversNum-1), which is
     * to say that by now, all location are free (not yet appear in randomLocations array). Loop giversNum times, choose
     * a random location between 0 and a decreasing max value, and add the chosen number to randomLocations.
     * Now we have a list of numbers between 0 to giversNum-1 in a random order. So, if randomLocations[0] == 5, we'll
     * look at giversArray[5] to find the record index of the family.
     * We can now free freeLocations.
     * 
     * We can now change randomLocations so it directly contains the giving family indaces. For each item in
     * randomLocations: randomLocations[i]=giversArray[randomLocations[i]]
     * 
     * Now sort randomLocations according to group number, so we get continuous blocks of families with same group.
     * Find the group with most members, and how many givers are from this group - maxMembers.
     * 
     * Allocate an integer table 'givingToArray' of (num_of_shipmets+max_extra_shipments)*giversNum.
     * Alloicate an integer table 'receivingFromArray' of num_of_shipmets * familiesNum.
     * 
     * Now we run through all givers (with giverIndex), to tell to whom they give.
     * Each giver will give shipments to the givers in offset of maxMembers from him. (to ensure that he gives to 
     * other givers from different group) So, if our giver is at entry 0 of randomLocations, the biggest group has 10
     * members, and each gives 2 shipments, then he'll give to the families that appear in randomLocations[10] and
     * randomLocations[11].
     * If the index exceeds giversNum then it will be the index minus giversNum.
     * The shipment will be stored at givingToArray[familyNum][shipmentNum].
     * Whenever writing a giving shipment, it will also be written at the receiver side, at:
     * receivingFromArray[receivingIndex][receiver]=giver.
     * 
     * After setting all the normal shipments, if he has extra shipments defined, then add them too.
     * Now sort givingToArray and receivingFromArray by their family number.
     * Free randomLocations.
     */
    unsigned long maxMembers, index, giverIndex, receiver, receiverFamily;
    int *giversArray ,*giversGroupArray;
    int *freeLocations, *randomLocations;
    int groupNum, shipment, shipmentsNum, extraNum;
    unsigned long randNum;

    is_shipments_data_loaded = FALSE;
    familiesNum = DB_get_persons_num();
    shipmentsNum = DB_get_shipments_num();
    if (familiesNum <= shipmentsNum)
    {
        msgBoxError( window, "אין מספיק משפחות לבצע את כמות המשלוחים הרצויה");
        return FALSE;
    }
#ifdef DEBUG    
    g_print("calculate: familiesNum=%d. shipmentsNum=%d\n", familiesNum, shipmentsNum);
#endif
    if (familiesNum <= 1) return FALSE;
    /* allocate the givers and their corresponding group lists */
    giversArray = malloc( sizeof(int) * familiesNum );
    giversGroupArray = malloc( sizeof(int) * familiesNum );
    /* find number of givers and fill the givers array anbd their corresponding group number list */
    giversNum=0;
    for (index=0; index < familiesNum; index++)
    {
        if (DB_is_free( index )==FALSE)
        {
            giversArray[giversNum] = index;
            groupNum = DB_get_person_groupnumber( index );
            if ((groupNum < 0) || (groupNum >= MAX_GROUPS_NUM))
            {
                g_print("ERROR: calculate: DB_get_person_groupnumber(%d) returned %d\n", index, groupNum);
                return FALSE;
            }
            giversGroupArray[giversNum] = groupNum;
            giversNum++;
        }
    }
    maxMembers = maxMembersOfGroup( giversGroupArray, giversNum );
#ifdef DEBUG
    g_print("calculate: giversNum=%d maxMembers=%d\n", giversNum, maxMembers);
#endif    
    if ((giversNum <= 1) || (giversNum > MAX_PUPULATION)) return FALSE;
    /* allocate the sending shipments array */
    for (index = 0; index < giversNum; index++)
    {
        givingToArray[ index ] = malloc( sizeof(giverstruct) );
        memset(givingToArray[ index ], 0, sizeof(giverstruct));
    }
    /* allocate the receiving shipments array */
    for (index = 0; index < familiesNum; index++)
    {
        receivingFromArray[ index ] = malloc( sizeof(receiverstruct) );
        memset(receivingFromArray[ index ], 0, sizeof(receiverstruct));
    }
    /* allocate 2 tables needed to randomly blend the givers */
    freeLocations = malloc( sizeof(int) * giversNum );
    randomLocations = malloc( sizeof(int) * giversNum );
    /* fill the free locations table with all the locations of givers */
    for (index = 0; index < giversNum; index++)
        freeLocations[ index ] = index;
    
    srand(time(0));
    for (index = 0; index < giversNum; index++)
    {
        /* Generate random value between 0 and last free location. */
        randNum = rand() * (giversNum - 1 - index) / RAND_MAX;
        if ((randNum < 0)||(randNum >= giversNum))
        {
            g_print("ERROR: randNum exceeds range. %dn", randNum);
            break;
        }
        randomLocations[index] = freeLocations[randNum];
        //bring the last free location into the newly vacant location
        freeLocations[randNum] = freeLocations[giversNum - index - 1];
    }
    
    /* change randomLocations so it directly contains the giving family indaces. */
    for (index = 0; index < giversNum; index++)
    {
        randomLocations[index]=giversArray[randomLocations[index]];
    }

    /* sort randomLocations according to group number, so we get continuous blocks of families with same group. */
    qsort((void *)randomLocations, giversNum, sizeof(randomLocations[0]), groupnumCompare);
#ifdef DEBUG
    g_print("randomLocations\n================\n");
    for (giverIndex=0; giverIndex < giversNum; giverIndex++)
    {
        g_print("%03d) %d\n", giverIndex, randomLocations[giverIndex]);
    }
#endif   
    /* run through all givers and set their shipments */
    for (giverIndex=0; giverIndex < giversNum; giverIndex++)
    {
        givingToArray[giverIndex]->familynum = randomLocations[giverIndex];
        receiver = giverIndex + maxMembers;
        for (shipment = 0; shipment < shipmentsNum; shipment++)
        {
            if (receiver == giverIndex) receiver++; // don't allow person to give to himself
            if (receiver >= giversNum) receiver -= giversNum;
            if (receiver == giverIndex) receiver++; // don't allow person to give to himself
            if (receiver >= giversNum) receiver -= giversNum;
            receiverFamily = randomLocations[receiver];
            givingToArray[giverIndex]->shipment[shipment] = receiverFamily;
            receivingFromArray[receiverFamily]->shipment[receivingFromArray[receiverFamily]->shipmentsnum] =
                givingToArray[giverIndex]->familynum;
            receivingFromArray[receiverFamily]->shipmentsnum++;
            receiver++;
        } /* for shipment */
        /* add the extra shipments */
        extraNum = DB_get_extra_shipments_num(randomLocations[giverIndex]);
        for (shipment = 0; shipment < extraNum; shipment++)
        {
            receiverFamily = DB_get_extra_shipment( randomLocations[giverIndex], shipment );
            givingToArray[giverIndex]->shipment[shipmentsNum+shipment] = receiverFamily;
            
            receivingFromArray[receiverFamily]->shipment[receivingFromArray[receiverFamily]->shipmentsnum] =
                givingToArray[giverIndex]->familynum;
            receivingFromArray[receiverFamily]->shipmentsnum++;
        }
        givingToArray[giverIndex]->shipmentsnum =shipmentsNum + extraNum;
    } /* for giverIndex */
    
    /* sort the givings array according to family number */
    pointerssort((void *)givingToArray, giversNum, sizeof(giverstruct), familyNumberCompare);
    
    /* free the givers and their corresponding group lists and the 2 blending tables */
    free( giversArray );
    free( giversGroupArray );
    free( freeLocations );
    free( randomLocations );
 
    is_shipments_data_loaded = TRUE;
    msgBoxSuccess( window, "רשימות המשלוחים חושבו בהצלחה");
    return TRUE;
}

/********************************************************/
long CALC_get_giver_shipment( unsigned long personNum, int shipmentNum )
{
    long giverIndex;
    
    if (is_shipments_data_loaded == FALSE) return (-1);
    if (personNum >= familiesNum) return (-1);
    giverIndex = find_giver_index( personNum );
    if (giverIndex == (-1)) return (-1);
    if (givingToArray[giverIndex] == NULL) return (-1);
    if (shipmentNum >= givingToArray[giverIndex]->shipmentsnum) return (-1);
    return givingToArray[giverIndex]->shipment[shipmentNum];
}

/********************************************************/
long CALC_get_person_by_giver( unsigned long giver )
{    
    if (is_shipments_data_loaded == FALSE) return (-1);
    if (giversNum < 1) return (-1);
    if (giver >= giversNum) return (-1);
    return givingToArray[giver]->familynum;
}

/********************************************************/
long CALC_get_givers_num( void )
{
    if (is_shipments_data_loaded == FALSE) return (-1);
    if (familiesNum < 1) return (-1);
    if (givingToArray[0] == NULL) return (-1);
    return giversNum;
}

/********************************************************/
long CALC_get_receivers_num( void )
{
    if (is_shipments_data_loaded == FALSE) return (-1);
    if (familiesNum < 1) return (-1);
    if (receivingFromArray[0] == NULL) return (-1);
    return familiesNum;
}

/********************************************************/
int CALC_get_shipments_num( unsigned long personNum )
{
    int giverIndex;
    
    if (is_shipments_data_loaded == FALSE) return (-1);
    if (personNum >= familiesNum) return (-1);
    giverIndex = find_giver_index( personNum );
    if (giverIndex < 0) return (-1);
    if (givingToArray[giverIndex] == NULL) return (-1);
    return givingToArray[giverIndex]->shipmentsnum;
}

/*************************************************************/
gboolean CALC_manual_change_shipments( unsigned long personNum, int shipmentsNum, unsigned long *shipmentsArray )
{
    long giverIndex;
    int shipmentIndex;
    
    if (is_shipments_data_loaded == FALSE) return FALSE;
    if ((familiesNum < 1) || (personNum >= familiesNum)) return FALSE;
    if (receivingFromArray[0] == NULL) return FALSE;
    giverIndex = find_giver_index( personNum );
    
    if (giverIndex < 0)
    { // this person was not a giver
        // if this person was not a giver, and now it has 0 shipments then no change has occured
        if  (shipmentsNum == 0) return TRUE;
        // this person was not a giver. Now he becomes one
        add_giver( personNum );
        for (shipmentIndex = 0; shipmentIndex < shipmentsNum; shipmentIndex++)
            add_giver_shipment( personNum, shipmentsArray[shipmentIndex] );
    }
    else
    { // this person was a giver 
        if  (shipmentsNum == 0) // this person was a giver and now he isn't
            remove_giver( personNum );
        else
        { // was a giver, still a giver. Update shipments list
            clear_giver_shipments( personNum );
            for (shipmentIndex = 0; shipmentIndex < shipmentsNum; shipmentIndex++)
                add_giver_shipment( personNum, shipmentsArray[shipmentIndex] );
        }
    }
    return TRUE;
}

/*****************************************************/
static long find_giver_index( unsigned long personNum )
{
    long index;
    
    if (is_shipments_data_loaded == FALSE) return (-1);
    if (familiesNum < 1) return (-1);
    if (giversNum < 1) return (-1);
    for (index = 0; index < giversNum; index++)
    {
        if (givingToArray[index]->familynum == personNum)
            return index;
    }
    return (-1);
}

/******************************************************/
static gboolean remove_shipment_from_receiver( unsigned long sender, unsigned long receiver )
{
    int shipmentIndex, shipmentsNum, index;
    
    shipmentsNum = receivingFromArray[receiver]->shipmentsnum;
    // find the sender in the receiver shipments list
    for (shipmentIndex=0; shipmentIndex < shipmentsNum; shipmentIndex++)
    {
        if (receivingFromArray[receiver]->shipment[shipmentIndex] == sender)
            break;
    }
    for (index = shipmentIndex; index < (shipmentsNum-1); index++)
        receivingFromArray[receiver]->shipment[index] = receivingFromArray[receiver]->shipment[index + 1];
    receivingFromArray[receiver]->shipment[shipmentsNum-1]  = (-1);
    receivingFromArray[receiver]->shipmentsnum--;
    return TRUE;
}

/******************************************************/
static gboolean remove_giver( unsigned long personNum )
{
    unsigned long giverIndex, receiver, index;
    int shipmentsNum, shipmentIndex;
    
    giverIndex = find_giver_index( personNum );
    shipmentsNum = givingToArray[giverIndex]->shipmentsnum;
    for (shipmentIndex = 0; shipmentIndex < shipmentsNum; shipmentIndex++)
    {
        receiver = givingToArray[giverIndex]->shipment[shipmentIndex];
        remove_shipment_from_receiver( personNum, receiver );
    }
    // remove giver from givers array
    free( givingToArray[giverIndex] );
    for (index = giverIndex; index < (giversNum-1); index++)
        givingToArray[index] = givingToArray[index + 1];
    givingToArray[giversNum-1] = NULL;
    giversNum--;

    return TRUE;
}

/******************************************************/
static gboolean add_giver( unsigned long personNum )
{
    unsigned long giverIndex, index;
    
    if ((giversNum >= MAX_PUPULATION) || (personNum >= MAX_PUPULATION)) return FALSE;
    // find the location in where to add the new giver
    for (giverIndex = 0; giverIndex < giversNum; giverIndex++)
        if (givingToArray[giverIndex]->familynum > personNum) break;
    // allocate memory for the new giver at the end of the list
    givingToArray[giversNum] = malloc(sizeof(givingToArray[0]));
    // push forward all the givers that come after the position of thye new giver
    for (index = giversNum; index > giverIndex; index--)
    {
        memcpy(givingToArray[index] ,givingToArray[index-1], sizeof(givingToArray[0]));
    }
    // set the new giver entry
    givingToArray[giverIndex]->familynum = personNum;
    givingToArray[giverIndex]->shipmentsnum = 0;
    giversNum++;
    return TRUE;
}

/*****************************************************/
static gboolean clear_giver_shipments( unsigned long personNum )
{
    int shipmentsNum, shipmentIndex;
    long giverIndex;
    unsigned long receiver;
    
    if (personNum >= MAX_PUPULATION) return FALSE;
    giverIndex = find_giver_index( personNum );
    if (giverIndex < 0) return FALSE;
    shipmentsNum = givingToArray[giverIndex]->shipmentsnum;
    if (shipmentsNum < 1) return TRUE;
    for (shipmentIndex = 0; shipmentIndex < shipmentsNum; shipmentIndex++)
    {
        receiver = givingToArray[giverIndex]->shipment[shipmentIndex];
        remove_shipment_from_receiver( personNum, receiver );
    }
    givingToArray[giverIndex]->shipmentsnum = 0;
    return TRUE;
}

/*****************************************************/
static gboolean add_giver_shipment( unsigned long personNum, unsigned long receiver )
{
    int shipmentsNum, recvShipments;
    long giverIndex;
    
    if ((personNum >= MAX_PUPULATION) || (receiver >= MAX_PUPULATION)) return FALSE;
    giverIndex = find_giver_index( personNum );
    if (giverIndex < 0) return FALSE;
    shipmentsNum = givingToArray[giverIndex]->shipmentsnum;
    if (shipmentsNum >= (MAX_SHIPMENTS + MAX_EXTRA_SHIPMENTS)) return FALSE;
    recvShipments = receivingFromArray[receiver]->shipmentsnum;
    if (recvShipments >= MAX_SHIPMENTS) return FALSE;
    givingToArray[giverIndex]->shipment[shipmentsNum] = receiver;
    givingToArray[giverIndex]->shipmentsnum++;
    receivingFromArray[receiver]->shipment[recvShipments] = personNum;
    receivingFromArray[receiver]->shipmentsnum++;
    return TRUE;
}

/*********************************************************************************/
/* Find the group which has the most members, and return number of members       */
static unsigned long maxMembersOfGroup( int *giversGroupArray, unsigned long giversNum)
{
    int groupNum;
    int groupMembers[MAX_GROUPS_NUM];
    unsigned long giver, maxMembers = 0;
    
    for (groupNum = 0; groupNum < MAX_GROUPS_NUM; groupNum++) groupMembers[groupNum] = 0;
    for (giver = 0; giver < giversNum; giver++)
    {
        if ((giversGroupArray[giver] >=0) && (giversGroupArray[giver] < MAX_GROUPS_NUM))
        {
            groupMembers[giversGroupArray[giver]]++;
        }
    }
    for (groupNum = 0; groupNum < MAX_GROUPS_NUM; groupNum++)
    {
        if (groupMembers[groupNum] > maxMembers) maxMembers = groupMembers[groupNum];
    }
    return maxMembers;
}

/*********************************************************************************/
// Compare the group number of two persons
static int groupnumCompare(const void* p_index_a, const void* p_index_b) 
{ 
    int person_a, person_b, groupnum_a, groupnum_b, result;
    
    person_a = *((int*)p_index_a);
    person_b = *((int*)p_index_b);
    groupnum_a = DB_get_person_groupnumber((unsigned long)person_a);
    groupnum_b = DB_get_person_groupnumber((unsigned long)person_b);
    if (groupnum_a > groupnum_b) result = 1;
    else if (groupnum_b > groupnum_a) result = (-1);
    else result = 0;
    return result;
} 

/*********************************************************************************/
// Compare the family number of two persons
static int familyNumberCompare(const void* a, const void* b) 
{ 
    giverstruct *person_a, *person_b;
    int result;
    
    person_a = (giverstruct*)a;
    person_b = (giverstruct*)b;
    
    if (person_a->familynum > person_b->familynum) return 1;
    if (person_b->familynum > person_a->familynum) return (-1);
    return 0; 
} 

/***********************************************/
static void free_calculations( void )
{
    unsigned long index;
    
    /* free the giving and receiving records */
    for (index = 0; index < giversNum; index++)
    {
        free( givingToArray[ index ] );
        givingToArray[ index ] = NULL;
        free( receivingFromArray[ index ] );
        receivingFromArray[ index ] = NULL;
    }
    is_shipments_data_loaded = FALSE;
}

/***********************************************/
gboolean CALC_save_shipments( char *filename, char **errmsg )
{
    FILE *dbfd = NULL;
    unsigned long personNum, giverNum, receiverNum;
    int shipmentsnum,shipmentIndex;
    char buffer[256];
    char lmsg[128];
    char *p_msg;

    if (errmsg != NULL)
    {
        *errmsg = malloc(128);
        p_msg = *errmsg;
    }
    else
    {
        p_msg = lmsg;
    }
#ifdef DEBUG
    g_print("%s: giversNum %d familiesNum %d\n", __FUNCTION__, giversNum, familiesNum);
#endif
    if ((giversNum < 1) || (familiesNum < 1))
    {
        sprintf(p_msg, "%s", "אין משפחות ששולחות משלוח מנות");
        g_print("%s\n", p_msg);
        return FALSE;
    }
    
    dbfd = fopen(filename, "w+");
    if (dbfd == NULL)
    {
        sprintf(p_msg, "%s %48s", "לא ניתן לפתוח את קובץ ", filename);
        g_print("%s\n", p_msg);
        return FALSE;
    }
    // 1st line: giversNum,familiesNum,<CR>
    sprintf( buffer, "%lu%c%lu%c\n", giversNum, SEPARATOR, familiesNum, SEPARATOR );
    fputs(buffer, dbfd);
    
    // givers lines: personNum,surname,firstname,shipmentNum,shipment[0],...shipment[MAX_SHIPMENTS + MAX_EXTRA_SHIPMENTS],\n"
    for (giverNum = 0; giverNum < giversNum; giverNum++)
    {
        if (givingToArray[giverNum] == NULL) break;
        personNum = givingToArray[giverNum]->familynum;
        shipmentsnum = givingToArray[giverNum]->shipmentsnum;
        sprintf( buffer, "%lu%c%s%c%s%c",
                 personNum, SEPARATOR,
                 DB_get_surname( personNum ), SEPARATOR,
                 DB_get_firstname( personNum ), SEPARATOR );
        fputs(buffer, dbfd);
        for (shipmentIndex=0; shipmentIndex < shipmentsnum; shipmentIndex++)
        {
            sprintf( buffer, "%lu%c", givingToArray[giverNum]->shipment[shipmentIndex] , SEPARATOR);
            fputs(buffer, dbfd);
        } // for shipmentIndex...
        fputs("\n", dbfd);
    } // for giverNum...
    
    // receivers lines: surname,firstname,shipmentNum,shipment[0],...shipment[MAX_SHIPMENTS],\n"
    for (receiverNum = 0; receiverNum < familiesNum; receiverNum++)
    {
        if (receivingFromArray[receiverNum] == NULL) break;
        shipmentsnum = receivingFromArray[receiverNum]->shipmentsnum;
        sprintf( buffer, "%s%c%s%c",
                 DB_get_surname( receiverNum ), SEPARATOR,
                 DB_get_firstname( receiverNum ), SEPARATOR );
        fputs(buffer, dbfd);
        for (shipmentIndex=0; shipmentIndex < shipmentsnum; shipmentIndex++)
        {
            sprintf( buffer, "%lu%c", receivingFromArray[receiverNum]->shipment[shipmentIndex] , SEPARATOR);
            fputs(buffer, dbfd);
        } // for shipmentIndex...
        fputs("\n", dbfd);
    } // for receiverNum...
    
    fclose(dbfd);
    return TRUE;
}

/***********************************************/
void CALC_debug_print_shipments( void )
{
   unsigned long giverIndex, receiver, shipments, shipmentIndex;
   
   if (giversNum <= 1)
   {
       g_print("Too few givers. (%d)\n", giversNum);
       return;
   }
   g_print("Sending table\n=============\n");
   for (giverIndex = 0; giverIndex < giversNum; giverIndex++)
   {
       if (givingToArray[giverIndex] == NULL) break;
       shipments = givingToArray[giverIndex]->shipmentsnum;
       g_print("Family %d gives %d shipments to: ", givingToArray[giverIndex]->familynum, shipments);
       for (shipmentIndex = 0; shipmentIndex < shipments; shipmentIndex++)
           g_print("[%d] ", givingToArray[giverIndex]->shipment[shipmentIndex]);
       g_print("\n");
   }
   
   g_print("Receiving table\n===============\n");
   for (receiver = 0; receiver < familiesNum; receiver++)
   {
       if (receivingFromArray[receiver] == NULL) break;
       shipments = receivingFromArray[receiver]->shipmentsnum;
       g_print("Family %d gets from: ", receiver);
       for (shipmentIndex = 0; shipmentIndex < shipments; shipmentIndex++)
           g_print("[%d] ", receivingFromArray[receiver]->shipment[shipmentIndex]);
       g_print("\n");
   }
}


/*******************************************************************/
gboolean CALC_load_shipments(char *filename, char **errmsg)
{
    unsigned long lines, columns, index;
    FILE *dbfd = NULL;
    char lmsg[128];
    char *p_msg;

    if (errmsg != NULL)
    {
        *errmsg = malloc(128);
        p_msg = *errmsg;
    }
    else
    {
        p_msg = lmsg;
    }
    
    /* free previous used resources */
    is_shipments_data_loaded = FALSE;
    for (index = 0; index < MAX_PUPULATION; index++)
    {
        if (givingToArray[index] != NULL) { free( givingToArray[index] ); givingToArray[index] = NULL; }
        if (receivingFromArray[index] != NULL) { free( receivingFromArray[index] ); receivingFromArray[index] = NULL; }
    }
    
    countOfLinesAndColumnsFile(filename, &lines, &columns);
    if (lines < 1) 
    {
        g_print("%s: no valid purim data in file %s\n", __FUNCTION__, filename);
        sprintf( p_msg, "%s\n%s", "אין נתוני משלוחים בקובץ ", filename );
        goto FAIL;
    }
    
    dbfd = fopen(filename, "r");
    if (dbfd == NULL) 
    {
        g_print("%s: failed to open file %s\n", __FUNCTION__, filename);
        sprintf( p_msg, "%s\n%s", "לא ניתן לפתוח את קובץ", filename );
        goto FAIL;
    }
    
    if (calc_state_machine( dbfd, p_msg ) == FALSE) goto FAIL;
    
    fclose( dbfd );
    is_shipments_data_loaded = TRUE;
    return TRUE;
    
FAIL:
    if (dbfd != NULL)
    {
        fclose( dbfd );
        dbfd = NULL;
    }
    return FALSE;
}

/*******************************************************************
 * Retrieve data from the shipments file to set the counters
 * giversNum and familiesNum, and fill the arrays
 * givingToArray and receivingFromArray
********************************************************************/
static gboolean calc_state_machine( FILE *dbfd, char *p_msg )
{
    calc_state state = CALC_S_INIT;
    unsigned long filesize, lineNum, byteNum, giverIndex=0, receiverIndex=0;
    int chrIndex, newChr, tmpVal;
    int shipmentsnum, shipmentIndex;
    char buffer[sizeof(String32)];
    char firstname[sizeof(String32)];
    char surname[sizeof(String32)];
    
    fseek(dbfd, 0L, SEEK_END);
    filesize = ftell(dbfd);
    fseek(dbfd, 0L, SEEK_SET);
#ifdef DEBUG
    g_print("%s filesize=%lu...\n", __FUNCTION__, filesize);
#endif
    if (filesize <=0) 
    {
        g_print("%s: shipings file size 0\n", __FUNCTION__ );
        sprintf( p_msg, "%s:\n%s", __FUNCTION__, "קובץ המשלוחים ריק");
        return FALSE;
    }
    giversNum = 0; familiesNum = 0;
    lineNum = 0; chrIndex=0;
    for (byteNum=0; byteNum < filesize; byteNum++)
    {
        newChr = fgetc(dbfd);
        switch (state)
        {
            case CALC_S_INIT:
                if (newChr == SEPARATOR)
                {
                    sprintf(p_msg, "Error %s: Line %d starts with the CSV separator\n", __FUNCTION__, lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: Line %d is empty\n", __FUNCTION__, lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file is empty\n", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    buffer[0]=(char)(newChr); chrIndex=1; state = CALC_S_GIVERSNUM;
                }
                else // normal char
                {
                    sprintf( p_msg, "Error %s: illegal givers number", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                break;
            case CALC_S_GIVERSNUM:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    tmpVal = my_atoi( buffer );
                    if ((tmpVal > 0) && (tmpVal <= MAX_PUPULATION))
                    {
                        giversNum = tmpVal;
                        state = CALC_S_RECEIVERSNUM;
                    }
                    else
                    {
                        sprintf(p_msg, "Error %s: wrong numer of givers %d\n", __FUNCTION__, tmpVal );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: 1st line ends after number of givers\n", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends after number of givers\n", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < 10)
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    else
                    {
                        sprintf(p_msg, "Error %s: too large number of givers\n", __FUNCTION__ );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else // normal char
                {
                    sprintf(p_msg, "Error %s: illegal number of givers\n", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                break;
            case CALC_S_RECEIVERSNUM:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    tmpVal = my_atoi( buffer );
                    if ((tmpVal > 0) && (tmpVal <= MAX_PUPULATION))
                    {
                        familiesNum = tmpVal;
                        state = CALC_S_1STLINETAIL;
                    }
                    else
                    {
                        sprintf(p_msg, "Error %s: wrong numer of receivers %d", __FUNCTION__, tmpVal );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else if (newChr == '\n')
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    tmpVal = my_atoi( buffer );
                    if ((tmpVal > 0) && (tmpVal <= MAX_PUPULATION))
                    {
                        familiesNum = tmpVal;
                        lineNum++;
                        givingToArray[giverIndex] = malloc(sizeof(giverstruct));
                        state = CALC_S_INIT_GIVERLINE;
                    }
                    else
                    {
                        sprintf(p_msg, "Error %s: wrong numer of receivers %d", __FUNCTION__, tmpVal );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends after number of receivers", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < 10)
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    else
                    {
                        sprintf(p_msg, "Error %s: too large number of receivers", __FUNCTION__ );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else // normal char
                {
                    sprintf(p_msg, "Error %s: illegal number of receivers", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                break;
            case CALC_S_1STLINETAIL:
                if (newChr == SEPARATOR)
                {
                    // keep reading. Tail may contain several empty fields which make a sequence of separators
                }
                else if (newChr == '\n')
                {
                    givingToArray[giverIndex] = malloc(sizeof(giverstruct));
                    lineNum++;
                    state = CALC_S_INIT_GIVERLINE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends after number of receivers", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    sprintf(p_msg, "Error %s: shipments file has too many fields in 1st line", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else // normal char
                {
                    sprintf(p_msg, "Error %s: shipments file has too many fields in 1st line", __FUNCTION__ );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                break;
            case CALC_S_INIT_GIVERLINE:
                if (newChr == SEPARATOR)
                {
                    sprintf(p_msg, "Error %s: shipments file giver line %d start with a separator", __FUNCTION__ , lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: shipments file empty line %d", __FUNCTION__ , lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends after givers line %d", __FUNCTION__ , lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    buffer[0] = (char)newChr;
                    chrIndex = 1;
                    state = CALC_S_GIVERFAMILYNUM;
                }
                else // normal char
                {
                    sprintf(p_msg, "Error %s: givers line %d illegal family number", __FUNCTION__ , lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                break;
            case CALC_S_GIVERFAMILYNUM:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    tmpVal = my_atoi( buffer );
                    if ((tmpVal >= 0) && (tmpVal <= familiesNum))
                    {
                        givingToArray[giverIndex]->familynum = tmpVal;
#ifdef DEBUG                        
                        g_print("%s: givingToArray[%d]->familynum = %d\n", __FUNCTION__, giverIndex, givingToArray[giverIndex]->familynum);
#endif
                        state = CALC_S_GIVERSURNAME;
                    }
                    else
                    {
                        sprintf(p_msg, "Error %s: wrong giver family number at line %d", __FUNCTION__, lineNum );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: giver line ends after family number at line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends in middle of giver line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < 10)
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    else
                    {
                        sprintf(p_msg, "Error %s: too large family number at line %d\n", __FUNCTION__, lineNum );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else // normal char
                {
                    sprintf(p_msg, "Error %s: illegal family number at line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                break;
            case CALC_S_GIVERSURNAME:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    strcpy(surname, buffer); state = CALC_S_GIVERFIRSTNAME;
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: giver line ends after surname at line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends in middle of giver line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                else // normal char
                {if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    
                }
                break;
            case CALC_S_GIVERFIRSTNAME:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    shipmentsnum = 0; shipmentIndex = 0;
                    strcpy(firstname, buffer); state = CALC_S_GIVERSHIPMENT;
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: giver line ends after first name at line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends in middle of giver line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                else // normal char
                {if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    
                }
                break;
            case CALC_S_GIVERSHIPMENT:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    tmpVal = my_atoi( buffer );
                    if ((tmpVal >= 0) && (tmpVal <= familiesNum))
                    {
                        givingToArray[giverIndex]->shipment[shipmentIndex] = tmpVal;
#ifdef DEBUG                        
                        g_print("%s: givingToArray[%d]->shipment[%d] = %d\n", 
                                __FUNCTION__, giverIndex, shipmentIndex, givingToArray[giverIndex]->shipment[shipmentIndex] );
#endif                        
                        shipmentIndex++;
                    }
                    else
                    {
                        sprintf(p_msg, "Error %s: wrong shipment number at line %d index %d", __FUNCTION__, lineNum, shipmentIndex );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                    
                    givingToArray[giverIndex]->shipmentsnum = shipmentIndex;
                }
                else if (newChr == '\n')
                {
                    if (chrIndex > 0)
                    {
                        buffer[chrIndex]='\0';
                        tmpVal = my_atoi( buffer );
                        if ((tmpVal >= 0) && (tmpVal <= familiesNum))
                        {
                            givingToArray[giverIndex]->shipment[shipmentIndex] = tmpVal;
#ifdef DEBUG                            
                            g_print("%s: givingToArray[%d]->shipment[%d] = %d\n", 
                                    __FUNCTION__, giverIndex, shipmentIndex, givingToArray[giverIndex]->shipment[shipmentIndex] );
#endif
                            shipmentIndex++;
                        }
                        else
                        {
                            sprintf(p_msg, "Error %s: wrong shipment number at line %d index %d", __FUNCTION__, lineNum, shipmentIndex );
                            g_print("%s\n", p_msg);
                            return FALSE;
                        }
                    } // chrIndex > 0
                    givingToArray[giverIndex]->shipmentsnum = shipmentIndex;chrIndex=0;
                    lineNum++;
                    giverIndex++;
                    if (giverIndex < giversNum)
                    {
                        givingToArray[giverIndex] = malloc(sizeof(giverstruct));
                        state = CALC_S_INIT_GIVERLINE;
                    }
                    else
                    {
                        receivingFromArray[0] = malloc(sizeof(receiverstruct));
                        state = CALC_S_INITRECEIVERLINE;
                    }
                    
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipment file end at line %d with no receiving lines", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < 10)
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    else
                    {
                        sprintf(p_msg, "Error %s: too large shipment number at line %d index %d\n", __FUNCTION__, lineNum, shipmentIndex );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else // normal char
                {
                    sprintf(p_msg, "Error %s: wrong shipment number at line %d index %d", __FUNCTION__, lineNum, shipmentIndex );
                    g_print("%s\n", p_msg);
                    return FALSE;
                    
                }
                break;
            case CALC_S_INITRECEIVERLINE:
                if (newChr == SEPARATOR)
                {
                    sprintf(p_msg, "Error %s: shipments file receiver line %d start with a separator", __FUNCTION__ , lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: shipments file empty line %d", __FUNCTION__ , lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends after receivers line %d", __FUNCTION__ , lineNum);
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    buffer[0] = (char)newChr;
                    chrIndex = 1;
                    state = CALC_S_RECEIVERSURNAME;
                }
                else // normal char
                {
                    buffer[0] = (char)newChr;
                    chrIndex = 1;
                    state = CALC_S_RECEIVERSURNAME;
                }
                break;
            case CALC_S_RECEIVERSURNAME:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    strcpy(surname, buffer); state = CALC_S_RECEIVERFIRSTNAME;
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: receiver line ends after surname at line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends in middle of receiver line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                else // normal char
                {if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    
                }
                break;
            case CALC_S_RECEIVERFIRSTNAME:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    shipmentsnum = 0; shipmentIndex = 0;
                    strcpy(surname, buffer); state = CALC_S_RECIEVERSHIPMENT;
                }
                else if (newChr == '\n')
                {
                    sprintf(p_msg, "Error %s: receiver line ends after first name at line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if (newChr == EOF)
                {
                    sprintf(p_msg, "Error %s: shipments file ends in middle of receiver line %d\n", __FUNCTION__, lineNum );
                    g_print("%s\n", p_msg);
                    return FALSE;
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                }
                else // normal char
                {if (chrIndex < (sizeof(String32)-1))
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    
                }
                break;
            case CALC_S_RECIEVERSHIPMENT:
                if (newChr == SEPARATOR)
                {
                    buffer[chrIndex]='\0'; chrIndex=0;
                    tmpVal = my_atoi( buffer );
                    if ((tmpVal >= 0) && (tmpVal <= familiesNum))
                    {
                        receivingFromArray[receiverIndex]->shipment[shipmentIndex] = tmpVal;
 #ifdef DEBUG                       
                        g_print("%s: receivingFromArray[%d]->shipment[%d] = %d\n", 
                                __FUNCTION__, receiverIndex, shipmentIndex, receivingFromArray[receiverIndex]->shipment[shipmentIndex] );
#endif
                        shipmentIndex++;
                    }
                    else
                    {
                        sprintf(p_msg, "Error %s: wrong shipment number at line %d index %d", __FUNCTION__, lineNum, shipmentIndex );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                    
                    receivingFromArray[receiverIndex]->shipmentsnum = shipmentIndex;
                }
                else if (newChr == '\n')
                {
                    if (chrIndex > 0)
                    {
                        buffer[chrIndex]='\0';
                        tmpVal = my_atoi( buffer );
                        if ((tmpVal >= 0) && (tmpVal <= familiesNum))
                        {
                            receivingFromArray[receiverIndex]->shipment[shipmentIndex] = tmpVal;
#ifdef DEBUG
                            g_print("%s: receivingFromArray[%d]->shipment[%d] = %d\n", 
                                    __FUNCTION__, receiverIndex, shipmentIndex, receivingFromArray[receiverIndex]->shipment[shipmentIndex] );
#endif
                            shipmentIndex++;
                        }
                        else
                        {
                            sprintf(p_msg, "Error %s: wrong shipment number at line %d index %d", __FUNCTION__, lineNum, shipmentIndex );
                            g_print("%s\n", p_msg);
                            return FALSE;
                        }
                    } // chrIndex > 0
                    receivingFromArray[receiverIndex]->shipmentsnum = shipmentIndex;
                    chrIndex=0;
                    lineNum++;
                    receiverIndex++;
                    if (receiverIndex < familiesNum)
                    {
                        receivingFromArray[receiverIndex] = malloc(sizeof(receiverstruct));
                        state = CALC_S_INITRECEIVERLINE;
                    }
                    else
                    {
                        //got to end of last receiving line
#ifdef DEBUG
                        g_print("%s: Success\n", __FUNCTION__ );
#endif
                        return TRUE;
                    }
                    
                }
                else if (newChr == EOF)
                {
                    if ((receiverIndex + 1) < familiesNum)
                    {
                        sprintf(p_msg, "Error %s: shipment file only %d receiver lines while expecting %d", __FUNCTION__, (receiverIndex + 1), familiesNum );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                    if (chrIndex > 0)
                    {
                        buffer[chrIndex]='\0';
                        tmpVal = my_atoi( buffer );
                        if ((tmpVal >= 0) && (tmpVal <= familiesNum))
                        {
                            receivingFromArray[receiverIndex]->shipment[shipmentIndex] = tmpVal;
#ifdef DEBUG
                            g_print("%s: receivingFromArray[%d]->shipment[%d] = %d\n", 
                                    __FUNCTION__, receiverIndex, shipmentIndex, receivingFromArray[receiverIndex]->shipment[shipmentIndex] );
#endif
                            shipmentIndex++;
                        }
                        else
                        {
                            sprintf(p_msg, "Error %s: wrong shipment number at line %d index %d", __FUNCTION__, lineNum, shipmentIndex );
                            g_print("%s\n", p_msg);
                            return FALSE;
                        }
                    } // chrIndex > 0
                    receivingFromArray[receiverIndex]->shipmentsnum = shipmentIndex;
                    //got to end of last receiving line
#ifdef DEBUG
                    g_print("%s: Success\n", __FUNCTION__ );
#endif
                    return TRUE;
                    
                }
                else if ((newChr >= '0') && (newChr <= '9'))
                { // digit
                    if (chrIndex < 10)
                    { buffer[chrIndex] = (char)newChr; chrIndex++; }
                    else
                    {
                        sprintf(p_msg, "Error %s: too large shipment number at line %d index %d\n", __FUNCTION__, lineNum, shipmentIndex );
                        g_print("%s\n", p_msg);
                        return FALSE;
                    }
                }
                else // normal char
                {
                    sprintf(p_msg, "Error %s: wrong shipment number at line %d index %d", __FUNCTION__, lineNum, shipmentIndex );
                    g_print("%s\n", p_msg);
                    return FALSE;
                    
                }
                break;
            default:
                break;
        } // switch state
    } // for byteNum
    
    
    return TRUE;
}
