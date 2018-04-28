/*
*
*
*           AUTHOR:  AMIT ROY
*           DEPARTMENT OF CSE
*          UNIVERSITY OF DHAKA
*    Topic: DVR Algorithm Implementation
*
*
*/



#include "distance_vector.h"
#include "packet_header.h"
#include "dv_entry.h"
#include "config.h"
#include "node.h"

void sendToSelf(DV &dv, int socketfd, int type, char source = 0, char dest = 0, int dataLength = 0, void *data = 0);
void sendToAll(DV &dv, int socketfd);
void *makePacket(int type, char source, char dest, int dataLength, void *data);
packet_header findpacket_header(void *packet);
void *finddata(void *packet, int length);





int main(int argc, char **argv)
{


    /// COMMAND LINE ERROR DETECTION
    if(argc<3)
    {
        perror("\n\n\tERROR: Invalid Command Line Arguments.\n\tFormat ./dv_routing <router name> <port no> <initialization file>\n\n");
    }



    /// COMMAND LINE DATA PARSING
    routerName = (string)argv[1];
    selfName = routerName[0];
    portNo = (string)argv[2];
    myPortNo = atoi(portNo.c_str());
    fileName = (string)argv[3];



    ///CREATE OWN DISTANCE VECTOR
    DV dv(fileName.c_str(), routerName.c_str());
    vector<node> neighbors = dv.neighbors();



    ///INITAILIZE OWN SOCKET
    int socketfd;
    dv.initializeMyAddress(myPortNo);
    sockaddr_in myaddr = dv.myaddr();
    socklen_t addrlen = sizeof(sockaddr_in);


    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("\n\n\tERROR: Unable to create socket!!!\n");
        return 0;
    }


    ///LOCALHOST and MyPORT Socket Binding
    if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
        perror("bind failed");
        return 0;
    }



    /// distance vector routing
    int pid = fork();
    if (pid == 0)                                   /// send to each neighbor periodically
    {
        while(true)
        {
                                                    /// periodically wake up parent process
            sendToSelf(dv, socketfd, TYPE_WAKEUP);
            sleep(5);
        }
    }
    else if (pid < 0)
    {
        perror("FORK UNSUCESSFUL!!!");
        return 0;
    }
    else                                            /// listen for advertisements
    {
        void *rcvbuf = malloc(BUFFER_SIZE);
        sockaddr_in remaddr;


        while(true)
        {
            memset(rcvbuf, 0, BUFFER_SIZE);
            int recvlen = recvfrom(socketfd, rcvbuf, BUFFER_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);



            packet_header h = findpacket_header(rcvbuf);
            void *data = finddata(rcvbuf, h.length);



            if(h.type==TYPE_BROADCAST)
            {


                dv_entry entries[MAX_ROUTERS];
                memcpy((void*)entries, data, h.length);


                for (int i = 0; i < neighbors.size(); i++)
                {
                    if (neighbors[i].name == h.source)
                        dv.startTimer(neighbors[i]);
                }


                dv.update(data, h.source);

            }

            else if(h.type==TYPE_WAKEUP)        /// perform periodic tasks
            {

                for (int i = 0; i < neighbors.size(); i++)
                {
                    node curNeighbor = neighbors[i];
                    if ((dv.getEntries()[dv.getIndexOf(curNeighbor.name)].cost() != -1) && dv.timerExpired(neighbors[i]))
                    {
                        sendToSelf(dv, socketfd, TYPE_NODE_CLOSED, dv.getSelfName(), neighbors[i].name, dv.getSize() / sizeof(dv_entry) - 2);
                    }
                }
                sendToAll(dv, socketfd);
            }

            else if(h.type==TYPE_NODE_CLOSED)
            {
                int hopcount = (int)h.length - 1;
                dv.reset(h.dest);
                if (hopcount > 0)
                {
                    void *forwardPacket = makePacket(TYPE_NODE_CLOSED, dv.getSelfName(), h.dest, hopcount, (void*)0);
                    for (int i = 0; i < neighbors.size(); i++)
                    {
                        if (neighbors[i].name != h.source)
                            sendto(socketfd, forwardPacket, sizeof(packet_header), 0, (struct sockaddr *)&neighbors[i].addr, sizeof(sockaddr_in));
                    }
                }
            }
        }

        free(rcvbuf);
    }

}

/// CREATE PACKET WITH HEADERS AND DATA
void *makePacket(int type, char source, char dest, int dataLength, void *data)
{
    int allocateddataLength = dataLength;
    if ((type != TYPE_PAYLOAD) && (type != TYPE_BROADCAST))
        allocateddataLength = 0;

    /// CREATE EMPTY PACKET
    void *packet = malloc(sizeof(packet_header)+allocateddataLength);

    /// CREATE HEADER OF PACKET
    packet_header h;
    h.type = type;
    h.source = source;
    h.dest = dest;
    h.length = dataLength;

    /// FILL THE PACKET WITH DATA
    memcpy(packet, (void*)&h, sizeof(packet_header));
    memcpy((void*)((char*)packet+sizeof(packet_header)), data, allocateddataLength);

    return packet;
}


/// EXTRACT DATA FROM PACKET
void *finddata(void *packet, int length)
{
    void *data = malloc(length);
    memcpy(data, (void*)((char*)packet+sizeof(packet_header)), length);
    return data;
}


/// EXTRACT HEADER FROM PACKET
packet_header findpacket_header(void *packet)
{
    packet_header h;
    memcpy((void*)&h, packet, sizeof(packet_header));
    return h;
}

/// WAKE UP PERIODICALLY TO MULTICAST ADVERTISEMENT
void sendToSelf(DV &dv, int socketfd, int type, char source, char dest, int dataLength, void *data)
{
    void *sendPacket = makePacket(type, source, dest, dataLength, data);
    sockaddr_in destAddr = dv.myaddr();
    sendto(socketfd, sendPacket, sizeof(packet_header), 0, (struct sockaddr *)&destAddr, sizeof(sockaddr_in));
    free(sendPacket);

}


/// BROADCAST ADVERTISEMENT TO ALL NEIGHBOURS
void sendToAll(DV &dv, int socketfd)
{
    vector<node> neighbors = dv.neighbors();
    for (int i = 0; i < neighbors.size(); i++)
    {
        void *sendPacket = makePacket(TYPE_BROADCAST, dv.getSelfName(), neighbors[i].name, dv.getSize(), (void*)dv.getEntries());
        sendto(socketfd, sendPacket, sizeof(packet_header) + dv.getSize(), 0, (struct sockaddr *)&neighbors[i].addr, sizeof(sockaddr_in));
        free(sendPacket);
    }

}
