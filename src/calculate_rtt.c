#include "header.h"

double calculate_rtt(struct timeval *start, struct timeval *end) {
    double rtt = (end->tv_sec - start->tv_sec) * 1000.0; // saniyeyi milisaniyeye çevir
    rtt += (end->tv_usec - start->tv_usec) / 1000.0;     // mikro saniyeyi milisaniyeye çevir
    return rtt;
}
