#define ABSOLUTE_MAX 100

typedef struct {
   int numlinks;
   int numhosts;
   int numswitch;
   int type;
   int link[ABSOLUTE_MAX];
} Topo;

int initializeTop(Topo *topology);
void initTopo(Topo *topology);

