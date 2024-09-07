#include "header.h"

unsigned short checksum(void *buffer, int length)
{
    unsigned short *data = buffer;
    unsigned long sum = 0;
    for (sum = 0; length > 1; length -= 2)
    {
        sum += *data++;
    }
    if (length == 1)
    {
        sum += *(unsigned char *)data;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}