#ifndef CONFIG_H_
#define CONFIG_H_


#define MAX_ROUTERS 20                  ///MAXIMUM 20 Routers allowed
#define BUFFER_SIZE 2048                ///BUFFERSIZE


char selfName;                          ///SELF ROUTER NAME
int myPortNo;                           ///SELF ROUTER PORT NAME
string routerName,portNo,fileName;      ///COMMAND LINE ARGUMENTS
std::map<int, char> m_nodename;         ///PORT NO to NODE NAME MAPPING


enum type
{
    TYPE_PAYLOAD,TYPE_BROADCAST, TYPE_WAKEUP, TYPE_NODE_CLOSED
};



#endif // CONFIG_H_
