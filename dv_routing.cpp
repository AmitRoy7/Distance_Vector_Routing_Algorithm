/*
*
*
*              AUTHOR:  AMIT ROY
*              DEPARTMENT OF CSE
*             UNIVERSITY OF DHAKA
*    Topic: Distance Vector Routing Protocol
*
*
*/



#include "distance_vector.h"
#include "packet_header.h"
#include "dv_entry.h"
#include "config.h"
#include "node.h"


void quit(int arg) {

		printf("\n Router is Killed\n\n");
}


int main(int argc, char **argv)
{

    ///FOR TERMINATION USING CTRL-C
    signal(SIGTERM, quit);


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
            sendToSelf(dv, socketfd, TYPE_WAKE_UP);
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



            if(h.type==TYPE_BROADCAST)                      ///UPDATE NEIGHBOUR ROUTERS' ROUTING TABLE
            {                                               ///SEND PACKET TO NEIGHBOUR ROUTER PERIODICALLY


                dv_entry entries[MAX_ROUTERS];
                memcpy((void*)entries, data, h.length);


                for (int i = 0; i < neighbors.size(); i++)
                {
                    if (neighbors[i].name == h.source)
                        dv.startTimer(neighbors[i]);
                }


                dv.update(data, h.source);

            }

            else if(h.type==TYPE_WAKE_UP)                   /// WAKE UP PERIODICALLY
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


