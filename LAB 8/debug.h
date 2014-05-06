//
//  debug.h
//  LAB 8
//
//  Created by Dee on 4/20/14.
//  Copyright (c) 2014 Dee. All rights reserved.
//

#ifndef LAB_8_debug_h
#define LAB_8_debug_h
#define NADA -42

void debuginit();
void debugnuminit();

void debugmessage(char * message);
void debugmessageint(int n);
void debugnummessageint(int n,int file);
void debugnummessage(char * message,int file);
void Newdebugmessage(char * message, int data1, int data2, int file);

#define debug
#endif
