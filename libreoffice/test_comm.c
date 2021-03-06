// C program to implement communication with the macro 'MainCommLoop'
// in source.odt LibreOffice document.
//  
#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 

#define MAX_ATTEMPS 100
#define MIN_RESPONSE_LEN 12

static char * srvFileName = "/tmp/purimsrv.txt";
static char *clientFileName = "/tmp/purimclient.txt";
static int refNum = 0;

//-------------------------------------------------------------
int COMM_build_comm_libreoffice( void )
{
    int fd1, index;
    
    //erase communication files if exist from previous session
    unlink(clientFileName);
    unlink(srvFileName); 
    printf("Creating Client file\n"); 
    fd1 = open(clientFileName, O_RDWR | O_CREAT, 0777); 
    if (fd1 < 0)
    {
        printf("Broken comm - open for create failed\n");
        return -1;
    }
    close(fd1);    
    // Invoke the LibreOffice file macro. Start with killing all existing LibreOffice unterminated background threads.
    system("ps aux | grep -i office | awk {'print $2'} | xargs kill -9");
    system("soffice --headless --invisible \"vnd.sun.star.script:Standard.Module1.Main_Comm_Loop?language=Basic&location=application\"&");
    
    for (index=0; index< MAX_ATTEMPS; index++)
    {
        if ( access( srvFileName, F_OK ) != -1 ) break;
        sleep(1);
    }
    if (index >= MAX_ATTEMPS)
    {
        printf("Broken comm - access failed\n");
        unlink(clientFileName);
        return -1;
    }
    return 0;
}

//-------------------------------------------------------------
int COMM_send_command( int commandId, char *body ) 
{ 
    int fd1, fd2, index;
    int replyRef; 
    char str1[80], refStr[5];
    char *cmdPtr;
    ssize_t bytes, newBytes;
    
    fd1 = open(clientFileName,O_WRONLY | O_CREAT); 
    if (fd1 < 0)
    {
        printf("Broken comm - open for write failed\n");
        unlink(clientFileName);
        return -1;
    }
    sprintf(str1,"%03d:%04d:%s\n", commandId, refNum, body);
#ifdef DEBUG
    printf("Writing command: %s", str1);
#endif
    write(fd1, str1, strlen(str1));
    close(fd1);
#ifdef DEBUG    
    printf("Trying to read...\n");
#endif
    sleep(1);
    memset(str1, 0, sizeof(str1));
    bytes = 0;
    while(1)
    { // loop until reading correct reply
        do
        {
            fd2 = open(srvFileName,O_RDONLY); 
            if (fd2 < 0)
            {
                printf("Broken comm - open for read failed\n");
                unlink(clientFileName);
                return -1;
            }
            newBytes = read(fd2, &(str1[bytes]), MIN_RESPONSE_LEN);
            bytes += newBytes;
            close(fd2);
            sleep(1);
        } while (bytes < MIN_RESPONSE_LEN);
        // Print the read string and close 
#ifdef DEBUG
        printf("Got response: %s [%d]\n", str1, strlen(str1));
#endif
        if (strncmp(str1, "goodby", 6) == 0)
        {
            unlink(srvFileName);
            unlink(clientFileName);
            return 1; // server finished and closed communication channel
        }  
        replyRef = atoi(&(str1[4]));
        if (replyRef == refNum)
        {
            refNum++;
            return 0; //got good mresponse
        }
        printf("Error: Expected reference %d but received %d\n", refNum, replyRef);
    } // loop until reading correct reply
    return -1; //good wrong reply
} 

//-------------------------------------------------------------
int main() 
{ 
    int fd1, fd2, index, cmd, elements;
    int  rc; 
    char str1[80], refStr[5];
    char *cmdPtr;
    ssize_t bytes, newBytes;
    int commandIDs[] =
    {
        1,2,3,3,3,4
    };
    char *commands[] =
    {
        "/home/mwiener/cfiles/ipc/purim_template.odt",
        "/home/mwiener/cfiles/ipc/tmp_notes.doc",
        "Moshe Wiener,Izak Asimov,Donald Trump",
        "Izak Asimov,Donald Trump,Moshe Wiener",
        "Donald Trump,Moshe Wiener,Izak Asimov",
        "Done"
    };
    
    if (COMM_build_comm_libreoffice() != 0)
    {
        printf("Broken comm - setup communication with Libreoffice failed\n");
        unlink(clientFileName);
        return -1;
    }
    
    elements = sizeof(commands)/ sizeof(commands[0]);
    printf("Will send %d commands to server\n", elements);
    for (cmd = 0; cmd < elements; cmd++)
    { // loop to send all the commands in the array
        printf("sending command [%d] %s\n", commandIDs[cmd], commands[cmd] );
        rc = COMM_send_command( commandIDs[cmd], commands[cmd] );
        if (rc == -1)
        {
            printf("Broken comm - sending command #d failed\n", cmd);
            unlink(clientFileName);
            return -1;
        }
        else if (rc == 1)
        {
            printf("Server finished and closed communication channel\n");
            unlink(srvFileName);
            unlink(clientFileName);
            return 0;
        }
        else printf("Command #%d sent successfuly\n", cmd);
    } // loop to send all the commands in the array
    printf("Finished loop...\n");
    unlink(clientFileName);
    return -1; 
} 

