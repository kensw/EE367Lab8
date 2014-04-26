/*
 * FWTable.h
 */

#ifndef FWTABLE_H
#define FWTABLE_H

typedef struct FWTable {
   int dest_addr; //address of destination network
   int out_link; // outgoing link number
   int valid; // Is the entry valid?
   struct FWTable * next;
} FWTable;

FWTable * createTable(int dest, int link, int val);
void fwTableAdd(FWTable ** head, FWTable * newentry);
void fwTableUpdate(FWTable ** head, int des_addr, int new_link, int val);

FWTable ** fwTableSearch(FWTable ** head, int dest);
int linkDestSearch(FWTable ** head, int des);
void debugtable(FWTable ** head);

#endif