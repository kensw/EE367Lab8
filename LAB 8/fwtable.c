#include <stdio.h>
#include <stdlib.h>
#include "fwtable.h"

#define NOT_FOUND -1

// Creates a Table Entry for FWTable
FWTable * createTable(int dest, int link, int val)
{
   FWTable * entry;
   entry = (FWTable *) malloc(sizeof(FWTable));
   entry->dest_addr = dest;
   entry->out_link = link;
   entry->valid = val;
   entry->next = NULL;
   return entry;
}

void fwTableAdd(FWTable ** head, FWTable * newentry)
{
   /* Recursive Insertion */
   FWTable * indexptr;
   if(head == NULL){
      printf("Something went wrong attempting to add node");
      return;
   } else {
      indexptr = *head;
      if(indexptr == NULL){
         *head = newentry;
      } else {
         fwTableAdd(&(indexptr->next), newentry); 
      }
   }
}

FWTable ** fwTableSearch(FWTable ** head, int dest)
{
   /* Recursive Search */
   FWTable * indexptr;
   if(*head == NULL){
       return NULL;
   } else {
      indexptr = *head;
      if(indexptr->dest_addr == dest){
         return head;
      } else {
         return fwTableSearch(&(indexptr->next), dest);
      }
   }
}

void fwTableUpdate(FWTable ** head, int des_addr, int new_link, int val)
{
   FWTable ** des_index;
   des_index = fwTableSearch(head, des_addr);
   if(des_index != NULL){ 
      (*des_index)->out_link = new_link;
      (*des_index)->valid = val;
   }
}

int linkDestSearch(FWTable ** head, int des)
{
   FWTable ** sch = fwTableSearch(head, des);
   if(sch == NULL) {
      return NOT_FOUND; 
   } else {
      return ((*sch)->out_link);
   }
}

void fwTableIterate(FWTable ** head)
{
   FWTable * iterate = *head;
   while(iterate != NULL){
      printf("Addr: %d, Link:%d, Valid:%d \n", (iterate)->dest_addr,
      (iterate)->out_link, (iterate)->valid);
      iterate = (iterate)->next;
   }
}


void debugtable(FWTable ** head)
{
   FILE * write = fopen("Table.txt", "a");
   FWTable * iterate = *head;

   if(!write){
      printf("something wrong happened \n");
   } else {
      fprintf(write, "TABLE: ");
      while(iterate != NULL){
         fprintf(write,"Addr: %d Link:%d Valid:%d \n", (iterate)->dest_addr,
         (iterate)->out_link, (iterate)->valid);
         iterate = (iterate)->next;
      }
      fprintf(write,"----------\n");
      fclose(write);
   }
}

/*
//This is just driver code for testing purposes
int main()
{
   FWTable * head = createTable(0, 4, 0);
   FWTable * other = createTable(1,7,0);
   FWTable * newt = createTable(2,6,0);
   FWTable * test = createTable(3,1,1);
   

   fwTableAdd(&head, other);
   fwTableAdd(&head, newt);
   fwTableAdd(&head, test);
  
   fwTableUpdate(&head, 2, 4, 1);
   fwTableUpdate(&head, 4, 2, 1);

   fwTableIterate(&head);


   fwTableUpdate(&head, 0, 8, 1);
   printf("New Iteration: \n");
   fwTableIterate(&head); 

   printf("Corresponding outlink to dest 0 is %d \n", linkDestSearch(&head,0)); 
   
   printf("Corresponding outlink to dest 0 is %d \n", linkDestSearch(&head,9)); 
   
   FWTable ** sch = fwTableSearch(&head, 3);
   printf("Found: %d as link number,\n",(*sch)->out_link);
   sch = fwTableSearch(&head,9);
   if(sch != NULL){
      printf("null pointer",(*sch)->out_link);
   }
}*/
