#include "header.h"

int ping_create_allocates(t_ping **ping)
{
    *ping = malloc(sizeof(t_ping));  // ping işaretçisinin içeriğini ayırıyoruz
    if (!*ping)
    {
        perror("Failed to allocate memory for t_ping");
        return -1;
    }

    (*ping)->result = malloc(sizeof(t_ping_result));  // ping işaretçisinin içeriğini dereference edip ayırıyoruz
    if (!(*ping)->result)
    {
        perror("Failed to allocate memory for t_ping_result");
        free(*ping);  // Ayırılan belleği serbest bırakıyoruz
        return -1;
    }

    (*ping)->target_addr = (struct sockaddr_ll *)malloc(sizeof(struct sockaddr_ll));
    if (!(*ping)->target_addr)
    {
        perror("Memory Allocation Error for target_addr!");
        free((*ping)->result);  // Dinamik bellekleri serbest bırakıyoruz
        free(*ping);
        return -1;
    }

    (*ping)->ifreq = (struct ifreq *)malloc(sizeof(struct ifreq));
    if (!(*ping)->ifreq)
    {
        perror("Failed to allocate memory for ifreq");
        close((*ping)->sock_fd);
        free((*ping)->target_addr);
        free((*ping)->result);
        free(*ping);
        return -1;
    }

    (*ping)->packet = (t_packet *)malloc(sizeof(t_packet));
    if (!(*ping)->packet)
    {
        perror("Failed to allocate memory for t_packet");
        close((*ping)->sock_fd);
        free((*ping)->ifreq);
        free((*ping)->target_addr);
        free((*ping)->result);
        free(*ping);
        return -1;
    }

    (*ping)->payload = (t_payload *)malloc(sizeof(t_payload));
    if (!(*ping)->payload)
    {
        perror("Failed to allocate memory for t_payload");
        close((*ping)->sock_fd);
        free((*ping)->packet);
        free((*ping)->ifreq);
        free((*ping)->target_addr);
        free((*ping)->result);
        free(*ping);
        return -1;
    }

    return 1;
}
