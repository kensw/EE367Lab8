/*
 * switchlink.h
 */

typedef struct switchLinks {
    LinkInfo linkin; //Incoming Link (Receive Data)
    LinkInfo linkout; //Outgoing Link (Send Data)
    struct switchLinks * next;
} switchLinks;

switchLinks * constructLink(LinkInfo in, LinkInfo out);
LinkInfo * linkSearch(switchLinks ** head, int id);
LinkInfo * outputLink(switchLinks ** head, int out_id);
void addLink(switchLinks ** head, switchLinks * newlink);

void InlinkIterate(switchLinks ** head);
switchLinks * getswitchLinks(linkArrayType * linkArray, int switchID, switchLinks * head);
