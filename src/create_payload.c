#include "header.h"

int create_payload(t_ping *ping){

    const char* data = "enes ozmert 1121092387432784732894783279";
    uint32_t length = strlen(data);
    
    // Bellek tahsisi yapılıyor ve boyut set ediliyor
    ping->payload->payload = (char *)malloc(length + 1);  // Null terminator için ekstra yer
    if (ping->payload->payload == NULL) {
        return -1; // Bellek tahsisi başarısızsa hata döndür
    }

    memcpy(ping->payload->payload, data, length + 1); // Null terminator dahil kopyalama
    ping->payload->payload_size = length;



    return 1;
}
