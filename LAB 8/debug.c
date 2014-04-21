//
//  debug.c
//  LAB 8
//
//  Created by Dee on 4/20/14.
//  Copyright (c) 2014 Dee. All rights reserved.
//

#include <stdio.h>

void debuginit()
{
    FILE * pFile;
    pFile = fopen ("debug.txt","a");
    if (pFile!=NULL)
    {
        fputs ("BEGIN LOG\n",pFile);
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