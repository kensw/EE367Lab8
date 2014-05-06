/*
 * This is the source code for the host.
 * hostMain is the main function for the host.  It is an infinite
 * loop that repeatedy polls the connection from the manager and
 * its input link.
 *
 * If there is command message from the manager,
 * it parses the message and executes the command.  This will
 * result in it sending a reply back to the manager.
 *
 * If there is a packet on its incoming lik, it checks if
 * the packet is destined for it.  Then it stores the packet
 * in its receive packet buffer.
 *
 * There is also a 10 millisecond delay in the loop caused by
 * the system call "usleep".  This puts the host to sleep.  This
 * should reduce wasted CPU cycles.  It should also help keep
 * all nodes in the network to be working at the same rate, which
 * helps ensure no node gets too much work to do compared to others.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "debug.h"
#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
/* It also indicates that the broadcast address */
#define MAXBUFFER 2000
#define PIPEWRITE 1
#define PIPEREAD  0
#define TENMILLISEC 10000   /* 10 millisecond sleep */

/*
 * hostInit initializes the host.  It calls
 * - hostInitState which initializes the host's state.
 * - hostInitRcvPacketBuff, which initializes the receive packet buffer
 * - hostInitSendPacketBuff, which initializes the send packet buffer
 */
void hostInitState(hostState * hstate, int physid);
void hostInitRcvPacketBuff(packetBuffer * packetbuff);
void hostInitSendPacketBuff(packetBuffer * packetbuff);

/*
 * hostMain is the main loop for the host. It has an infinite loop.
 * In the loop it first calls
 * hostCommandReceive to check if a command message came from the
 * manager.
 * If a command arrived, then it checks the first word of the
 * message to determine the type of command.  Depending on the
 * command it will call
 * - hostSetNetAddr to set the host's network address
 *      The command message should be "SetNetAddr <network address>"
 * - hostSetMainDir to set the host's main directory
 *      The command message should be "SetMainDir <directory name>"
 * - hostClearRcvFlg to clear the host's receive flag
 * - hostUploadPacket to upload a file to the host's send packet
 *      buffer. The command message should be "UploadPacket <file name>"
 * - hostDownloadPacket to download the payload of the host's
 *      receive packet buffer to a file.  The command message
 *      should be "DownloadPacket <file name>"
 * - hostTransmitPacket to transmit the packet in the send packet buffer.
 *      The command message should be "TransmitPacket <destination address>"
 * - hostGetHostState to get the host's state.  The command message
 *      should be "GetHostState".
 */
int  hostCommandReceive(managerLink * manLink, char command[]);

void hostSetNetAddr(hostState * hstate, int netaddr, char replymsg[]);
void hostSetMainDir(hostState * hstate, char filename[], char replymsg[]);
void hostClearRcvFlg(hostState * hstate, char replymsg[]);
void hostUploadPacket(hostState * hstate, char fname[], char replymsg[]);
void hostDownloadPacket(hostState * hstate, char fname[], char replymsg[]);
void hostTransmitPacket(hostState * hstate, char word[], char replymsg[]);
void hostGetHostState(hostState * hstate, managerLink * manLink, char replymsg[]);

/*
 * After host executes a command from the manager, it sends a reply
 * message to the manager in the format " Replytype <reply[]>".
 * - Replytype can be
 *    o "DISPLAY" which means the rest of the message should be
 *      displayed on the user's console
 *    o "GetHostStateACK" which means the rest of the message is the
 *      host's state
 */
void hostReplySend(managerLink * manLink, char replytype[], char reply[]);

/* This is used to send a message to the manager */
void hostToManSend(managerLink * manLink, char reply[]);

/*
 * Functions
 */

/*
 * hostTransmitPacket will transmit a packet in the send packet buffer
 */
void hostTransmitPacket(hostState * hstate, char word[], char replymsg[])
{
    char dest[1000];
    int  dstaddr;
    int i;
    
    /* Get the destination address from the manager's command message (word[]) */
    findWord(dest, word, 2);
    dstaddr = ascii2Int(dest);
    
    /*
     * Set up the send packet buffer's source and destination addresses
     */
    hstate->sendPacketBuff.dstaddr = dstaddr;
    hstate->sendPacketBuff.srcaddr = hstate->netaddr;
    
    
    /* Transmit the packet on the link */
    for (i=0; i<hstate->sendPacketBuff.paynum; i++) {
        linkSend(&(hstate->linkout), &(hstate->sendPacketBuff), i);
    }
        
    /* Message to be sent back to the manager */
    strcpy(replymsg,"Packet sent");
}


/*
 * Main loop of the host node
 *
 * It polls the manager connection for any requests from
 * the manager, and repliies
 *
 * Then it polls any incoming links and downloads any
 * incoming packets to its receive packet buffer
 *
 * Then it sleeps for 10 milliseconds
 *
 * Then back to the top of the loop
 *
 */
void hostMain(hostState * hstate)
{
    char buffer[1000]; /* The message from the manager */
    char word[1000];
    int  value;
    char replymsg[1000];   /* Reply message to be displayed at the manager */
    packetBuffer tmpbuff;
#ifdef debug
    debugnuminit(hstate->physid);
#endif
    
    
    while(1) {
        
        /* Check if there is a command message from the manager */
        int length; /* Size of string in pipe */
        length = hostCommandReceive(&(hstate->manLink),buffer);
#ifdef debug
        debugnummessageint(getpid(),hstate->physid);
        debugnummessage("hostcommandrec ",hstate->physid);
        debugnummessage(buffer,hstate->physid);
        debugnummessage(": ",hstate->physid);
        debugnummessageint(length,hstate->physid);
        debugnummessage("\n",hstate->physid);
#endif
        
        if (length > 1) { /* Execute the manager's command */
#ifdef debug
            debugnummessage("hostcommandrecexecute!\n",hstate->physid);
#endif
            findWord(word, buffer, 1);
            if (strcmp(word, "SetNetAddr")==0) {
                findWord(word, buffer, 2); /* Find net address */
                value = ascii2Int(word);   /* Convert it to integer */
                hostSetNetAddr(hstate, value, replymsg);
                hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
            }
            else if (strcmp(word, "SetMainDir")==0) {
                findWord(word, buffer, 2); /* Find directory name */
                hostSetMainDir(hstate, word, replymsg);
                hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
            }
            else if (strcmp(word, "ClearRcvFlg")==0) {
                hostClearRcvFlg(hstate, replymsg);
                hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
            }
            else if (strcmp(word, "UploadPacket")==0) {
                findWord(word, buffer, 2); /* Find file name */
                hostUploadPacket(hstate, word, replymsg);
                hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
            }
            else if (strcmp(word, "DownloadPacket")==0) {
                findWord(word, buffer, 2); /* Find file name */
                hostDownloadPacket(hstate, word, replymsg);
                hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
            }
            else if (strcmp(word, "GetHostState")==0) {
                hostGetHostState(hstate, &(hstate->manLink), replymsg);
                hostReplySend(&(hstate->manLink), "GetHostStateAck",replymsg);
            }
            else if (strcmp(word, "TransmitPacket")==0) {
                hostTransmitPacket(hstate, buffer, replymsg);
                hostReplySend(&(hstate->manLink), "DISPLAY", replymsg);
            }
        } /* end of if */
#ifdef debug
        debugnummessage("hmainloopend\n",hstate->physid);
#endif

        /* Check if there is an incoming packet */
        linkReceive(&(hstate->linkin), &tmpbuff);
        
#ifdef debug
        debugnummessage("linkReceivedone\n",hstate->physid);
#endif
        /*
         * If there is a packet and if the packet's destination address
         * is the host's network address then store the packet in the
         * receive packet buffer
         */
        if (tmpbuff.dstaddr == hstate->netaddr && tmpbuff.valid == 1 && tmpbuff.new == 1) {
            hstate->rcvPacketBuff = tmpbuff;
            hstate->rcvPacketBuff.new = 1;
            hstate->rcvPacketBuff.valid = 1;
        }
        
        /* The host goes to sleep for 10 ms */
        usleep(TENMILLISEC);
        
    } /* End of while loop */
    
}

/*
 * Sets the host's network address.  Also creates a reply message
 * to the manager.
 */
void hostSetNetAddr(hostState * hstate, int netaddr, char replymsg[])
{
    hstate->netaddr = netaddr;
    strcpy(replymsg, "Network address is set");
}

/*
 * The host checks the connection from the manager.  If the manager sent
 * a command, the host stores it in "command[]".  It returns the length
 * of the received message + 1.
 */

int hostCommandReceive(managerLink * manLink, char command[])
{
    int n;
    n = read(manLink->toHost[PIPEREAD],command,250);
#ifdef debug
    debugnummessageint(manLink->toHost[PIPEREAD],-1);
    debugnummessageint(manLink->toHost[PIPEREAD],-1);
    debugnummessageint(manLink->toHost[PIPEREAD],-1);
#endif
    command[n] = '\0';
    return n+1;
}

/*
 * The host sends a message to the manager.
 */
void hostToManSend(managerLink * manLink, char reply[])
{
    write(manLink->fromHost[PIPEWRITE],reply,strlen(reply));
}

/*
 * hostReplySend is called after the host executes a command
 * from the manager. It sends a reply
 * message to the manager in the format " Replytype <reply[]>".
 * - Replytype can be
 *    o "DISPLAY" which means the rest of the message should be
 *      displayed on the user's console
 *    o "GetHostStateACK" which means the rest of the message is the
 *      host's state
 */
void hostReplySend(managerLink * manLink, char replytype[], char replymsg[])
{
    char reply[1000];
    
    reply[0] = '\0';
    appendWithSpace(reply, replytype);
    appendWithSpace(reply, replymsg);
    hostToManSend(manLink, reply);
}

/*
 * Initializes the host.
 */
void hostInit(hostState * hstate, int physid)
{
    hostInitState(hstate, physid);     /* Initialize host's state */
    
    /* Initialize the receive and send packet buffers */
    hostInitRcvPacketBuff(&(hstate->rcvPacketBuff));
    hostInitSendPacketBuff(&(hstate->rcvPacketBuff));
#ifdef debug
    Newdebugmessage("Host physical ID: ", hstate->physid, NADA,NADA);
#endif
}

/*
 * Initialize send packet buffer
 */
void hostInitSendPacketBuff(packetBuffer * packetbuff)
{
    packetbuff->valid = 0;
    packetbuff->new = 0;
}


/*
 * Upload a file in the main directory into the send packet buffer
 */
void hostUploadPacket(hostState * hstate, char fname[], char replymsg[])
{
    FILE * fp;
    char path[MAXBUFFER];  /* Path to the file */
    char tempbuff[MAXBUFFER]; /* A temporary buffer */
    int length;
    int i;
    
    /*
     * Upload the file into tempbuff
     */
#ifdef debug
    Newdebugmessage("uploadstart!", NADA, NADA, NADA);
#endif
    
    if (hstate->maindirvalid == 0) {
        strcpy(replymsg, "Upload aborted:  the host's main directory is not yet chosen");
        return;
    }
    
    /* Create a path to the file and then open it */
    strcpy(path,"");
    strcat(path, hstate->maindir);
    strcat(path, "/");
    strcat(path, fname);
    
    fp = fopen(path,"rb");
    if (fp == NULL) { /* file didn't open */
        strcpy(replymsg, "Upload aborted: the file didn't open");
        return;
    }
    
    length = fread(tempbuff, 1, NEW_PAYLOAD_L*10+1, fp);
    
#ifdef debug
    Newdebugmessage("Tempbuff. Length:", length, NADA, NADA);
//    Newdebugmessage(tempbuff, NADA, NADA, NADA);
#endif
    
    if (length==0) {
        strcpy(replymsg, "Upload aborted: error in reading the file");
        return;
    }
    
#ifdef debug
    else if (length > PAYLOAD_LENGTH) {
        Newdebugmessage("uploading large file! Length:", length, NADA, NADA);
    }
#endif
    
    else if (length > NEW_PAYLOAD_L*10) { //changed
        strcpy(replymsg, "Upload aborted: file is too big");
        return;
    }
    
    tempbuff[length] = '\0';
    
    /* Fill in send packet buffer */
    
    hstate->sendPacketBuff.valid=1;
    hstate->sendPacketBuff.messlength=length;
    
     //initialize payloadbuff
    int x;
    int y;
    
//    for (y=0; y<10; y++) {
//        hstate->sendPacketBuff.payloads[y].length=0;
//        hstate->sendPacketBuff.payloads[y].type=0;
//    }
    
    //fill in payloadbuff
    
#ifdef debug
    Newdebugmessage("fill in payloadbuff", NADA, NADA, NADA);
#endif

    for (i=x=y=0; i<length; i++) { /* Store tempbuff in payloads of payload buffer of packet buffer */
        hstate->sendPacketBuff.payloads[y].message[x]=tempbuff[i];
        
#ifdef debug
        Newdebugmessage(hstate->sendPacketBuff.payloads[y].message, NADA, NADA, NADA);
#endif
        
        x++;
        
        if (x>NEW_PAYLOAD_L)
        {
            hstate->sendPacketBuff.payloads[y].length=x;
            hstate->sendPacketBuff.payloads[y].type=0;
            x=0;
            y++;
        }
    }
    hstate->sendPacketBuff.payloads[y].length=x;
    hstate->sendPacketBuff.payloads[y].type=1;
    hstate->sendPacketBuff.paynum=y+1;
#ifdef debug
    Newdebugmessage("paynum:", hstate->sendPacketBuff.paynum, NADA, NADA);
#endif
    
    /* Message to the manager */
    strcpy(replymsg, "Upload successful");
    
#ifdef debug
    Newdebugmessage(replymsg, hstate->sendPacketBuff.payloads[y].length, hstate->sendPacketBuff.payloads[y].type, NADA);
#endif
    
    fclose(fp);
}

/*
 * Initialize receive packet buffer
 */

void hostInitRcvPacketBuff(packetBuffer * packetbuff)
{
    packetbuff->valid = 0;
    packetbuff->new = 0;
    
    //initialize payloadbuff
    int y;
    for (y=0; y<10; y++) {
        packetbuff->payloads[y].length=0;
        packetbuff->payloads[y].type=0;
    }
}

/*
 * Download the payload of the packet buffer into a
 * file in the main directory
 */

void hostDownloadPacket(hostState * hstate, char fname[], char replymsg[])
{
    char c;
    FILE * fp;
    char path[MAXBUFFER];  /* Path to the file */
    char tempbuff[MAXBUFFER]; /* A temporary buffer */
    
#ifdef debug
    Newdebugmessage("hostDownloadPacket start",NADA,NADA, NADA);
#endif
    /* Create a path to the file and then open it */
    
    if (hstate->rcvPacketBuff.valid == 0) {
        strcpy(replymsg, "Download aborted: the receive packet buffer is empty");
        return;
    }
    
    if (hstate->maindirvalid == 0) {
        strcpy(replymsg, "Download aborted: the host's main directory is not yet chosen");
        return;
    }
#ifdef debug
    Newdebugmessage("hostDownloadPacket checks done",NADA,NADA, NADA);
#endif
    strcpy(path,"");
    strcat(path, hstate->maindir);
    strcat(path, "/");
    strcat(path, fname);
    printf("host:  path to the file: %s\n",path);
    fp = fopen(path,"wb");
    fputs("",fp); //clear file
    fclose(fp);
    fp = fopen(path,"a");

#ifdef debug
    Newdebugmessage("fill download buffer", NADA,NADA, NADA);
#endif
    
    /* Download the packet buffer payload into the tempbuff */
    int i;
#ifdef debug
    Newdebugmessage("paynum,1st paylen: ", hstate->rcvPacketBuff.paynum, hstate->rcvPacketBuff.payloads[i].length, NADA);
#endif
    for (i=0; i<hstate->rcvPacketBuff.paynum; i++) { /* Store tempbuff in payloads of payload buffer of packet buffer */
        fwrite(hstate->rcvPacketBuff.payloads[i].message,1,hstate->rcvPacketBuff.payloads[i].length,fp);
#ifdef debug
        Newdebugmessage(hstate->rcvPacketBuff.payloads[i].message, hstate->rcvPacketBuff.payloads[i].length, NADA, NADA);
#endif
    }
//    
//    /* Put tempbuff into file */
//    if (hstate->rcvPacketBuff.new == 1) {
//        fwrite(tempbuff,1,lengthpbuff(hstate->rcvPacketBuff),fp);
//        hstate->rcvPacketBuff.new=0;
//    }
    
    /* Message sent to the manager */
    strcpy(replymsg, "Download successful");
    fclose(fp);
}

/*
 * Clear the receive packet buffer
 */
void hostClearRcvFlg(hostState * hstate, char replymsg[])
{
    hstate->rcvflag = 0;
    hstate->rcvPacketBuff.valid = 0;
    hstate->rcvPacketBuff.new = 0;
    
    /* Message to the manager */
    strcpy(replymsg, "Host's packet received flag is cleared");
}

/*
 * Change main directory of the host
 */

void hostSetMainDir(hostState * hstate, char dirname[], char replymsg[])
{
    strcpy(hstate->maindir, dirname);
    hstate->maindirvalid = 1;
    
    /* Message to the manager */
    strcpy(replymsg, "Host's main directory name is changed");
}

/*
 * Get the host's state
 * - host's physical ID
 * - host's main directory
 * - host's network address
 * - host's neighbor address
 * - host's receive flag
 * and create a message that has all this information to be sent
 * to the manager
 */
void hostGetHostState(hostState * hstate, managerLink * manLink, char replymsg[])
{
    char word[1000];
    char empty[7] = "Empty";
    
    /* Create reply message */
    
    replymsg[0] = '\0';  /* Clear reply message */
    
    int2Ascii(word, hstate->physid);
    appendWithSpace(replymsg, word);
    
    if (hstate->maindirvalid==0) appendWithSpace(replymsg, empty);
    else appendWithSpace(replymsg, hstate->maindir);
    
    if (hstate->netaddr == EMPTY_ADDR) appendWithSpace(replymsg, empty);
    else {
        int2Ascii(word, hstate->netaddr);
        appendWithSpace(replymsg, word);
    }
    
    if (hstate->nbraddr == EMPTY_ADDR) appendWithSpace(replymsg, empty);
    else {
        int2Ascii(word, hstate->nbraddr);
        appendWithSpace(replymsg, word);
    }
    
    int2Ascii(word, hstate->rcvPacketBuff.new);
    appendWithSpace(replymsg, word);
    
}

/* 
 * Initialize the state of the host 
 */
void hostInitState(hostState * hstate, int physid)
{
    hstate->physid = physid;
    hstate->maindirvalid = 0; /* main directory name is empty*/
    hstate->netaddr = physid; /* default address */  
    hstate->nbraddr = EMPTY_ADDR;  
    hstate->rcvPacketBuff.valid = 0;
    hstate->rcvPacketBuff.new = 0;
}


