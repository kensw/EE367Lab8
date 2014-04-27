#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "top.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"
#include "net.h"
#include "switchlink.h"
#include "switch.h"

#ifdef debug
#include "debug.h"
#endif

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
/* It also indicates that the broadcast address */
#define MAXBUFFER 1000
#define PIPEWRITE 1
#define PIPEREAD  0
#define NODECOUNT 3

int main()
{
    hostState hstate;             /* The host's state */
    switchState sstate;
    linkArrayType linkArray;
    manLinkArrayType manLinkArray;
    Topo top;
    initTopo(&top);
    initializeTop(&top);
    /* Intialie Switch Links to NULL */
    sstate.sLinks = NULL;
    
#ifdef debug
    debuginit();
#endif
    
    pid_t pid;  /* Process id */
    int physid; /* Physical ID of host */
    int i;
    int k;
    
    /*
     * Create nonblocking (pipes) between manager and hosts
     * assuming that hosts have physical IDs 0, 1, ...
     */
    manLinkArray.numlinks = top.numhosts;
    netCreateConnections(& manLinkArray);
#ifdef debug
    printf("Created Nonblocking pipes\n");
#endif
    
    /* Create links between nodes but not setting their end nodes */
    linkArray.numlinks = top.numlinks;
    netCreateLinks(& linkArray);
#ifdef debug
    printf("Created links between nodes\n");
#endif
    
    /* Set the end nodes of the links */
    netSetNetworkTopology(&top, & linkArray);
#ifdef debug
    printf("Set the end nodes of the links\n");
    Newdebugmessage("Manager PID: ", getpid(), 0, -42);
#endif
    
    
    /* Create nodes and spawn their own processes, one process per node */
    for (physid = 0; physid < top.numhosts; physid++) {
        pid = fork();
        if (pid == -1) {
            printf("Error:  the fork() failed\n");
            return -1;
        }
        else if (pid == 0) { /* The child process -- a host node */
            hostInit(&hstate, physid); /* Initialize host's state */
            
            /* Initialize the connection to the manager */
            hstate.manLink = manLinkArray.link[physid];
            /*
             * Close all connections not connect to the host
             * Also close the manager's side of connections to host
             */
            netCloseConnections(& manLinkArray, physid);
            
            /* Initialize the host's incident communication links */
            
            k = netHostOutLink(&linkArray, physid); /* Host's outgoing link */
            hstate.linkout = linkArray.link[k];
            
            k = netHostInLink(&linkArray, physid); /* Host's incoming link */
            hstate.linkin = linkArray.link[k];
            
            /* Close all other links -- not connected to the host */
            netCloseHostOtherLinks(& linkArray, physid);
            
            /* Go to the main loop of the host node */
            hostMain(&hstate);
        }
    }
#ifdef debug
    printf("Created Hosts and spawned their own processes\n");
#endif
    
    int sw_end_addr = top.numhosts + top.numswitch;
    for(physid = top.numhosts; physid < sw_end_addr; physid++){
        pid = fork();
        if(pid == -1) {
            printf("Error occured forking for switch \n");
        } else if (pid == 0) {
            switchInitState(&sstate, physid);
            sstate.sLinks = getswitchLinks(&linkArray, physid, sstate.sLinks);
            //      TestIterate(&sstate, &sstate.sLinks);
            netCloseHostOtherLinks(&linkArray, physid);
            switchMain(&sstate);
        }
    }
#ifdef debug
    printf("Created Switches and spawned their own processes\n");
#endif
    
    /*
     * The manager is connected to the hosts and doesn't
     * need the links between nodes
     */
    
    /* Close all links between nodes */
    netCloseLinks(&linkArray);
    
    /* Close the host's side of connections between a host and manager */
    netCloseManConnections(&manLinkArray);
    
    /* Go to main loop for the manager */
    manMain(& manLinkArray);
    
    /*
     * We reach here if the user types the "q" (quit) command.
     * Now if we don't do anything, the child processes will continue even
     * after we terminate the parent process.  That's because these
     * child proceses are running an infinite loop and do not exit
     * properly.  Since they have no parent, and no way of controlling
     * them, they are called "zombie" processes.  Actually, to get rid
     * of them you would list your processes using the LINUX command
     * "ps -x".  Then kill them one by one using the "kill" command.
     * To use the kill the command just type "kill" and the process ID (PID).
     *
     * The following system call will kill all the children processes, so
     * that saves us some manual labor
     */
    kill(0, SIGKILL); /* Kill all processes */
}

int lengthpbuff(packetBuffer pbj)
{
    int i = 0;
    int length = 0;
    do{
        length = length + pbj.payloads[i].length;
    } while (pbj.payloads[i].type != 1);
    return length;
}

