#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

void TOUPPER(char * arr){
    for(int i=0; i<(int)strlen(arr); i++){
        arr[i] = toupper((unsigned char)arr[i]);
    }
}

void get_input(char *args[], int input[][2], int *n, int *size, int *policy) 
{
    FILE *input_file = fopen(args[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        fflush(stdout);
        exit(0);
    }

    parse_file(input_file, input, n, size);
    fclose(input_file);
  
    TOUPPER(args[2]);
  
    if((strcmp(args[2],"-F") == 0) || (strcmp(args[2],"-FIFO") == 0))
        *policy = 1;
    else if((strcmp(args[2],"-B") == 0) || (strcmp(args[2],"-BESTFIT") == 0))
        *policy = 2;
    else if((strcmp(args[2],"-W") == 0) || (strcmp(args[2],"-WORSTFIT") == 0))
        *policy = 3;
    else {
       printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
       exit(1);
    }
}

list_t* coalese_memory(list_t * list){
    list_t *temp_list = list_alloc();
    block_t *blk;
  
    // sort the list in ascending order by address
    while((blk = list_remove_from_front(list)) != NULL) {  
        list_add_ascending_by_address(temp_list, blk);
    }
  
    // try to combine physically adjacent blocks
    list_coalese_nodes(temp_list);
        
    return temp_list;
}

void allocate_memory(list_t * freelist, list_t * alloclist, int pid, int blocksize, int policy) {
    // 1. Check if a node in the freelist has enough space
    int idx = list_get_index_of_by_Size(freelist, blocksize);
    if(idx == -1) {
        // Updated error message to match reference output format:
        printf("Error: Memory Allocation %d blocks\n", blocksize);
        return;
    }

    // Remove that block from free list
    block_t *freeblk = list_remove_at_index(freelist, idx);

    // Original end
    int original_end = freeblk->end;

    // 3. set blk.pid = pid
    freeblk->pid = pid;

    // 4. set blk.end = blk.start + blocksize - 1
    freeblk->end = freeblk->start + blocksize - 1;

    // 5. add the blk to alloc_list in ascending order by address
    list_add_ascending_by_address(alloclist, freeblk);

    // 6. Deal with the fragment
    int fragment_start = freeblk->end + 1;
    if(fragment_start <= original_end) {
        block_t *fragment = malloc(sizeof(block_t));
        fragment->pid = 0;
        fragment->start = fragment_start;
        fragment->end = original_end;

        // add fragment back to the free list based on policy
        if(policy == 1) { // FIRST FIT = FIFO (just add to back)
            list_add_to_back(freelist, fragment);
        } else if(policy == 2) { // BEST FIT = ascending by blocksize
            list_add_ascending_by_blocksize(freelist, fragment);
        } else if(policy == 3) { // WORST FIT = descending by blocksize
            list_add_descending_by_blocksize(freelist, fragment);
        }
    }
}

void deallocate_memory(list_t * alloclist, list_t * freelist, int pid, int policy) {
    int idx = list_get_index_of_by_Pid(alloclist, pid);
    if(idx == -1) {
        printf("Error: Can't locate Memory Used by PID: %d\n", pid);
        return;
    }

    // Remove block from alloc list
    block_t *blk = list_remove_at_index(alloclist, idx);

    // Set pid back to 0
    blk->pid = 0;

    // Insert into free list based on policy
    if(policy == 1) { // FIFO
        list_add_to_back(freelist, blk);
    } else if(policy == 2) { // BEST FIT
        list_add_ascending_by_blocksize(freelist, blk);
    } else if(policy == 3) { // WORST FIT
        list_add_descending_by_blocksize(freelist, blk);
    }
}

void print_list(list_t * list, char * message){
    node_t *current = list->head;
    block_t *blk;
    int i = 0;
  
    printf("%s:\n", message);
  
    while(current != NULL){
        blk = current->blk;
        printf("Block %d:\t START: %d\t END: %d", i, blk->start, blk->end);
      
        if(blk->pid != 0)
            printf("\t PID: %d\n", blk->pid);
        else  
            printf("\n");
      
        current = current->next;
        i += 1;
    }
}

/* DO NOT MODIFY */
int main(int argc, char *argv[]) 
{
    int PARTITION_SIZE, inputdata[200][2], N = 0, Memory_Mgt_Policy;
  
    list_t *FREE_LIST = list_alloc();   // list that holds all free blocks (PID is always zero)
    list_t *ALLOC_LIST = list_alloc();  // list that holds all allocated blocks
    int i;
  
    if(argc != 3) {
        printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
        exit(1);
    }
  
    get_input(argv, inputdata, &N, &PARTITION_SIZE, &Memory_Mgt_Policy);
  
    // Allocated the initial partition of size PARTITION_SIZE
    block_t * partition = malloc(sizeof(block_t));   // create the partition meta data
    partition->pid = 0;                             
    partition->start = 0;
    partition->end = PARTITION_SIZE + partition->start - 1;
                                   
    list_add_to_front(FREE_LIST, partition);          // add partition to free list
                                   
    for(i = 0; i < N; i++) // loop through all the input data and simulate
    {
        printf("************************\n");
        if(inputdata[i][0] != -99999 && inputdata[i][0] > 0) {
            printf("ALLOCATE: %d FROM PID: %d\n", inputdata[i][1], inputdata[i][0]);
            allocate_memory(FREE_LIST, ALLOC_LIST, inputdata[i][0], inputdata[i][1], Memory_Mgt_Policy);
        }
        else if (inputdata[i][0] != -99999 && inputdata[i][0] < 0) {
            printf("DEALLOCATE MEM: PID %d\n", abs(inputdata[i][0]));
            deallocate_memory(ALLOC_LIST, FREE_LIST, abs(inputdata[i][0]), Memory_Mgt_Policy);
        }
        else {
            printf("COALESCE/COMPACT\n");
            FREE_LIST = coalese_memory(FREE_LIST);
        }   
     
        printf("************************\n");
        print_list(FREE_LIST, "Free Memory");
        print_list(ALLOC_LIST,"\nAllocated Memory");
        printf("\n\n");
    }
  
    list_free(FREE_LIST);
    list_free(ALLOC_LIST);
  
    return 0;
}