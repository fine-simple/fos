// Scenario that tests environment free run tef1 5 3
#include <inc/lib.h>

#define LOG(text, vars) cprintf(text, vars);
void _main(void)
{
	// Testing scenario 1: without using dynamic allocation/de-allocation, shared variables and semaphores
	// Testing removing the allocated pages in mem, WS, mapped page tables, env's directory and env's page file

	int freeFrames_before = sys_calculate_free_frames() ;
	int usedDiskPages_before = sys_pf_calculate_allocated_pages() ;
	cprintf("\n---# of free frames before running programs = %d\n", freeFrames_before);

	/*[4] CREATE AND RUN ProcessA & ProcessB*/
	//Create 3 processes

	int32 envIdProcessA = sys_create_env("ef_fib", (myEnv->page_WS_max_size),(myEnv->SecondListSize), 50);
	int freeFramesAfterA = sys_calculate_free_frames() ;
	LOG("Proc A used : %d Frames\n", freeFrames_before - freeFramesAfterA);
	int32 envIdProcessB = sys_create_env("ef_fact", (myEnv->page_WS_max_size)-1,(myEnv->SecondListSize), 50);
	int freeFramesAfterB = sys_calculate_free_frames() ;
	LOG("Proc B used : %d Frames\n",freeFramesAfterA - freeFramesAfterB);
	int32 envIdProcessC = sys_create_env("ef_fos_add",(myEnv->page_WS_max_size)*4,(myEnv->SecondListSize), 50);
	int freeFramesAfterC = sys_calculate_free_frames() ;
	LOG("Proc C used : %d Frames\n", freeFramesAfterB - freeFramesAfterC);

	//Run 3 processes
	sys_run_env(envIdProcessA);
	sys_run_env(envIdProcessB);
	sys_run_env(envIdProcessC);

	env_sleep(6000);
	cprintf("\n---# of free frames after running programs = %d\n", sys_calculate_free_frames());

	//Kill the 3 processes
	sys_free_env(envIdProcessA);
	int aReturned = sys_calculate_free_frames();
	LOG("Proc A returned : %d Frames\n", aReturned -  freeFramesAfterC);
	sys_free_env(envIdProcessB);
	int bReturned = sys_calculate_free_frames();
	LOG("Proc B returned : %d Frames\n", bReturned -  aReturned);
	sys_free_env(envIdProcessC);
	int cReturned = sys_calculate_free_frames();
	LOG("Proc C returned : %d Frames\n", cReturned -  bReturned);
	//TODO: delete cprints (debugging)
	cprintf("PROCESS C DONE\n");

	//Checking the number of frames after killing the created environments
	int freeFrames_after = sys_calculate_free_frames() ;
	int usedDiskPages_after = sys_pf_calculate_allocated_pages() ;
	//TODO: delete cprints (debugging)
	cprintf("\n---# of free frames before killing the processes = %d\n",freeFrames_before);
	cprintf("\n---# of free frames after killing the processes = %d\n", freeFrames_after);

	if((freeFrames_after - freeFrames_before) !=0)
		panic("env_free() does not work correctly... check it again.") ;

	cprintf("\n---# of free frames after closing running programs returned back to be as before running = %d\n", freeFrames_after);

	cprintf("\n\nCongratulations!! test scenario 1 for envfree completed successfully.\n");
	return;
}
