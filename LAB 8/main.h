
#define PAYLOAD_LENGTH 200 /* Maximum payload size */
#define NEW_PAYLOAD_L 198

typedef struct { /* Packet buffer */
   int srcaddr;  /* Source address */
   int dstaddr;  /* Destination addres */
   int length;   /* Length of packet */
   char payload[PAYLOAD_LENGTH + 1];  /* Payload section */
   int valid;   /* Indicates if the contents is valid */ 
   int new;     /* Indicates if the contents has been downloaded */
   int sendrcv;
} packetBuffer;


//This new packet will also have a container that is sent in place of the current payload

typedef struct{
	int type;  //0 indicates first or middle packet, 1 indicates last
	int length;
	char payload[NEW_PAYLOAD_L+1]; // or:  NEW_PAYLOAD_L];
} payload;

typedef struct { /* Packet buffer */
   int srcaddr;  /* Source address */
   int dstaddr;  /* Destination addres */
   payload load;  /* Payload section */
   int valid;   /* Indicates if the contents is valid */ 
   int new;     /* Indicates if the contents has been downloaded */
} myPacketBuffer;

   


