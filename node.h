#ifndef NODE_H_
#define NODE_H_

#include "headers.h"


struct node
{
    sockaddr_in addr;
    timespec startTime;
    char name;
    int portno;
};

#endif // NODE_H_
