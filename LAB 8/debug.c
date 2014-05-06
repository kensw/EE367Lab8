//
//  debug.c
//  LAB 8
//
//  Created by Dee on 4/20/14.
//  Copyright (c) 2014 Dee. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include <time.h>
#include <unistd.h>
#include "debug.h"



//void debugpid(FILE * pFile)
//{
//    char buff[20];
//    int2Ascii(buff, getpid());
//    strcat(buff, " ");
//    fputs(buff, pFile);
//}
//
//
void debuginit()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buff[500];
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    FILE * pFile;
    pFile = fopen ("debug.txt","w");
    if (pFile!=NULL)
    {
        strcpy(buff, "\n\n======================================================\n");
        strcat(buff, "BEGIN LOG ON ");
        strcat(buff, asctime (timeinfo));
        strcat(buff, "\n\0");
        fputs(buff, pFile);
        fclose (pFile);
    }
}

void debugnuminit(int file)
{
    time_t rawtime;
    struct tm * timeinfo;
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    char buff[25];
    char num[3];
    FILE * pFile;
    strcpy(buff, "debug");
    int2Ascii(num, file);
    strcat(buff,num);
    strcat(buff,".txt");
    pFile = fopen (buff,"a");
    if (pFile!=NULL)
    {
        fputs ("\n\n======================================================\n",pFile);
        fputs ("BEGIN LOG ON ",pFile);
        fputs (asctime (timeinfo),pFile);
        fputs ("\n",pFile);
        fclose (pFile);
    }
}

void debugmessageint(int n)
{
    char message[500];
    FILE * pFile;
    pFile = fopen ("debug.txt","a");
    if (pFile!=NULL)
    {
        int2Ascii(message, n);
        fputs (message,pFile);
        fclose (pFile);
    }
}

void debugnummessageint(int n,int file)
{
    char message[500];
    char buff[25];
    char num[3];
    FILE * pFile;
    strcpy(buff, "debug");
    int2Ascii(num, file);
    strcat(buff,num);
    strcat(buff,".txt");
    pFile = fopen (buff,"a");
    if (pFile!=NULL)
    {
        int2Ascii(message, n);
        if (file<0) {
            strcat(message,"\n");
        }
        fputs (message,pFile);
        fclose (pFile);
    }
}

void debugnummessage(char * message,int file)
{
    char buff[25];
    char num[3];
    FILE * pFile;
    strcpy(buff, "debug");
    int2Ascii(num, file);
    strcat(buff,num);
    strcat(buff,".txt");
    pFile = fopen (buff,"a");

    if (pFile!=NULL)
    {
        fputs (message,pFile);
        fclose (pFile);
    }
}

void debugmessage(char * message)
{
    FILE * pFile;
    pFile = fopen ("debug.txt","a");
    if (pFile!=NULL)
    {
        fputs (message,pFile);
        fclose (pFile);
    }
}

void Newdebugmessage(char * message, int data1, int data2, int file)
{
    char buff[250];
    char num[5];
    FILE * pFile;
    strcpy(buff, "debug");
    if (file!=NADA) {
        int2Ascii(num, file);
        strcat(buff,num);
    }
    strcat(buff,".txt");
    pFile = fopen (buff,"a");
    
    if (pFile!=NULL)
    {
        if (strcmp(message, "INITIT")!=0)
        {
            int2Ascii(num,getpid()-getppid()-6);
            strcpy(buff, num);
            strcat(buff, " ");
            strcat(buff, message);
            strcat(buff, " ");
            if (data1!=-42) {
                int2Ascii(num, data1);
                strcat(buff, num);
                strcat(buff, ", ");
            }
            if (data2!=-42) {
                int2Ascii(num, data2);
                strcat(buff,num);
            }
            strcat(buff, "\n\0");
            fputs (buff,pFile);
        }
        else
        {
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            fputs ("\n\n======================================================\n",pFile);
            fputs ("BEGIN LOG ON ",pFile);
            fputs (asctime (timeinfo),pFile);
            fputs ("\n",pFile);
        }
        fclose (pFile);
    }
}