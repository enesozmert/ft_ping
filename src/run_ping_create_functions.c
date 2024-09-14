#include "header.h"

void run_ping_create_functions(t_ping *ping, t_ping_func_entry *funcs, int num_funcs) {
    for (int i = 0; i < num_funcs; i++) {
        int result = funcs[i].func(ping);
        if (result > 0) {
            printf("Fonksiyon %s başarılı.\n", funcs[i].func_name);
        } else {
            printf("Fonksiyon %s başarısız.\n", funcs[i].func_name);
            exit(-1);
        }
    }
}
