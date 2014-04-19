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

void debug(int stage)
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

////////////////////////////////////////////////////////////////////////////////////
void switchInitState(switchState * sstate, int phys)
{
   sstate->physid = phys;
   sstate->rootid = phys;
   sstate->distance = 0;
   sstate->parent = 0;
   sstate->numchild = 0;
   printf("Switch %d's root is %d\n", sstate->physid, sstate->rootid);
}

void switchRecvPacketBuff(switchState * sstate, int in_id, packetBuffer * pbuff)
{
	int i;
	int childflag = 0;
	if(pbuff->type != 1)
	printf("RecvPacketBuff pbuff->type = %d\n", pbuff->type);
	

	if(pbuff->type == 0)
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
	
	else if(pbuff->type == 1 && sstate->rootid > pbuff->payload[0])
	{

		sstate->rootid = pbuff->payload[0];
		sstate->distance = pbuff->payload[2] + 1;
		sstate->parent = pbuff->srcaddr;
		printf("NEW Root of switch %d is %d\n NEW Distance of switch %d is %d\n NEW Parent of switch %d is %d\n", sstate->physid, sstate->rootid, sstate->physid, sstate->distance, sstate->physid, sstate->parent);

	}
	else if(pbuff->type == 1 && pbuff->payload[3] == sstate->physid)
		{
			for(i = 0; i < 9; i++)
			{
				if(sstate->child[i] == pbuff->srcaddr)
					childflag = 1;
			}
			
			if(childflag == 0)
			{
		sstate->child[sstate->numchild] = pbuff->srcaddr;		
		printf("NEW Child[%d] of switch %d is %d\n",sstate->numchild, sstate->physid, sstate->child[sstate->numchild]);
		sstate->numchild++;		
			}	
		}  
//  	   deQueue(sstate->recvPQ); //Pop top after sending
		return;
	
	
}

void switchSendAll(switchState * sstate, int src, packetBuffer * recv)
{
   //Head of link container
   switchLinks * ptr = sstate->sLinks;
   int i;
   int flag = 0;
   int destaddr, sourcelink;
   packetBuffer * temp = front(sstate->recvPQ);
   destaddr = temp->dstaddr;
   sourcelink = linksourcefront(sstate->recvPQ);
   
 //  printf("switch %d sendAll, sourcelink: %d, destaddr: %d\n", sstate->physid, sourcelink, destaddr);
 
 /*  
   // Check if 
   for(i = 0 ; i < sstate->numchild+1 ; i++)   
   {
	   if(destaddr == sstate->child[i] || destaddr == sstate->parent)	   
   }
   */
   while(ptr != NULL) {
      if(ptr->linkin.linkID != src) {
	
		 // Check if linkout is a parent or child
	      for(i = 0 ; i < sstate->numchild+1 ; i++)   
	      {
	   	  	if(ptr->linkout.uniPipeInfo.physIdDst == sstate->child[i] || ptr->linkout.uniPipeInfo.physIdDst == sstate->parent)	   
			flag = 1;
			
			printf("ptr->linkout->UniPipeInfo->physIdDst = %d,  ptr->linkout.linkID = %d, sstate->child[i] = %d \n",ptr->linkout.uniPipeInfo.physIdDst, ptr->linkout.linkID, sstate->child[i]);
		 }
	 
		 printf("switch %d ptr->linkout.ID: %d \n", sstate->physid, ptr->linkout.linkID);


		 if(ptr->linkout.linkSwitch == 0)
		 {
			 printf("sending to host %d\n", ptr->linkout.uniPipeInfo.physIdDst);
			 linkSend(&(ptr->linkout), recv);
	 	 }
		 else if(flag == 1)
		 {
			printf("FLAG == 1, switch %d sending to ptr->linkout.linkID : %d\n", sstate->physid, ptr->linkout.linkID);
         	  	linkSend(&(ptr->linkout), recv);
    		 	flag = 0;
		 }
		 

	 }
      ptr = ptr->next;
   }
   deQueue(sstate->recvPQ); //Pop top after sending
}

void switchSendAllLocal(switchState * sstate)
{	
	
	packetBuffer netpacket;
	netpacket.srcaddr = sstate->physid;
	netpacket.length = 200;
	netpacket.type = 1;
	netpacket.payload[0] = sstate->rootid;
	netpacket.payload[2] = sstate->distance;
	netpacket.payload[3] = sstate->parent;
	
//	printf("Switch %d, Netpacket payload[0]-rootid = %d\n", sstate->physid, netpacket.payload[0]);
//	printf("Switch %d, Netpacket payload[2]-distance = %d\n", sstate->physid, netpacket.payload[2]);
//	printf("Switch %d, Netpacket payload[3]-parent = %d\n", sstate->physid, netpacket.payload[3]);
	//printf("Sending: Root of %d is: %d\n", sstate->physid, netpacket->payload[0]);
	
   //Head of link container
   switchLinks * ptr = sstate->sLinks;
   while(ptr != NULL) {
	 //  printf("switch %d SendAllLocal ptr->linkout.linkId = %d\n", sstate->physid, ptr->linkout.linkID);
         linkSend(&(ptr->linkout), &netpacket);
      ptr = ptr->next;
   }
}

void switchSendPacketBuff(switchState * sstate)
{
   //Send data from top of queue
   //Packet from top of queue
   int destaddr, sourcelink;
   packetBuffer * temp = front(sstate->recvPQ);
   destaddr = temp->dstaddr;
   sourcelink = linksourcefront(sstate->recvPQ);
   
   //printf("switch %d SendPacketBuff, sourcelink: %d, destaddr: %d\n", sstate->physid, sourcelink, destaddr);
   
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
		       debug3(ptr->linkin.linkID, sstate->physid);
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
   
   int count = 10;
  
   switchLinks * ptr = sstate->sLinks;
   while(ptr != NULL) {
	    printf("switch %d SendAllLocal ptr->linkout.linkId = %d\n", sstate->physid, ptr->linkout.linkID);
      ptr = ptr->next;
   }
  
  
   while(1){
	packetBuffer pb;
     scanAllLinks(sstate, &pb); 
    	
	if(!isEmpty(sstate->recvPQ) && count != 0) {		  
       switchSendPacketBuff(sstate);
	}   
	else if(count == 0) 
	{	switchSendAllLocal(sstate);
		count = 10;
	}
	
	count--;
	 usleep(100000);
   }

   count--;
}

