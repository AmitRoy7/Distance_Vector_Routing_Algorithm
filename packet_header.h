#ifndef PACKETHEADER_H_
#define PACKETHEADER_H_


#include "distance_vector.h"
#include "packet_header.h"
#include "dv_entry.h"
#include "config.h"
#include "node.h"


struct packet_header
{
    int type;
    char source;
    char dest;
    int length;
};




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
void sendToSelf(DV &dv, int socketfd, int type, char source = 0, char dest = 0, int dataLength = 0, void *data = 0)
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


#endif // PACKETHEADER_H_
