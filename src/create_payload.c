#include "header.h"

int create_payload(t_ping *ping){

    const char* data = "ft_ping-1453-1453-1453-1453-1453-1453-1453-1453--ft_ping";
    uint32_t length = strlen(data);

    ping->payload->payload = (char *)malloc(length + 1);
    if (ping->payload->payload == NULL) {
        return -1;
    }

    memcpy(ping->payload->payload, data, length + 1);
    ping->payload->payload_size = length;

    return 1;
}
