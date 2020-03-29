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

int main() 
{ 
    int fd1, fd2, index, cmd, elements;
    int refNum = 0, replyRef; 
    char * srvFileName = "/tmp/purimsrv.txt";
    char *clientFileName = "/tmp/purimclient.txt";
    char str1[80], refStr[5];
    char *cmdPtr;
    ssize_t bytes, newBytes;
    char *commands[] =
    {
        "001:XXXX:/home/mwiener/cfiles/ipc/purim_template.odt",
        "002:XXXX:/home/mwiener/cfiles/ipc/tmp_notes.doc",
        "003:XXXX:Moshe Wiener,Izak Asimov,Donald Trump",
        "003:XXXX:Izak Asimov,Donald Trump,Moshe Wiener",
        "003:XXXX:Donald Trump,Moshe Wiener,Izak Asimov",
        "004:XXXX:Done"
    };
    
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
#if 1 /* Temporary invoke LibreOffice manually */    
    // Invoke the LibreOffice file macro. Start with killing all existing LibreOffice unterminated background threads.
    //system("ps aux | grep -i office | awk {'print $2'} | xargs kill -9");
    system("soffice --headless --invisible \"vnd.sun.star.script:Standard.Module1.Main_Comm_Loop?language=Basic&location=application\"&");
    //system("soffice \"vnd.sun.star.script:Standard.Module1.Main_Comm_Loop?language=Basic&location=application\"&");
    //system("soffice  --nofirststartwizard --norestore \"/home/mwiener/cfiles/ipc/source.odt\" \"macro:///Standard.Module1.main_comm_loop\"&");
    //system("soffice  \"/home/mwiener/cfiles/ipc/source.odt\" &");
#endif
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
 
    elements = sizeof(commands)/ sizeof(commands[0]);
    printf("Will send %d commands to server\n", elements);
    for (cmd = 0; cmd < elements; cmd++) 
    { // loop to send all the commands in the array
        fd1 = open(clientFileName,O_WRONLY | O_CREAT); 
        if (fd1 < 0)
        {
            printf("Broken comm - open for write failed\n");
            unlink(clientFileName);
            return -1;
        }
        sprintf(str1,"%s\n", commands[cmd]);
        sprintf(refStr, "%04d", refNum);
        memcpy(&(str1[4]),refStr, 4);
        printf("Writing command #%d: %s", cmd, str1);
        write(fd1, str1, strlen(str1)+1);
        close(fd1);
        
        printf("Trying to read...\n");
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
            printf("Got response: %s [%d]\n", str1, strlen(str1));
            if (strncmp(str1, "goodby", 6) == 0)
            {
                unlink(srvFileName);
                unlink(clientFileName);
                return 0;
            }  
            replyRef = atoi(&(str1[4]));
            if (replyRef == refNum)
            {
                refNum++;
                break;
            }
            printf("Error: Expected reference %d but received %d\n", refNum, replyRef);
        } // loop until reading correct reply
    } // loop to send all the commands in the array
    printf("Finished loop...\n");
    unlink(clientFileName);
    return -1; 
} 
