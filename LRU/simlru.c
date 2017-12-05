#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "simlru.h"


PROC *create_proc(int num)
{
    int i;
    PROC *head, *p, *q;

    head = p = q = (PROC *)malloc(sizeof(PROC));
    srand((unsigned int)time(NULL));
    for (i = 0; i < num; ++i) {
        p->pid = pid_used++;  // increase available PID after allocation
        p->size = rand() % PROC_SIZE_MAX;
        q = p;
        q->next = p = (PROC *)malloc(sizeof(PROC));
    }
    q->next = NULL;
    free(p);

    // if no process created
    if (head == p)
        return NULL;
    return head;
}


void print_proc(PROC *head)
{
#ifdef PRINT_PAGE
    PAGE *page_head;
    PAGE_ORDER *order_head;
#endif
    while (head != NULL) {
#ifdef PRINT_PAGE
        printf("PID: %d  size: %d  n_page: %d\n", head->pid, head->size, head->n_page);
        printf("FLAGS: ");
        page_head = head->i_page;
        while (page_head != NULL) {
            printf("%d ", page_head->flag);
            page_head = page_head->next;
        }
        printf("\n");

        printf("ORDERS: ");
        order_head = head->order;
        while (order_head != NULL) {
            printf("%d ", order_head->flag);
            order_head = order_head->next;
        }
        printf("\n\n");
#else
        printf("PID: %5d  size: %d\n", head->pid, head->size);
#endif
        head = head->next;
    }
}


void print_rss(int pid)
{
    int i;

    printf("PID: %d  RS: ", pid);
    for (i = 0; i < RSS; ++i) {
        if (memory[pid].last_order[i] == 0)
            break;
        printf("%3d ", memory[pid].pages[i]->flag);
    }
    printf("\n");
#ifdef PRINT_LRU_RECORD
    printf("RECORD: ");
    for (i = 0; i < RSS; ++i) {
        if (memory[pid].last_order[i] == 0)
            break;
        printf("%3d ", memory[pid].last_order[i]);
    }
    printf("\n\n");
#endif
}


int alloc_page(PROC *head)
{
    int i;
    int size;
    int n_page;
    PAGE *page_head, *p, *q;

    while (head != NULL) {
        size = head->size;
        n_page = (size % PAGE_SIZE == 0) ? (size / PAGE_SIZE): (size / PAGE_SIZE + 1);

        page_head = p = q = (PAGE *)malloc(sizeof(PAGE));
        for (i = 0; i < n_page; ++i) {
            p->flag = i;  // page flag base on the number of pages
            q = p;
            q->next = p = (PAGE *)malloc(sizeof(PAGE));
        }
        q->next = NULL;
        free(p);

        if ((page_used += n_page) > PAGE_NUM) {
            printf("Resource exhausted: out of memory\n");
            exit(1);
        }

        head->n_page = n_page;
        head->i_page = page_head;
        head = head->next;
    }
    return page_used;
}


int alloc_order(PROC *head)
{
    int i;
    int n_order;
    PAGE_ORDER *order_head, *p, *q;

    srand((unsigned int)time(NULL));
    while (head != NULL) {
        n_order = head->n_page;  // orders base on the number of pages

        order_head = p = q = (PAGE_ORDER *)malloc(sizeof(PAGE_ORDER));
        for (i = 0; i < n_order; ++i) {
            p->flag = rand() % head->n_page;
            q = p;
            q->next = p = (PAGE_ORDER *)malloc(sizeof(PAGE_ORDER));
        }
        q->next = NULL;
        free(p);

        order_used += n_order;
        head->order = order_head;
        head = head->next;
    }
    return order_used;
}


PAGE *get_order_page(PROC *proc, PAGE_ORDER *order)
{
    PAGE *head = proc->i_page;

    while (head != NULL) {
        if (head->flag == order->flag)
            return head;
        head = head->next;
    }
    return NULL;
}


int get_order_rs(PAGE *RS[], PAGE_ORDER *order)
{
    int i;
    int res = -1;

    for (i = 0; i < RSS; ++i) {
        if (!RS[i])
            break;  // current page is not allocated in RS
        else if (RS[i]->flag == order->flag) {
            res = i;
            break;
        }
    }
    return res;
}


int get_oldest_rs(int pid)
{
    int i;
    int res = 0;

    for (i = 1; i < RSS; ++i) {
        if (memory[pid].last_order[i] < memory[pid].last_order[res])
            res = i;
    }
    return res;
}


int LRU(PROC *proc)
{
    int index;
    int pid = proc->pid;
    static int order_rec[PROC_MAX] = {0};  // count for page access for each process
    PAGE_ORDER *order = proc->order;
    PAGE *page;

    // done for current process
    if (order == NULL)
        return LRU_DONE;

    page = get_order_page(proc, order);

    // hit the page in RS
    if ((index = get_order_rs(memory[pid].pages, order)) != -1) {
        ++order_rec[pid];
        memory[pid].last_order[index] = order_rec[pid];
        proc->order = proc->order->next;
#ifdef PRINT_LRU
        print_rss(pid);
#endif
        return LRU_HIT;
    }

    // page fault
    else {
        index = get_oldest_rs(pid);
        ++order_rec[pid];
        memory[pid].pages[index] = page;
        memory[pid].last_order[index] = order_rec[pid];
        proc->order = proc->order->next;
#ifdef PRINT_LRU
        print_rss(pid);
#endif
        return LRU_FAULT;
    }
}


void simulate(PROC *head)
{
    int pid;
    int status;
    int n_order;
    int n_done = 0;
    int n_page_fault[PROC_MAX] = {0};
    int n_page_access[PROC_MAX] = {0};
    float miss_page_rate;
    float mpr_avg = 0;
    PROC *cur_proc;

    while (n_done != pid_used) {
        cur_proc = head;
        n_done = 0;
        while (cur_proc != NULL) {
            pid = cur_proc->pid;
            status = LRU(cur_proc);
            if (status == LRU_HIT) {
                ++n_page_access[pid];
            }
            else if (status == LRU_FAULT) {
                ++n_page_fault[pid];
                ++n_page_access[pid];
            }
            else if (status == LRU_DONE) {
                ++n_done;
            }
            cur_proc = cur_proc->next;
        }
    }
#ifdef PRINT_LRU
    printf("------------------------------\n");
#endif
    cur_proc = head;
    while (cur_proc != NULL) {
        pid = cur_proc->pid;
        n_order = cur_proc->n_page;  // orders base on the number of pages
        miss_page_rate = (float)n_page_fault[pid] / (float)n_page_access[pid];
        mpr_avg += miss_page_rate;
        printf("PID: %d  miss_page_rate(%d / %d): %f\n", pid, n_page_fault[pid], n_page_access[pid], miss_page_rate);
        printf("RSS: %d  n_order: %d  n_page_fault: %d  n_page_access: %d\n\n", RSS, n_order, n_page_fault[pid], n_page_access[pid]);
        cur_proc = cur_proc->next;
    }
    mpr_avg /= pid_used;
    printf("AVG(miss_page_rate): %f\n", mpr_avg);
}