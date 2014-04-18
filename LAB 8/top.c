#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "top.h"

#define STAR 0
#define TREE 1
#define FAIL -1
#define SUCCESS 1
#define FILENAMEMAX 100

//Defaults (No Longer Needed)
#define TREE_SIZE 4
#define STAR_HOST 3
#define TREE_HOST 5
#define STAR_SWITCH 1
#define TREE_SWITCH 4

char tree_cmp[] = "tree";
char star_cmp[] = "star";

void initTopo(Topo *topology)
{
    topology->numlinks = 0;
    topology->numhosts = 0;
    topology->numswitch = 0;
    topology->type = -1;
}

void iterateLink(Topo * top)
{
    int x;
    for(x = 0; x < (top->numlinks*3); x++) {
        printf("%d \n", top->link[x]);
    }
}

void removeNewLine(char *line){
    line[strcspn (line, "\n")] = '\0';
}

void initTreeData(char *file, Topo *top)
{
    char *linebuff = NULL;
    size_t length = 0;
    ssize_t linesize;
    
    printf("[TREE] Topology detected.\n");
    
    FILE *data = fopen(file,"r");
    if(!data){
        printf("Error: Something wrong occurred!");
        return;
    }
    
    /* First three lines are NOT Link Data*/
    int index;
    for(index = 0; index < 3; index++) {
        linesize = getline(&linebuff, &length, data);
        removeNewLine(linebuff);
        if(linesize != -1){
            int val = atoi(linebuff);
            switch(index){
                case 0: break;
                case 1:
                    top->numhosts = val;
                    printf("Number of hosts: %d \n", val);
                    break;
                case 2:
                    top->numswitch = val;
                    printf("Number of switches: %d \n", val);
                    break;
            }
        } else {
            printf("Something went wrong reading first three lines \n");
            return;
        }
    }
    
    int count = 0;
    while((linesize = getline(&linebuff, &length, data)) != -1){
        removeNewLine(linebuff);
        int num = atoi(linebuff);
        top->link[count] = num;
        count++; // Increment Counter
    }
    int num_link = count / 3;
    top->numlinks = num_link;
    printf("Number of links: %d \n", num_link);
    fclose(data);
    
}


void initStarData(char *file, Topo *top)
{
    char *linebuff = NULL;
    size_t length = 0;
    ssize_t linesize;
    
    printf("[STAR] Topology detected.\n");
    FILE *data = fopen(file,"r");
    if(!data){
        printf("Error: Something wrong occurred!");
        return;
    }
    
    /* First three lines are NOT Link Data*/
    int index;
    for(index = 0; index < 3; index++) {
        linesize = getline(&linebuff, &length, data);
        removeNewLine(linebuff);
        if(linesize != -1){
            int val = atoi(linebuff);
            switch(index){
                case 0: break;
                case 1:
                    top->numhosts = val;
                    printf("Number of hosts: %d \n", val);
                    break;
                case 2:
                    top->numswitch = val;
                    printf("Number of switches: %d \n", val);
                    break;
            }
        } else {
            printf("Something went wrong reading first three lines \n");
            return;
        }
    }
    
    int count = 0;
    while((linesize = getline(&linebuff, &length, data)) != -1){
        removeNewLine(linebuff);
        int num = atoi(linebuff);
        top->link[count] = num;
        count++; // Increment Counter
    }
    int num_link = count / 3;
    top->numlinks = num_link;
    printf("Number of links: %d \n", num_link);
    fclose(data);
}

int initializeTop(Topo *topology)
{
    char filename[FILENAMEMAX];
    char *linebuff = NULL;
    size_t length = 0;
    ssize_t linesize;
    
    printf("Enter file name for topology: ");
    scanf("%s", &filename);
    
    FILE *file = fopen(filename,"r");
    if(!file){
        printf("ERROR: File not found..\n");
        return FAIL;
    } else {
        linesize = getline(&linebuff, &length, file);
        removeNewLine(linebuff);
        if(linesize == -1) {
            printf("Incorrect syntax detected..stopping.\n");
            fclose(file);
            return FAIL;
        } else {
            if(strcmp(linebuff, tree_cmp) == 0) {
                //Remember to close file
                fclose(file);
                initTreeData(filename, topology);
                return SUCCESS;
            } else if (strcmp(linebuff, star_cmp) == 0) {
                fclose(file);
                initStarData(filename, topology);
                return SUCCESS;
            } else {
                printf("Incorrect syntax detected..stopping.\n");
                fclose(file);
                return FAIL;
            }
        }
    }
}
/*
 int main()
 {
 Topo top;
 initTopo(&top);
 initializeTop(&top);
 }*/

