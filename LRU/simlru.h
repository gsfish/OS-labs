#ifndef _SIMLRU_H
#define _SIMLRU_H

#define MEM_SIZE (1024 * 1024 * 1024)    // size of vailable memory (bytes)
#define PROC_MAX 10                      // max number of process
#define PROC_SIZE_MAX (1024 * 1024)      // max size of single process (bytes)
#define PAGE_SIZE (4 * 1024)             // size of each page (bytes)
#define PAGE_NUM (MEM_SIZE / PAGE_SIZE)  // number of pages
#define RSS 10                           // resident set size

#define LRU_DONE 0
#define LRU_HIT 1
#define LRU_FAULT 2

// comment if you want simple output
#define PRINT_PAGE
#define PRINT_LRU
// #define PRINT_LRU_RECORD


// PAGE: content for each page
// PAGE_ORDER: access order of pages for each process
typedef struct _page_info {
    int flag;
    struct _page_info *next;
} PAGE, PAGE_ORDER;


// information for each process
typedef struct _proc_info {
    int pid;            // start from pid_used
    int size;           // size of image (bytes)
    int n_page;         // cost of pages
    PAGE *i_page;       // pages for image
    PAGE_ORDER *order;  // access orders for this process
    struct _proc_info *next;
} PROC;


// virtual memory for each process
typedef struct _virtual_mem {
    int last_order[RSS];  // count orders for each pages in RS
    PAGE *pages[RSS];     // pages in RS for image
} VMEM;


// create proc_list for specified num
PROC *create_proc(int num);
// print informations for each process in proc_list
void print_proc(PROC *proc_list);
// print contents in RS for each process
void print_rss(int pid);
// allocate pages for each process based on the size in proc_list
int alloc_page(PROC *proc_list);
// create random access orders
int alloc_order(PROC *proc_list);
// simulate LRU memory allocation algorithm
int LRU(PROC *proc);
// get the original page of process for specified page order
PAGE *get_order_page(PROC *proc, PAGE_ORDER *order);
// get the index of order in RS, return -1 if not match
int get_order_rs(PAGE *rss[], PAGE_ORDER *order);
// get the oldest page in RS (based on VMEM.last_order[index])
int get_oldest_rs(int pid);
// simulate the whole progress of virtual memory management
void simulate(PROC *proc_list);


static int pid_used = 0;
static int page_used = 0;
static int order_used = 0;
static VMEM memory[PROC_MAX] = {0};

#endif