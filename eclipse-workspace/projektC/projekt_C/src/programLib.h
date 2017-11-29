#ifndef PROGRAMLIB_H_
#define PROGRAMLIB_H_


//linked list struct
struct dnode
{
    struct dnode *prev;
    char  *datagram;
    struct dnode *next;
};


int SendPacket ( char *interface );
int Window(int *count, char *interface);
void PrintInterfaces();
void AddToList(int *count, char *dtgr);
void DisplayList();
void DeleteList ();
#endif /* PROGRAMLIB_H_ */
