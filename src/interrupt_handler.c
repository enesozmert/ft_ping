#include "header.h"

void interrupt_handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("\nInterrupt signal received. Terminating...\n");

        if (g_ping != NULL)
        {
            printf("ok ok ok end");
            // Display results
            printf("\n--- %s ping statistics ---\n", g_ping->dest_ip_addr);
            printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.2fms\n",
                   g_ping->result->sent_packets, g_ping->result->received_packets,
                   ((g_ping->result->sent_packets - g_ping->result->received_packets) / (double)g_ping->result->sent_packets) * 100,
                   g_ping->result->rtt);
            free(g_ping);
        }

        exit(0);
    }
}