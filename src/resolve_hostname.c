#include "header.h"

int resolve_hostname(const char *hostname, char *ip_str, size_t ip_str_len) {
    struct addrinfo hints, *res;
    int result;

    // Prepare hints for getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Use IPv4
    hints.ai_socktype = SOCK_RAW; // Raw socket for ping

    // Try to resolve the hostname
    result = getaddrinfo(hostname, NULL, &hints, &res);
    if (result != 0) {
        fprintf(stderr, "ping: %s: %s\n", hostname, gai_strerror(result));
        return 1;
    }

    // Convert the IP address to a string
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, ip_str_len) == NULL) {
        perror("inet_ntop");
        freeaddrinfo(res);
        return 1;
    }

    // Free resources after resolution
    freeaddrinfo(res);
    return 0;
}
