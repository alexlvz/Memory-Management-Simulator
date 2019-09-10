#ifndef MEM_SIM_H_INCLUDED
#define MEM_SIM_H_INCLUDED
#define PAGE_SIZE 4
#define MEMORY_SIZE 24
#define SWAP_SIZE 80
#define NUM_OF_PAGES 20

typedef struct page_descriptor
{
    int valid;
    int dirty;
    int in_backing_store;
    int frame;
}page_descriptor;

typedef struct sim_database
{
    page_descriptor *page_table;
    int swapfile_fd;
    int program_fd;
    char main_memory[MEMORY_SIZE];
    int fifo[MEMORY_SIZE/PAGE_SIZE]; //HOLDS THE ARRANGEMENT OF FRAMES TO BE SWAPPED
    int mainMemory_free_index; //NEXT FREE BLOCK IN MEMORY
    int mainMemoryBlocks; //NUM OF BLOCKS CURRENTLY IN MEMORY
    int textSize; //TEXT PART SIZE
    int bssHeapStackSize; //CODE PARY SIZE
    char swapName[100]; //SWAP FILE NAME
    char execName[100]; //CODE FILE NAME

}sim_database;

sim_database *init_system(char exe_file_name[], char swap_file_name[], int text_size, int bss_heap_stack_size);
char load(struct sim_database *mem_sim,int address);
void store(struct sim_database *mem_sim, int address, char value);
void print_memory(struct sim_database *mem_sim);
void print_swap(struct sim_database *mem_sim);
void print_page_table(struct sim_database *mem_sim);
void shiftArray(int *arr,int size);
void updatePageValues(page_descriptor *pageTable, int validd, int dirtyy, int inBackingStore, int memoryFrame, int pageLocationInTable);
void storePageInMemory(char mainMemory[], int memoryFrame, char pageData[]);
void storeValueInMemory(char mainMemory[], int memoryFrame, int offset, char value);
char *createNewPageData(char value, int offset);
char *getPageFromMemory(int memoryFrame, char mainMemory[]);
void storePageInSwap(int swapfile_fd, char pageData[], int swapIndex, char swapName[]);
char *getPageFromSwap(int swapfile_fd, int swapIndex , char swapName[]);
char *getPageFromExecFile(int swapfile_fd, int execFileIndex, char execName[]);
void clear_system(struct sim_database *mem_sim);
void updateValues(struct sim_database * mem_sim , int pageToSwap , int frameToWriteInMemory, char *data, int pageLocationInTable);

#endif // MEM_SIM_H_INCLUDED
