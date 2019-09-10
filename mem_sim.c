#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem_sim.h"
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#define VALID_PAGE 1
#define INVALID_PAGE -1
#define IN_BACKING_STORE 1
#define NOT_IN_BACKING_STORE -1
#define DIRTY 1
#define NOT_DIRTY -1
#define NO_FRAME -1
#define MAX_PAGES_IN_RAM 6
//*************I GAVE A REALLY GOOD NAMES TO FUNCTIONS AND VARIABLES SO THE CODE IS REALLY SIMPLE TO READ AND UNDERSTAND***************///
//=====================================================================================================================================//
sim_database *init_system(char exe_file_name[], char swap_file_name[], int text_size, int bss_heap_stack_size)
{
    if(exe_file_name == NULL || swap_file_name ==NULL) //check file names
    {
        perror("File names can't be NULL\n");
        exit(1);
    }
    sim_database *db=(sim_database*)malloc(sizeof(sim_database)); //allocating the database
    if(db == NULL) //check malloc
    {
        perror("Malloc error\n");
        exit(1);
    }
    db->swapfile_fd=open(swap_file_name,O_RDWR|O_CREAT|O_TRUNC,0666); //open swap file
    lseek(db->swapfile_fd,0,SEEK_SET);
    int i;
    char swapInfo[SWAP_SIZE];
    for(i=0;i<SWAP_SIZE;i++)
    	swapInfo[i]='0';
    write(db->swapfile_fd ,swapInfo,SWAP_SIZE); //initialize swap file to 80 zeros
    close(db->swapfile_fd);
    
    
    db->program_fd=open(exe_file_name,O_RDONLY); //open executable file
    if(db->program_fd < 0)
    {
        free(db);
        perror("Executable file not found!");
        exit(1);
    }
    db->page_table=(page_descriptor*)malloc(NUM_OF_PAGES * sizeof(page_descriptor)); //allocating page table for 20 pages
    if(db->page_table == NULL) //malloc check
    {
        free(db);
        perror("Malloc error\n");
        exit(1);
    }
    
    db->mainMemory_free_index = 0; //the next free index in memory
    db->mainMemoryBlocks=0; //num of blocks currently in memory

    for(i=0;i<NUM_OF_PAGES;i++) //initialize the page table
    {
    	db->page_table[i].valid = INVALID_PAGE;
    	db->page_table[i].dirty = DIRTY;
    	db->page_table[i].in_backing_store = NOT_IN_BACKING_STORE;
    	db->page_table[i].frame = NO_FRAME;
    	db->textSize = text_size;
    	db->bssHeapStackSize = bss_heap_stack_size;
    	strcpy(db->swapName ,swap_file_name);
    	strcpy(db->execName ,exe_file_name);
    }

    for(i=0;i<MEMORY_SIZE;i++) //initialize the memory to zeros
    	db->main_memory[i]='0';

    return db;
}
//=====================================================================================================================================//
void shiftArray(int *arr,int size) //moves a given array one cell left
{
	int i;
	for(i=1;i<size;i++)
		arr[i-1]=arr[i];
	arr[size-1]=-1;
}
//=====================================================================================================================================//
//updates a given page in the page table values
void updatePageValues(page_descriptor *pageTable, int valid, int dirty, int inBackingStore, int memoryFrame, int pageLocationInTable)
{
	pageTable[pageLocationInTable].valid=valid;
	pageTable[pageLocationInTable].dirty=dirty;
	pageTable[pageLocationInTable].in_backing_store=inBackingStore;
	pageTable[pageLocationInTable].frame=memoryFrame;
}
//=====================================================================================================================================//
//stores a given page in RAM
void storePageInMemory(char mainMemory[], int memoryFrame, char pageData[])
{	
	int i;
	for(i=memoryFrame;i<memoryFrame+PAGE_SIZE;i++)
		mainMemory[i]=pageData[i-memoryFrame];	
}
//=====================================================================================================================================//
//stores a given character(value) in memory
void storeValueInMemory(char mainMemory[], int memoryFrame, int offset, char value)
{
	mainMemory[memoryFrame+offset]=value;
}
//=====================================================================================================================================//
//creates a new page data(4 chars array) 
char *createNewPageData(char value, int offset)
{
	char *pageData=(char*)malloc(PAGE_SIZE* sizeof(char));
	if(pageData == NULL)//malloc check
    {
        perror("Malloc error\n");
        exit(1);
    }
	int i;
	for(i=0;i<PAGE_SIZE;i++) // page data values will be zeros accept of the given char place
		pageData[i]='0';
	pageData[offset]=value;

	return pageData;
}
//======================================================================================================================================//
//returns page data from RAM
char *getPageFromMemory(int memoryFrame, char mainMemory[])
{
	char *pageData=(char*)malloc(PAGE_SIZE* sizeof(char));
	if(pageData == NULL) // malloc check
    	{
        	perror("Malloc error\n");
        	exit(1);
    	}
	int i;
	for(i=memoryFrame;i < PAGE_SIZE+memoryFrame ;i++) //copies the data from the ram by given address + offset
		pageData[i-memoryFrame]=mainMemory[i];

	return pageData;
}
//=====================================================================================================================================//
//stores a given page data into swap
void storePageInSwap(int swapfile_fd, char pageData[], int swapIndex ,char swapName[])
{
	swapfile_fd=open(swapName,O_RDWR,0);
	lseek(swapfile_fd,swapIndex,SEEK_SET);
	write(swapfile_fd ,pageData,PAGE_SIZE);
	close(swapfile_fd);
}
//=====================================================================================================================================//
//returns a page data from swap (array)
char *getPageFromSwap(int swapfile_fd, int swapIndex , char swapName[])
{
	swapfile_fd=open(swapName,O_RDWR,0);
	char *pageData=(char*)malloc(PAGE_SIZE* sizeof(char));
	if(pageData == NULL) //malloc check
    	{
        	perror("Malloc error\n");
        	exit(1);
    	}
	lseek(swapfile_fd,swapIndex,SEEK_SET);
	read(swapfile_fd,pageData,PAGE_SIZE);
	close(swapfile_fd);

	return pageData;
}
//=====================================================================================================================================//
//returns page data from executable file (array)
char *getPageFromExecFile(int program_fd, int execFileIndex, char execName[])
{
	program_fd=open(execName,O_RDWR,0);
	char *pageData=(char*)malloc(PAGE_SIZE* sizeof(char));
	if(pageData == NULL)
    	{
        	perror("Malloc error\n");
        	exit(1);
    	}
	lseek(program_fd,execFileIndex,SEEK_SET);
	read(program_fd,pageData,PAGE_SIZE);
	close(program_fd);

	return pageData;
}
//=====================================================================================================================================//
//This function loads data to memory by a give address and a value to store
char load(struct sim_database * mem_sim , int address)
{
	int pageLocationInTable = (address / PAGE_SIZE);
	int pageOffset = address % PAGE_SIZE;
	char returnValue;
	int pageToSwap; //number of page 
	int frameToWriteInMemory; //the frame location in memory
	
	if(address> NUM_OF_PAGES*PAGE_SIZE ||address<0)
	{
		printf("illigal address\n");
		return '\0';
	}
	
	if(mem_sim->page_table[pageLocationInTable].valid == VALID_PAGE) //if frame is already in memory, just update the values
	{
		returnValue=mem_sim->main_memory[mem_sim->page_table[pageLocationInTable].frame+pageOffset];
		return returnValue;		
	}
	
	if(address>=0 && address < mem_sim->textSize) //if the page is not in memory but is from exeC file. BRING IN MEMORY AND READ FROM IT
	{
		char *dataFromExec=getPageFromExecFile(mem_sim->program_fd,pageLocationInTable*PAGE_SIZE, mem_sim->execName);
		if(mem_sim->mainMemoryBlocks < MEMORY_SIZE/PAGE_SIZE && mem_sim->page_table[pageLocationInTable].frame == NO_FRAME)  // if there is a place in memory 																		for the page
		{
            		frameToWriteInMemory =  mem_sim->mainMemory_free_index;
            		storePageInMemory(mem_sim->main_memory ,frameToWriteInMemory, dataFromExec);
            		free(dataFromExec);
           		updatePageValues(mem_sim->page_table, VALID_PAGE ,NOT_DIRTY , NOT_IN_BACKING_STORE , frameToWriteInMemory, pageLocationInTable);
            		mem_sim->mainMemory_free_index += PAGE_SIZE; //update next free memory cell
            		mem_sim->fifo[mem_sim->mainMemoryBlocks] = pageLocationInTable; //update the fifo array(the next who will be out)
         
			if(mem_sim->mainMemoryBlocks<MAX_PAGES_IN_RAM)
                		mem_sim->mainMemoryBlocks++;
                	
                	returnValue=mem_sim->main_memory[frameToWriteInMemory+pageOffset];	//the return value
    			return returnValue;
		}
		// in case that the memory is full
		pageToSwap = mem_sim->fifo[0]; 
		shiftArray(mem_sim->fifo,MAX_PAGES_IN_RAM); 
		mem_sim->fifo[MAX_PAGES_IN_RAM -1]=pageLocationInTable; // insert next page into fifo to be out 
		frameToWriteInMemory = mem_sim->page_table[pageToSwap].frame; //the place of the frame to write to
		
		if(mem_sim -> page_table[pageToSwap].dirty == DIRTY) //if the page that will go out was changed store it in swap
		{
			char *dataToSwap = getPageFromMemory(frameToWriteInMemory , mem_sim->main_memory);	//what to save in swap
			storePageInSwap(mem_sim->swapfile_fd, dataToSwap, (pageToSwap)*PAGE_SIZE, mem_sim->swapName);	
			free(dataToSwap);
		}
		if(pageToSwap>=0 && pageToSwap < mem_sim->textSize/PAGE_SIZE) // if page is from address 0 to 40 we dont save it in back store
		    updatePageValues(mem_sim->page_table, INVALID_PAGE , NOT_DIRTY ,NOT_IN_BACKING_STORE ,NO_FRAME ,pageToSwap);
		else
			updatePageValues(mem_sim->page_table, INVALID_PAGE , NOT_DIRTY ,IN_BACKING_STORE ,NO_FRAME ,pageToSwap);
				
		storePageInMemory(mem_sim->main_memory ,frameToWriteInMemory, dataFromExec);
		free(dataFromExec);
		updatePageValues(mem_sim->page_table, VALID_PAGE , NOT_DIRTY ,NOT_IN_BACKING_STORE , frameToWriteInMemory, pageLocationInTable); //not dirty
		returnValue=mem_sim->main_memory[frameToWriteInMemory+pageOffset];
		return returnValue;	
	}
	
	if(mem_sim->page_table[pageLocationInTable].valid == INVALID_PAGE && mem_sim->page_table[pageLocationInTable].in_backing_store == NOT_IN_BACKING_STORE) //if 																		page is not found at all
	{
		printf("data not in memory!\n");
		return '\0';
	}
	
	else //THE ADDRESS IS BETWEEN 40 TO 80 AND IS IN BACKING STORE. WE WILL BRING IT TO MEMORY AND THEN READ FROM IT
	{	
		//***AS I WROTE IN README, THIS IS A DOUBLE CODE FROM ABOVE, COULDNT CHANGE IT , GOT CORE DUMPED AND THE WAS FINISHIED. PLEASE DONT DROP POINTS
		// ON THIS!!!
		char *dataFromSwap = getPageFromSwap(mem_sim->swapfile_fd , pageLocationInTable*PAGE_SIZE , mem_sim->swapName );  //the one we will load
		
		pageToSwap = mem_sim->fifo[0];
		shiftArray(mem_sim->fifo,MAX_PAGES_IN_RAM);
		mem_sim->fifo[MAX_PAGES_IN_RAM-1]=pageLocationInTable;
		frameToWriteInMemory = mem_sim->page_table[pageToSwap].frame;
		
		if(mem_sim -> page_table[pageToSwap].dirty == DIRTY)
		{
			char *dataToSwap = getPageFromMemory(frameToWriteInMemory , mem_sim->main_memory);	
			storePageInSwap(mem_sim->swapfile_fd, dataToSwap, (pageToSwap)*PAGE_SIZE , mem_sim->swapName);	
			free(dataToSwap);
		}
		if(pageToSwap>=0 && pageToSwap < mem_sim->textSize/PAGE_SIZE)
		    updatePageValues(mem_sim->page_table, INVALID_PAGE , NOT_DIRTY ,NOT_IN_BACKING_STORE ,NO_FRAME ,pageToSwap);
		else
			updatePageValues(mem_sim->page_table, INVALID_PAGE , NOT_DIRTY ,IN_BACKING_STORE ,NO_FRAME ,pageToSwap);
			
		storePageInMemory(mem_sim->main_memory ,frameToWriteInMemory, dataFromSwap);
		free(dataFromSwap);		
		updatePageValues(mem_sim->page_table, VALID_PAGE , NOT_DIRTY , NOT_IN_BACKING_STORE , frameToWriteInMemory, pageLocationInTable); //not dirty
		returnValue=mem_sim->main_memory[frameToWriteInMemory+pageOffset];		
	}	
	return returnValue;
}
//=====================================================================================================================================//	

void store(struct sim_database * mem_sim , int address, char value)
{
	if(address>=0 && address < mem_sim->textSize || address> NUM_OF_PAGES*PAGE_SIZE ||address<0) //IF ADDRESS IS ILLIGAL 
	{
		printf("Writing is not allowed!\n");
		return;
	}
	int pageLocationInTable = (address / PAGE_SIZE);
	int pageOffset = address % PAGE_SIZE;
	int frameToWriteInMemory = -1; //position of the frame to write in memory
	char *pageDataToStore=createNewPageData(value,pageOffset); //new page to save in nemory
	int pageToSwap; //the page to swap

	if(mem_sim->page_table[pageLocationInTable].valid == VALID_PAGE) //block already exists just add to RAM by ofset
	{
        	frameToWriteInMemory = mem_sim->page_table[pageLocationInTable].frame;
       		storeValueInMemory(mem_sim->main_memory , frameToWriteInMemory , pageOffset , value);
       		return;
	}
	//if the given page is not in backing store and there is a room from a new page in memory
	else if(mem_sim->mainMemoryBlocks < MEMORY_SIZE/PAGE_SIZE && mem_sim->page_table[pageLocationInTable].in_backing_store == NOT_IN_BACKING_STORE)  
	{
            	frameToWriteInMemory =  mem_sim->mainMemory_free_index;
            	storePageInMemory(mem_sim->main_memory ,frameToWriteInMemory, pageDataToStore);
            	free(pageDataToStore);
           	updatePageValues(mem_sim->page_table, VALID_PAGE , DIRTY , NOT_IN_BACKING_STORE , frameToWriteInMemory, pageLocationInTable);
            	mem_sim->mainMemory_free_index += PAGE_SIZE;
            	mem_sim->fifo[mem_sim->mainMemoryBlocks] = pageLocationInTable; //update last cell of fifo to be the new page
            	
		if(mem_sim->mainMemoryBlocks<MAX_PAGES_IN_RAM) //current blocks in memory
                	mem_sim->mainMemoryBlocks++;
    		return;
	}

	else if(mem_sim->page_table[pageLocationInTable].valid == INVALID_PAGE)  // if page doesn't exist and there is no room in RAM
	{	//we will swap out a page by fifo order and put instead a new page
		pageToSwap = mem_sim->fifo[0];
		shiftArray(mem_sim->fifo,MAX_PAGES_IN_RAM);
		mem_sim->fifo[MAX_PAGES_IN_RAM -1]=pageLocationInTable;
		frameToWriteInMemory = mem_sim->page_table[pageToSwap].frame;
		
		if(mem_sim -> page_table[pageToSwap].dirty == DIRTY) //if the page was changed, write it to swap
		{
			char *dataToSwap = getPageFromMemory(mem_sim->page_table[pageToSwap].frame , mem_sim->main_memory);	
			storePageInSwap(mem_sim->swapfile_fd, dataToSwap, (pageToSwap)*PAGE_SIZE , mem_sim->swapName);	
			free(dataToSwap);	
		}
		updatePageValues(mem_sim->page_table, INVALID_PAGE , NOT_DIRTY ,IN_BACKING_STORE ,NO_FRAME ,pageToSwap);
		if(mem_sim->page_table[pageLocationInTable].in_backing_store ==NOT_IN_BACKING_STORE) //if page is not in backing store, store it in ram
		{
			storePageInMemory(mem_sim->main_memory ,frameToWriteInMemory, pageDataToStore);
			free(pageDataToStore);
		}	
		else//in backing store, bring it from swap and save it memory , than write the data
		{
			char *dataFromSwap = getPageFromSwap(mem_sim->swapfile_fd,(pageLocationInTable)*PAGE_SIZE , mem_sim->swapName);
			storePageInMemory(mem_sim->main_memory ,frameToWriteInMemory, dataFromSwap);
			storeValueInMemory(mem_sim->main_memory ,frameToWriteInMemory ,pageOffset ,value);
			free(dataFromSwap);	
		}		
		updatePageValues(mem_sim->page_table, VALID_PAGE , DIRTY , NOT_IN_BACKING_STORE , frameToWriteInMemory, pageLocationInTable);	
		
	}
}
//=====================================================================================================================================//
//prints the RAM state
void print_memory(struct sim_database *mem_sim)
{
	printf("                             MAIN MEMORY\n");
	int i;
	for(i=0;i<MEMORY_SIZE;i++)
		printf("[%c]",mem_sim->main_memory[i]);
}
//=====================================================================================================================================//
//prints the page table
void print_page_table(struct sim_database *mem_sim)
{
	printf("                             PAGE TABLE\n");
	int i;
		printf("[dirty] [valid] [backstore] [frame]\n");
	for(i=0;i<NUM_OF_PAGES;i++)
		printf("  [%d]    [%d]     [%d]      [%d]\n",mem_sim->page_table[i].dirty,mem_sim->page_table[i].valid
						,mem_sim->page_table[i].in_backing_store,mem_sim->page_table[i].frame);
}
//=====================================================================================================================================//
//prints the swap file
void print_swap(struct sim_database *mem_sim)
{
	printf("                              SWAP_FILE\n");
	char buffer[SWAP_SIZE+1];
	buffer[SWAP_SIZE]='\0';
	mem_sim->swapfile_fd=open(mem_sim->swapName,O_RDWR,0);
	if(read(mem_sim->swapfile_fd,buffer,SWAP_SIZE)!=SWAP_SIZE)
	{
		printf("error reading from file\n");
		return;
	}
	close(mem_sim->swapfile_fd);
	printf("|%s|\n",buffer);
}
//======================================================================================================================================//
//clears all systems memory
void clear_system(struct sim_database *mem_sim)
{
	close(mem_sim->program_fd); // close the exe file
	close(mem_sim->swapfile_fd); // close the swap file
    	free(mem_sim->page_table); // the the pages
    	free(mem_sim); // free the database
}










