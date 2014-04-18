
#define PAYLOAD_LENGTH 200 /* Maximum payload size */
#define NEW_PAYLOAD_L 198


//This new packet will also have a container that is sent in place of the current payload

typedef struct{
	int type;  //0 indicates first or middle packet, 1 indicates last
	int length;
	char message[NEW_PAYLOAD_L]; // or:  NEW_PAYLOAD_L];
} payloadbuff;


typedef struct { /* Packet buffer */
    int srcaddr;  /* Source address */
    int dstaddr;  /* Destination addres */
    int length;   /* Length of packet */
    payloadbuff payloads[10];  /* Payload section */
    int valid;   /* Indicates if the contents is valid */
    int new;     /* Indicates if the contents has been downloaded */
    int sendrcv;
} packetBuffer;

//typedef struct { /* Packet buffer */
//    int srcaddr;  /* Source address */
//    int dstaddr;  /* Destination addres */
//    payload load;  /* Payload section */
//    int valid;   /* Indicates if the contents is valid */
//    int new;     /* Indicates if the contents has been downloaded */
//} myPacketBuffer;




////Old packetBuffer:
//typedef struct { /* Packet buffer */
//    int srcaddr;  /* Source address */
//    int dstaddr;  /* Destination addres */
//    int length;   /* Length of packet */
//    char payload[PAYLOAD_LENGTH + 1];  /* Payload section */
//    int valid;   /* Indicates if the contents is valid */
//    int new;     /* Indicates if the contents has been downloaded */
//    int sendrcv;
//} packetBuffer;
