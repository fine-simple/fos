#include <inc/lib.h>

// malloc()
//	This function use BEST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

#define MAX_NUM_OF_PAGES ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE)

int pages_allocated[MAX_NUM_OF_PAGES];
int unallocated_start;

int best_fit(int num_of_pages)
{	
	int min_fit_index = -1;
	int min_fit_pages = MAX_NUM_OF_PAGES + 1;
	int current_free_pages = 0;
	
	for (int i = 0; i < MAX_NUM_OF_PAGES && i < unallocated_start; )
	{
		// if allocated, jump this space
		if(pages_allocated[i] != 0)
		{
			if(current_free_pages >= num_of_pages && current_free_pages < min_fit_pages )
			{
				min_fit_index = i - current_free_pages;
				min_fit_pages = current_free_pages;
			}
			i+= pages_allocated[i];
			current_free_pages=0;
			continue;
		}
		current_free_pages++;
		i++;
	}

	// count last free pages
	current_free_pages += MAX_NUM_OF_PAGES - unallocated_start;
	if(current_free_pages >= num_of_pages && current_free_pages < min_fit_pages)
	{
		min_fit_index = MAX_NUM_OF_PAGES - current_free_pages;
	}
	// update last unallocated part
	if(min_fit_index >= unallocated_start)
		unallocated_start = min_fit_index + num_of_pages;
	return min_fit_index;
}

void* malloc(uint32 size)
{
	//TODO[DONE] : Malloc User
	size = ROUNDUP(size, PAGE_SIZE);
	int num_of_pages = size / PAGE_SIZE;
	
	int index = -1;
	if(sys_isUHeapPlacementStrategyBESTFIT())
		index = best_fit(num_of_pages);
	
	if(index == -1)
		return NULL;
	uint32 va = USER_HEAP_START + index * PAGE_SIZE;
	// allocate the pages
	pages_allocated[index] = num_of_pages;
	sys_allocateMem(va, size);

	return (uint32*)va;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO[DONE] : Free User
	int index = ((uint32)virtual_address - USER_HEAP_START) / PAGE_SIZE;
	
	// update unallocated start
	if(index + pages_allocated[index] == unallocated_start)
		unallocated_start = index;
	
	// freeing
	sys_freeMem((uint32)virtual_address, pages_allocated[index] * PAGE_SIZE);
	pages_allocated[index] = 0;
}

//==================================================================================//
//================================ OTHER FUNCTIONS =================================//
//==================================================================================//

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	panic("this function is not required...!!");
	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	panic("this function is not required...!!");
	return 0;
}

void sfree(void* virtual_address)
{
	panic("this function is not required...!!");
}

void *realloc(void *virtual_address, uint32 new_size)
{
	panic("this function is not required...!!");
	return 0;
}

void expand(uint32 newSize)
{
	panic("this function is not required...!!");
}
void shrink(uint32 newSize)
{
	panic("this function is not required...!!");
}

void freeHeap(void* virtual_address)
{
	panic("this function is not required...!!");
}
