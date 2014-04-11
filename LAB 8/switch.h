/*
 *switch.h
 * */

/*Things needed 
 * ForwardingTable Container
 * PacketQueue
*/

typedef struct {
   int physid; /* Physical ID */
   struct FWTable * ftable; //Container of forwarding data
   struct PacketQueue * recvPQ; //Container of packets
   struct switchLinks * sLinks; //Container of all connections   
} switchState;

//Initialization Functions

void switchInitState(switchState * sstate, int phys);

//Main run function
void switchMain(switchState * sstate);
void TestIterate(switchState *sstate, switchLinks ** head);
