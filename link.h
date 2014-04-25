/* Definitions and prototypes for the link (link.c)
 */

#define NUMLINKS 16

enum LinkType {UNIPIPE, SOCKET}; /* UNIPIPE = unidirectional pipe
                          * We can add more types later
                          */

enum UniPipeType {BLOCKING, NONBLOCKING};


//PhysIdSrc is the SOURCE of connection
//Pipes are unidirection (one direction, so there is a source)
//PhysIdDst are the destination of connection
//So if your linkIN is say to the switch, the dst is the id of the switch
//and the src is the hostID

typedef struct {
   enum UniPipeType pipeType;
   int         fd[2];
   int         physIdSrc;
   int	       physIdDst;
} UniPipeInfo;

typedef struct {  /* Has all the information to implement a link */
   int linkID;             /* ID for the link */
   enum LinkType linkType; /* The type of link */
   UniPipeInfo uniPipeInfo; /* If the link is a pipe, this is the information */
} LinkInfo;


typedef struct {
   int numlinks;
   LinkInfo link[NUMLINKS];
} linkArrayType;


typedef struct {
   int port;
} Socket;


/* Transmit the packet in pbuff on the link */
int linkSend(LinkInfo * link, packetBuffer * pbuff);

/* Downloads a packet from the link into pbuff */
int linkReceive(LinkInfo * link, packetBuffer * pbuff);

/* Closes a link */
int linkClear(LinkInfo * link);

/* Initializes a link */
int linkCreate(LinkInfo * link);

