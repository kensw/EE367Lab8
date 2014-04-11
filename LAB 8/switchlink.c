#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "link.h"
#include "switchlink.h"

#define LINK_ERROR -2

switchLinks * constructLink(LinkInfo in, LinkInfo out)
{
   switchLinks * head;
   head = (switchLinks *) malloc(sizeof(switchLinks));
   head->linkin = in;
   head->linkout = out;
   head->next = NULL;
   return head;
}


switchLinks * getswitchLinks(linkArrayType * linkArray, int switchID, switchLinks * head)
{
   int i,j;
   for(i = 0; i < linkArray->numlinks; i++) {
      if(linkArray->link[i].uniPipeInfo.physIdDst == switchID) {
         LinkInfo in = linkArray->link[i];
         int source = linkArray->link[i].uniPipeInfo.physIdSrc;
         for(j = 0; j < linkArray->numlinks; j++) {
            if(linkArray->link[j].uniPipeInfo.physIdSrc == switchID &&
               linkArray->link[j].uniPipeInfo.physIdDst == source) {
                LinkInfo out = linkArray->link[j];
               if(head == NULL) {
                  head = constructLink(in, out);
                  break;
               } else {
                  switchLinks * newlink = constructLink(in, out);
                  addLink(&head, newlink);
               }
            }
         }
      }
   }
   return head;
}

void addLink(switchLinks ** head, switchLinks * newlink)
{

   /* Recursive Insertion */
   switchLinks * indexptr;

   if(*head == NULL){
      printf("Something went wrong attempting to add new link");
      return;
   } else {
      indexptr = * head;

      while(indexptr->next != NULL) {
         indexptr = indexptr->next;
      }
      
      indexptr->next = newlink;
      return;
   }
}

//Returns Outlink
//IN_ID = linkID of INPUT LINK
LinkInfo * linkSearch(switchLinks ** head, int in_id)
{
   /*Recursive Search*/
   switchLinks * index;
   if(*head == NULL){
      return NULL;
   } else {
      index = *head;
      if(index->linkin.linkID == in_id){
         return &(index->linkout);
      } else {
         return linkSearch(&(index->next), in_id);
      }
   }
}


LinkInfo * outputLink(switchLinks ** head, int out_id)
{
   /*Recursive Search*/
   switchLinks * index;
   if(*head == NULL){
      return NULL;
   } else {
      index = *head;
      if(index->linkout.linkID == out_id){
         return &(index->linkout);
      } else {
         return outputLink(&(index->next), out_id);
      }
   }
}

void InlinkIterate(switchLinks ** head)
{
   printf("\n");
   switchLinks * ptr = *head;
   while(ptr != NULL) {
      printf("LinkIn ID: %d,\n", (ptr)->linkin.linkID);
      printf("LinkIn src: %d, LinkIn dest: %d \n", (ptr)->linkin.uniPipeInfo.physIdSrc, (ptr)->linkin.uniPipeInfo.physIdDst);
      ptr = (ptr)->next;
      printf("\n");
   }
   printf("\n");
}
/*
//This is just drive code for testing purposes
int main()
{
   LinkInfo l_i;
   LinkInfo l_o;

   LinkInfo l_i2;
   LinkInfo l_o2;

   l_i.linkType = UNIPIPE;
   l_i.linkID = 0;
   l_i.uniPipeInfo.pipeType = NONBLOCKING;
   l_i.uniPipeInfo.physIdDst = 1;
   l_i.uniPipeInfo.physIdSrc = 0;

   l_o.linkType = UNIPIPE;
   l_o.linkID = 1;
   l_o.uniPipeInfo.pipeType = NONBLOCKING;
   l_o.uniPipeInfo.physIdDst = 0;
   l_o.uniPipeInfo.physIdSrc = 1;

   l_i2.linkType = UNIPIPE;
   l_i2.linkID = 3;
   l_i2.uniPipeInfo.pipeType = NONBLOCKING;
   l_i2.uniPipeInfo.physIdDst = 2;
   l_i2.uniPipeInfo.physIdSrc = 3;


   l_o2.linkType = UNIPIPE;
   l_o2.linkID = 4;
   l_o2.uniPipeInfo.pipeType = NONBLOCKING;
   l_o2.uniPipeInfo.physIdDst = 3;
   l_o2.uniPipeInfo.physIdSrc = 2;

   switchLinks * header = constructLink(l_i,l_o);
   switchLinks * test = constructLink(l_i2,l_o2);
   addLink(&header, test);
   TestIterate(&header);

   LinkInfo * tester = outputLink(&header, 4);
   printf("Tester: %d \n", tester->linkID);

   LinkInfo * searcher = linkSearch(&header,0);
   if(searcher == NULL){
      printf("something wrong happened \n");
   }
}*/
