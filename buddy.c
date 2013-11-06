/*
Justin Lannin and Adam Becker
11/2/13
Cosc301
Professor Sommers
Project 3

Both present throughout entire creation of project.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// function declarations
void *mymalloc(size_t);
static int checkbuddy(int*, int*);
void myfree(void *);
void dump_memory_map(void);
static size_t manageInput(size_t);
static void *findandalloc(int, int *);
static int *getbefore(int*);
static int* merge(int *, int *);

const int HEAPSIZE = (1*1024*1024); // 1 MB
const int MINIMUM_ALLOC = sizeof(int) * 2;

// global file-scope variables for keeping track
// of the beginning of the heap.
void *heap_begin = NULL;
int *freelist = NULL;

void *mymalloc(size_t request_size) {
	if (request_size < 1)
	{
		return NULL;
	}
    // if heap_begin is NULL, then this must be the first
    // time that malloc has been called.  ask for a new
    // heap segment from the OS using mmap and initialize
    // the heap begin pointer.
    if (!heap_begin) {
        heap_begin = mmap(NULL, HEAPSIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        freelist = (int *) heap_begin;
	*(freelist + 0) = HEAPSIZE;
	*(freelist + 1) = 0;
	atexit(dump_memory_map);
    }
	request_size = manageInput(request_size);  //request_size is size we need
	int * temp = freelist;
	if(temp == NULL)
	{
		return NULL;
	}
	while (*(temp+1) != 0) //check up to last spot in freelist
	{
		if(request_size <= (*(temp+0))) //small enough
		{
			return findandalloc(request_size, temp);
		}
		else
		{
			temp = temp + ((*(temp+1))/4); //next spot in freelist
		}
	}
	if (request_size <= (*(temp+0))) //check if last spot is big enough
	{
		return findandalloc(request_size, temp);
	}
	else
	{
		return NULL;
	}
}

//finds a block that is large enough for request_size
//allocates (and updates) that block and returns it.
static void *findandalloc(int request_size, int *temp)
{
	while(request_size <= ((*(temp+0))/2))  //while the half block is still big enough for size requested
	{
		int halfsize = (*(temp+0))/2;
		*(temp + (halfsize/4)) = halfsize; //size of right free block
		if((*(temp+1))==0)//if in last spot of free
		{
			*(temp + (halfsize/4) + 1) = 0;
		}
		else
		{
			*(temp + (halfsize/4) + 1) = (*(temp+1)) - halfsize; //next of right free block
		}
		*(temp+0) = halfsize;
		*(temp+1) = halfsize;
	}
	if(freelist == temp)//freelist is pointing where we are allocating (first item in freelist)
	{
		if( *(freelist+1) == 0) //only item in freelist?
		{
			freelist = NULL;
		}
		else // update freelist to point to next block
		{
			freelist += *(freelist+1)/4;
		}
	}
	else
	{
		int * tempfree = freelist;
		while (tempfree + (*(tempfree + 1)/4) != temp)
		{
			tempfree += (*(tempfree + 1)/4);
		}
		//currently at spot before block to be malloced
		if (*(temp+1) == 0) //if block allocating is in last spot
		{
			*(tempfree+1) = 0;
		}
		else //next = current next + temp's next
		{
			*(tempfree+1) += *(temp+1);
		}
	}
	*(temp+1) = 0; //next to 0
	return (void*)(temp+2); //return after header
}

//returns the correct size of block required
static size_t manageInput(size_t size)
{
	size_t newsize = size + 8;
	size_t powtwo = 1;
	while (powtwo < newsize)
	{
		powtwo = powtwo*2;
	}
	return powtwo;
}

void myfree(void *memory_block) 
{
	if(memory_block== NULL)
	{
		return;
	}
	int * memtemp = (int*) memory_block;//header of to be freed block
	memtemp = (memtemp-2);
	int * freetemp = freelist;

	if (freelist == NULL)
	{
		freelist = memtemp;
	}
	else if (freetemp > memtemp) // allocated block is before the start of free list
	{
		int dif = (freelist - memtemp)*4;
		*(memtemp+1) = dif;//set next
		freelist = memtemp;
	}
	else
	{

		while (((freetemp + (*(freetemp+1))/4)< memtemp) && (*(freetemp + 1) != 0)) //while freelist is before to be freed and not in last spot
		{
			freetemp = freetemp + (*(freetemp+1))/4;
		}
		//should be at free block before our to be freed block
		if (*(freetemp+1)!= 0)
		{
			int dif = (memtemp - freetemp)*4;
			*(memtemp+1) = *(freetemp+1) - dif;
			*(freetemp+1) = dif;//set next
		}
		else//last of freelist
		{
			int dif = (memtemp - freetemp)*4;
			*(freetemp+1) = dif;//set next
		}
		
	}
	while(1)  //merging of blocks
	{
		int *before = getbefore(memtemp);
		if(checkbuddy(before, memtemp)) //previous is buddy?
		{
			memtemp = merge(before, memtemp);
		}
		else if(*(memtemp+0) == *(memtemp+1)) //if size = next, then neighbor in freelist so we can check the right block
		{
			if(checkbuddy(memtemp + (*(memtemp+0)/4), memtemp))
			{
				memtemp = merge(memtemp, memtemp + (*(memtemp+0)/4));
			}		
			else
			{
				return;
			}
		}
		else
		{
			return;  //no buddies!
		}
	}
}

//takes pointers to two blocks and merges them together
static int* merge(int *left, int* right)
{
	*(right+0) = 0;
	*(left+0) = (*(left+0))*2;
	if ((*(right+1)) == 0)
	{
		*(left+1) = 0;
	}
	else
	{
		*(left+1) = *(left+1) + *(right+1);
	}	
	*(right+1) = 0;
	return left;
}

//returns the block in freelist before 
//memtemp, or memtemp if that is the first
//block in freelist
static int *getbefore(int * memtemp)
{
	if(freelist == memtemp)
	{
		return memtemp;
	}
	int * freetemp = freelist;
	while (((freetemp + (*(freetemp+1))/4) != memtemp) && (*(freetemp + 1) != 0)) //while freelist is before to be freed and not in last spot
		{
			freetemp = freetemp + (*(freetemp+1))/4;
		}
	return freetemp;
}

//takes two blocks and returns 1 if they are buddies and 0 otherwise
static int checkbuddy(int* one, int* two)
{
	if(*(one) != *(two)) //if the sizes are not the same, no reason to keep going
	{
		return 0;
	}
	uint64_t convertOne = (uint64_t) one;
	convertOne = (convertOne - (uint64_t)heap_begin);
	uint64_t convertTwo = (uint64_t) two;
	convertTwo = (convertTwo - (uint64_t)heap_begin);
	uint64_t xor = convertTwo^convertOne;
	//if xor is power of two, then we know that the two only differ by one bit
	if (xor == 0)
	{
		return 0;
	}
	int x = 1;
	while (x < *(one))
	{
	x = x*2;
	}
	if(x == xor)//xor is a power of 2
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void dump_memory_map(void) {
	int count = 0;
	int* temp = heap_begin;
	if(freelist == NULL) //all allocated
	{
		printf("%s%d%s%d%s\n", "Block size: ", HEAPSIZE, ", offset ", count, ", allocated");
		return;
	}
	else if(*(temp+1) == 0 && freelist != heap_begin) //1st block allocated
	{
		int firstsize = (freelist - (int*) heap_begin)*4;
		printf("%s%d%s%d%s\n", "Block size: ", firstsize, ", offset ", count, ", allocated");
		count = firstsize;
	}
	temp = freelist;
	while(*(temp+1) != 0) //next is not 0 (not end of freelist)
	{
		printf("%s%d%s%d%s\n", "Block size: ", *(temp + 0), ", offset ", count, ", free");
		count = count + *(temp+1);
		if(*(temp+1) > *(temp+0)) //if next > size
		{
			int sizealloc = ((*(temp+1)) - (*(temp + 0))); // next - size
			printf("%s%d%s%d%s\n", "Block size: ", sizealloc, ", offset ", count-sizealloc, ", allocated");
		}	
		temp = temp + (*(temp + 1))/4;
	}
	printf("%s%d%s%d%s\n", "Block size: ", *(temp + 0), ", offset ", count, ", free"); //print out last element of free
	count = count + *(temp+0);
	//test if one more allocated
	if(count != HEAPSIZE)
	{
		printf("%s%d%s%d%s\n", "Block size: ", HEAPSIZE - count, ", offset ", count, ", allocated");
	}

}

