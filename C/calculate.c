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

static giverstruct *givingToArray[MAX_PUPULATION] = { NULL };
static receiverstruct *receivingFromArray[MAX_PUPULATION] = { NULL };
static unsigned long giversNum = 0;
static unsigned long familiesNum = 0;

static void free_calculations( void );
static int groupnumCompare(const void* p_index_a, const void* p_index_b);
static int familyNumberCompare(const void* a, const void* b);
static unsigned long maxMembersOfGroup( int *giversGroupArray, unsigned long giversNum);

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
    
    familiesNum = DB_get_persons_num();
    shipmentsNum = DB_get_shipments_num();
    if (familiesNum <= shipmentsNum)
    {
        msgBoxError( window, "אין מספיק משפחות לבצע את כמות המשלוחים הרצויה");
        return FALSE;
    }
    
    g_print("calculate: familiesNum=%d\n", familiesNum);
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
    /* allocate 2 tables neede to randomly blend the givers */
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
        freeLocations[randNum] = freeLocations[giversNum - index - 1];
    }
    
    /* change randomLocations so it directly contains the giving family indaces. */
    for (index = 0; index < giversNum; index++)
    {
        randomLocations[index]=giversArray[randomLocations[index]];
    }

    /* sort randomLocations according to group number, so we get continuous blocks of families with same group. */
    qsort((void *)randomLocations, giversNum, sizeof(randomLocations[0]), groupnumCompare);

    /* run through all givers and set their shipments */
    for (giverIndex=0; giverIndex < giversNum; giverIndex++)
    {
        givingToArray[giverIndex]->familynum = randomLocations[giverIndex];
        receiver = giverIndex + maxMembers;
        for (shipment = 0; shipment < shipmentsNum; shipment++)
        {
            if (receiver >= giversNum) receiver -= giversNum;
            receiverFamily = randomLocations[receiver];
            givingToArray[giverIndex]->shipment[shipment] = receiverFamily;
            receivingFromArray[receiverFamily]->shipment[receivingFromArray[receiverFamily]->shipmentsnum] =
                givingToArray[giverIndex]->familynum;
            receivingFromArray[receiverFamily]->shipmentsnum++;
            receiver++;
        } /* for shipment */
        givingToArray[giverIndex]->shipmentsnum = shipmentsNum;
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
        givingToArray[giverIndex]->shipmentsnum += extraNum;
    } /* for giverIndex */
    
    /* sort the givings array according to family number */
    pointerssort((void *)givingToArray, giversNum, sizeof(giverstruct), familyNumberCompare);
    
    /* free the givers and their corresponding group lists and the 2 blending tables */
    free( giversArray );
    free( giversGroupArray );
    free( freeLocations );
    free( randomLocations );
    
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
       g_print("Family %d gives to: ", givingToArray[giverIndex]->familynum);
       for (shipmentIndex = 0; shipmentIndex < shipments; shipmentIndex++)
           g_print("[%d] ", givingToArray[giverIndex]->shipment[shipmentIndex]);
       g_print("\n");
   }
   
   g_print("Receiving table\n===============\n");
   for (receiver = 0; receiver < familiesNum; receiver++)
   {
       if (receivingFromArray[giverIndex] == NULL) break;
       shipments = receivingFromArray[receiver]->shipmentsnum;
       g_print("Family %d gets from: ", receiver);
       for (shipmentIndex = 0; shipmentIndex < shipments; shipmentIndex++)
           g_print("[%d] ", receivingFromArray[receiver]->shipment[shipmentIndex]);
       g_print("\n");
   }
}
