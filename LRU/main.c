#include <stdlib.h>
#include <stdio.h>
#include "simlru.h"


int main()
{
    int num;
    PROC *proc_list;

    printf("Simulation for Memory Management\n");
    printf("------------------------------\n");

    printf("Bacis information:\n");
    printf("mem_size: %d  page_size: %d  page_num: %d  rss: %d\n", MEM_SIZE, PAGE_SIZE, PAGE_NUM, RSS);
    printf("proc_max: %d  proc_size_max: %d\n", PROC_MAX, PROC_SIZE_MAX);

    printf("------------------------------\n");
    while (1) {
        printf("Number of process: ");
        scanf("%d", &num);
        if (num > PROC_MAX)
            printf("Should less than %d, try again\n", PROC_MAX);
        else
            break;
    }
    proc_list = create_proc(num);
    if (proc_list == NULL) {
        printf("No process created\n");
        exit(1);
    }
    printf("%d process created\n", num);
    alloc_page(proc_list);
    printf("%d page allocated\n", page_used);
    alloc_order(proc_list);
    printf("Access orders allocated\n");

    printf("------------------------------\n");
    print_proc(proc_list);
    printf("page_used: %d\n", page_used);

    printf("------------------------------\n");
    printf("Simulation begin\n");
    printf("------------------------------\n");
    simulate(proc_list);

    return 0;
}
