#include <stdio.h>   // printf, fprintf, stderr, etc. için
#include <stdlib.h>  // exit için
#include <unistd.h>  // getopt için

void parse_args(int argc, char *argv[], int *verbose_flag)
{
    int opt;
    while ((opt = getopt(argc, argv, "v?")) != -1)
    {
        switch (opt)
        {
        case 'v':
            *verbose_flag = 1;
            break;
        case '?':
            printf("Usage: %s [-v] [-?] <hostname>\n", argv[0]);
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "Usage: %s [-v] [-?] <hostname>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}