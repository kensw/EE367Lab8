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


void debuginit()
{
    time_t rawtime;
    struct tm * timeinfo;
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    FILE * pFile;
    pFile = fopen ("debug.txt","a");
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

void debugintmessageint(int n,int file)
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
        fputs (message,pFile);
        fclose (pFile);
    }
}

void debugintmessage(char * message,int file)
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