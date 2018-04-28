#ifndef PACKETHEADER_H_
#define PACKETHEADER_H_




struct packet_header
{
    int type;
    char source;
    char dest;
    int length;
};


#endif // PACKETHEADER_H_
