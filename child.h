#define BUFFERSIZE 1000

typedef struct child child;

child* createChild(int pid, int serialNum, char *task);
void insertChild(child **head, child **tail, child *newChild);
int deleteChild(child **head, child **tail, int childNum, int childCount);
void deletePid(child **head, child **tail, int pid, int childCount);

struct child
{
    int pid;
    int serialNum;
    child *next;
    char task[BUFFERSIZE];
};