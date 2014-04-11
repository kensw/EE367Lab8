#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"
#include "net.h"
#include "switch.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 1000
#define PIPEWRITE 1 
#define PIPEREAD  0

char line[256];
int number_hosts;
int number_links;
int number_switches;

int main(int argc,char *filename[])
{


if (argc != 2){
	printf("Incorrect format! Please use: %s topology.txt\n",filename[0]);
	return 1;
}
//printf("%s recieved filename=%s\n",filename[0],filename[1]);

FILE *file = fopen(filename[1],"r");
int count = 2;
int lineNumber = 0;
if(file != NULL)
{
	//This will keep reading lines until the there is nothing in the file
	while(fgets(line,sizeof line, file) != NULL)
	{
		//Here we want to read the first two lines of the txt file
		//which is our NUMHOSTS and NUMLINKS
		if(count >= lineNumber)
		{
			//set NUMHOSTS
			if(lineNumber == 0)
			{
				char numhosts[256];
		 		strcpy(numhosts,line);
				 number_hosts = atoi(numhosts);
				//printf("%s",line);
				//printf("NUMHOSTS = %d\n",number_hosts);
				lineNumber++;
			}
			//set NUMLINKS
			else if(lineNumber == 1)
			{
				char numlinks[256];
		 		strcpy(numlinks,line);
				number_links = atoi(numlinks);
				//printf("%s",line);
				//printf("NUMLINKS = %d\n",number_links);
				lineNumber++;
			}
			else if(lineNumber == 2)
			{
				char numswitches[256];
		 		strcpy(numswitches,line);
				number_switches = atoi(numswitches);
				//printf("%s",line);
				//printf("NUMSWITCHES = %d\n",number_switches);
				lineNumber++;
			}
		}
		//else
		//{
		//	count++;
		//}
	}
	
	//printf("closing file in main\n");
	fclose(file);
	//printf("closed file in main \n");
}

//printf("This is stored in line %s",line);



hostState hstate;             /* The host's state */
switchState sstate;
linkArrayType linkArray;
manLinkArrayType manLinkArray;

pid_t pid;  /* Process id */
int physid; /* Physical ID of host */
int i;
int k;

/* 
 * Create nonblocking (pipes) between manager and hosts 
 * assuming that hosts have physical IDs 0, 1, ... 
 */

manLinkArray.numlinks = number_hosts;
netCreateConnections(& manLinkArray);

/* Create links between nodes but not setting their end nodes */

linkArray.numlinks = number_links;
netCreateLinks(& linkArray, number_links);

/* Set the end nodes of the links */

netSetNetworkTopology(& linkArray, filename);

/* Create nodes and spawn their own processes, one process per node */ 
//printf("Number of hosts: %d ", number_hosts);
for (physid = 0; physid < number_hosts; physid++) {

   pid = fork();

   if (pid == -1) {
      printf("Error:  the fork() failed\n");
      return;
   }
   else if (pid == 0) { /* The child process -- a host node */

      hostInit(&hstate, physid);              /* Initialize host's state */

      /* Initialize the connection to the manager */ 
      hstate.manLink = manLinkArray.link[physid];

      /* 
       * Close all connections not connect to the host
       * Also close the manager's side of connections to host
       */
      netCloseConnections(& manLinkArray, physid);

      /* Initialize the host's incident communication links */

      //k = netHostOutLink(&linkArray, physid); /* Host's outgoing link */
      hstate.linkout = linkArray.link[number_links-1];

      k = netHostInLink(&linkArray, physid); /* Host's incoming link */
      hstate.linkin = linkArray.link[k];

      /* Close all other links -- not connected to the host */
      //netCloseHostOtherLinks(& linkArray, physid);

      /* Go to the main loop of the host node */
      hostMain(&hstate);
   }  
}

for(;physid <= (number_hosts + number_switches - 1) ; physid++){
	pid = fork();

	if(pid == -1){
	printf("Error: the fork() failed \n");
	return;
	}
	else if(pid ==0){
	switchInit(& sstate);
	switchMain(& linkArray, & sstate, number_links);
	}
	//printf("physid: %d\n",physid);
}

/* Manager */

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




