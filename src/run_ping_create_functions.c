#include "header.h"

void run_ping_create_functions(t_ping *ping, ping_create_func_t funcs[], int num_funcs) {
    for (int i = 0; i < num_funcs; i++) {
        int result = funcs[i](ping);
        if (result > 0) {
            printf("Fonksiyon %d başarılı.\n", i + 1);
        } else {
            printf("Fonksiyon %d başarısız.\n", i + 1);
            exit(-1);
        }
    }
}