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
#include "link.h"
#include "man.h"
#include "host.h"
#include "fwtable.h"
#include "utilities.h"
#include "switchlink.h"
#include "pkqueue.h"
#include "switch.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
#define MAXBUFFER 1000
#define PIPEWRITE 1
#define PIPEREAD  0
#define FIRST_ADDR 0
#define INVALID 0
#define VALID 1
#define LINK_OFFSET 1
#define NOT_FOUND -1

void debug1(int stage)
{
    FILE * file = fopen("DEBUGFILE", "a");
    if(!file) {
        printf("Cant open \n");
    } else {
        fprintf(file, "Src: %d \n", stage);
        fclose(file);
    }
}

void debug2(int stage)
{
    FILE * file = fopen("DEBUGFILE", "a");
    if(!file) {
        printf("Cant open \n");
    } else {
        fprintf(file, "Dest: %d \n", stage);
        fprintf(file, "-----\n");
        fclose(file);
    }
}

void debug3(int stage, int id)
{
    FILE * file = fopen("DEBUGFILE", "a");
    if(!file) {
        printf("Cant open \n");
    } else {
        fprintf(file, "HostID: %d \n", id);
        fprintf(file, "LinkID Received from  %d \n", stage);
        fclose(file);
    }
}

void TestIterate(switchState * sstate, switchLinks ** head)
{
    
    FILE * data = fopen("tester", "a");
    fprintf(data, "SwitchID: %d \n",sstate->physid);
    fclose(data);
    
    switchLinks * ptr = *head;
    if(ptr == NULL) {
        FILE * data = fopen("bugger", "a");
        fprintf(data, "Something went wrong");
        fclose(data);
        
    }
    while(ptr != NULL) {
        data = fopen("tester", "a");
        fprintf(data, "Link ID %d \n", (ptr)->linkout.linkID);
        fprintf(data, "LinkOut src: %d, LinkOut dest: %d \n", (ptr)->linkout.uniPipeInfo.physIdSrc, (ptr)->linkout.uniPipeInfo.physIdDst);
        fprintf(data, "LinkIn ID: %d,\n", (ptr)->linkin.linkID);
        fprintf(data, "LinkIn src: %d, LinkIn dest: %d \n", (ptr)->linkin.uniPipeInfo.physIdSrc, (ptr)->linkin.uniPipeInfo.physIdDst);
        fprintf(data, "\n");
        fclose(data);
        ptr = (ptr)->next;
    }
}

void switchInitState(switchState * sstate, int phys)
{
    sstate->physid = phys;
#ifdef debug
    Newdebugmessage("Switch PID: ", sstate->physid, -42, -42);
#endif
}

void switchRecvPacketBuff(switchState * sstate, int in_id, packetBuffer * pbuff)
{
    pbuff->valid = 1;
    int src = pbuff->srcaddr;
    LinkInfo * out = linkSearch(&(sstate->sLinks), in_id);
    if(out == NULL){
        return;
    }
    int outlink = out->linkID;
    /* Update table entry */
    /* First Case Table is Empty */
    if(sstate->ftable == NULL){
        FWTable * fable = createTable(src, outlink, VALID);
        sstate->ftable = fable;
    } else {
        FWTable ** search_index = fwTableSearch(&(sstate->ftable), src);
        //If the source address doesn't already exist
        //(we have not associated the address with a link yet)
        if(search_index == NULL){
            FWTable * fable = createTable(src, outlink, VALID);
            fwTableAdd(&(sstate->ftable), fable);
        }
    }
    enQueue((sstate->recvPQ), *pbuff, in_id);
    
}

void switchSendAll(switchState * sstate, int src, packetBuffer * recv)
{
    //Head of link container
    switchLinks * ptr = sstate->sLinks;
    while(ptr != NULL) {
        if(ptr->linkin.linkID != src) {
            linkSend(&(ptr->linkout), recv);
        }
        ptr = ptr->next;
    }
    deQueue(sstate->recvPQ); //Pop top after sending
}

void switchSendPacketBuff(switchState * sstate)
{
    //Send data from top of queue
    //Packet from top of queue
    int destaddr, sourcelink;
    packetBuffer * temp = front(sstate->recvPQ);
    destaddr = temp->dstaddr;
    sourcelink = linksourcefront(sstate->recvPQ);
    
    //Forwarding Table Entry not found
    int dest_link = linkDestSearch(&(sstate->ftable), destaddr);
    if(dest_link == NOT_FOUND) {
        switchSendAll(sstate, sourcelink, temp);
    } else {
        //Entry exists
        LinkInfo * out = outputLink(&(sstate->sLinks), dest_link);
        linkSend(out, temp);
        deQueue(sstate->recvPQ); //Pop top after sending
    }
    //debugtable(&(sstate->ftable));
}

void scanAllLinks(switchState * sstate, packetBuffer * pb)
{
    switchLinks * ptr = sstate->sLinks;
    while(ptr != NULL){
        int data = 0;
        data = linkReceive(&(ptr->linkin), pb);
        if(data != -1) {
            //     debug3(ptr->linkin.linkID, sstate->physid);
            switchRecvPacketBuff(sstate, ptr->linkin.linkID, pb);
        }
        //Check something is being sent through link
        ptr = ptr->next;
    }
}

void switchSetLinkHead(switchState * sstate, switchLinks * head)
{
    sstate->sLinks = head;
}

void switchMain(switchState * sstate)
{
    sstate->recvPQ = createQueue();
    sstate->ftable = NULL;
    while(1){
        packetBuffer pb;
        scanAllLinks(sstate, &pb);
        if(!isEmpty(sstate->recvPQ)) {
            switchSendPacketBuff(sstate);
        }
        //   usleep(100000);
    }
}

