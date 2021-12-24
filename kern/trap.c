#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/memory_manager.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/command_prompt.h>
#include <kern/user_environment.h>
#include <kern/file_manager.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/trap.h>

extern void __static_cpt(uint32 *ptr_page_directory, const uint32 virtual_address, uint32 **ptr_page_table);

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va);
void page_fault_handler(struct Env * curenv, uint32 fault_va);
void page_fault_handler_old(struct Env * curenv, uint32 fault_va);
void table_fault_handler(struct Env * curenv, uint32 fault_va);

static struct Taskstate ts;

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;


/// Interrupt descriptor table.  (Must be built at run time because
/// shifted function addresses can't be represented in relocation records.)
///

struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
		sizeof(idt) - 1, (uint32) idt
};
extern  void (*PAGE_FAULT)();
extern  void (*SYSCALL_HANDLER)();
extern  void (*DBL_FAULT)();

extern  void (*ALL_FAULTS0)();
extern  void (*ALL_FAULTS1)();
extern  void (*ALL_FAULTS2)();
extern  void (*ALL_FAULTS3)();
extern  void (*ALL_FAULTS4)();
extern  void (*ALL_FAULTS5)();
extern  void (*ALL_FAULTS6)();
extern  void (*ALL_FAULTS7)();
//extern  void (*ALL_FAULTS8)();
//extern  void (*ALL_FAULTS9)();
extern  void (*ALL_FAULTS10)();
extern  void (*ALL_FAULTS11)();
extern  void (*ALL_FAULTS12)();
extern  void (*ALL_FAULTS13)();
//extern  void (*ALL_FAULTS14)();
//extern  void (*ALL_FAULTS15)();
extern  void (*ALL_FAULTS16)();
extern  void (*ALL_FAULTS17)();
extern  void (*ALL_FAULTS18)();
extern  void (*ALL_FAULTS19)();


extern  void (*ALL_FAULTS32)();
extern  void (*ALL_FAULTS33)();
extern  void (*ALL_FAULTS34)();
extern  void (*ALL_FAULTS35)();
extern  void (*ALL_FAULTS36)();
extern  void (*ALL_FAULTS37)();
extern  void (*ALL_FAULTS38)();
extern  void (*ALL_FAULTS39)();
extern  void (*ALL_FAULTS40)();
extern  void (*ALL_FAULTS41)();
extern  void (*ALL_FAULTS42)();
extern  void (*ALL_FAULTS43)();
extern  void (*ALL_FAULTS44)();
extern  void (*ALL_FAULTS45)();
extern  void (*ALL_FAULTS46)();
extern  void (*ALL_FAULTS47)();



static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
			"Divide error",
			"Debug",
			"Non-Maskable Interrupt",
			"Breakpoint",
			"Overflow",
			"BOUND Range Exceeded",
			"Invalid Opcode",
			"Device Not Available",
			"Double Fault",
			"Coprocessor Segment Overrun",
			"Invalid TSS",
			"Segment Not Present",
			"Stack Fault",
			"General Protection",
			"Page Fault",
			"(unknown trap)",
			"x87 FPU Floating-Point Error",
			"Alignment Check",
			"Machine-Check",
			"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	return "(unknown trap)";
}


void
idt_init(void)
{
	extern struct Segdesc gdt[];

	// LAB 3: Your code here.
	//initialize idt
	SETGATE(idt[T_PGFLT], 0, GD_KT , &PAGE_FAULT, 0) ;
	SETGATE(idt[T_SYSCALL], 0, GD_KT , &SYSCALL_HANDLER, 3) ;
	SETGATE(idt[T_DBLFLT], 0, GD_KT , &DBL_FAULT, 0) ;


	SETGATE(idt[T_DIVIDE   ], 0, GD_KT , &ALL_FAULTS0, 3) ;
	SETGATE(idt[T_DEBUG    ], 1, GD_KT , &ALL_FAULTS1, 3) ;
	SETGATE(idt[T_NMI      ], 0, GD_KT , &ALL_FAULTS2, 3) ;
	SETGATE(idt[T_BRKPT    ], 1, GD_KT , &ALL_FAULTS3, 3) ;
	SETGATE(idt[T_OFLOW    ], 1, GD_KT , &ALL_FAULTS4, 3) ;
	SETGATE(idt[T_BOUND    ], 0, GD_KT , &ALL_FAULTS5, 3) ;
	SETGATE(idt[T_ILLOP    ], 0, GD_KT , &ALL_FAULTS6, 3) ;
	SETGATE(idt[T_DEVICE   ], 0, GD_KT , &ALL_FAULTS7, 3) ;
	//SETGATE(idt[T_DBLFLT   ], 0, GD_KT , &ALL_FAULTS, 3) ;
	//SETGATE(idt[], 0, GD_KT , &ALL_FAULTS, 3) ;
	SETGATE(idt[T_TSS      ], 0, GD_KT , &ALL_FAULTS10, 3) ;
	SETGATE(idt[T_SEGNP    ], 0, GD_KT , &ALL_FAULTS11, 3) ;
	SETGATE(idt[T_STACK    ], 0, GD_KT , &ALL_FAULTS12, 3) ;
	SETGATE(idt[T_GPFLT    ], 0, GD_KT , &ALL_FAULTS13, 3) ;
	//SETGATE(idt[T_PGFLT    ], 0, GD_KT , &ALL_FAULTS, 3) ;
	//SETGATE(idt[ne T_RES   ], 0, GD_KT , &ALL_FAULTS, 3) ;
	SETGATE(idt[T_FPERR    ], 0, GD_KT , &ALL_FAULTS16, 3) ;
	SETGATE(idt[T_ALIGN    ], 0, GD_KT , &ALL_FAULTS17, 3) ;
	SETGATE(idt[T_MCHK     ], 0, GD_KT , &ALL_FAULTS18, 3) ;
	SETGATE(idt[T_SIMDERR  ], 0, GD_KT , &ALL_FAULTS19, 3) ;


	SETGATE(idt[IRQ0_Clock], 0, GD_KT , &ALL_FAULTS32, 3) ;
	SETGATE(idt[33], 0, GD_KT , &ALL_FAULTS33, 3) ;
	SETGATE(idt[34], 0, GD_KT , &ALL_FAULTS34, 3) ;
	SETGATE(idt[35], 0, GD_KT , &ALL_FAULTS35, 3) ;
	SETGATE(idt[36], 0, GD_KT , &ALL_FAULTS36, 3) ;
	SETGATE(idt[37], 0, GD_KT , &ALL_FAULTS37, 3) ;
	SETGATE(idt[38], 0, GD_KT , &ALL_FAULTS38, 3) ;
	SETGATE(idt[39], 0, GD_KT , &ALL_FAULTS39, 3) ;
	SETGATE(idt[40], 0, GD_KT , &ALL_FAULTS40, 3) ;
	SETGATE(idt[41], 0, GD_KT , &ALL_FAULTS41, 3) ;
	SETGATE(idt[42], 0, GD_KT , &ALL_FAULTS42, 3) ;
	SETGATE(idt[43], 0, GD_KT , &ALL_FAULTS43, 3) ;
	SETGATE(idt[44], 0, GD_KT , &ALL_FAULTS44, 3) ;
	SETGATE(idt[45], 0, GD_KT , &ALL_FAULTS45, 3) ;
	SETGATE(idt[46], 0, GD_KT , &ALL_FAULTS46, 3) ;
	SETGATE(idt[47], 0, GD_KT , &ALL_FAULTS47, 3) ;



	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KERNEL_STACK_TOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS field of the gdt.
	gdt[GD_TSS >> 3] = SEG16(STS_T32A, (uint32) (&ts),
			sizeof(struct Taskstate), 0);
	gdt[GD_TSS >> 3].sd_s = 0;

	// Load the TSS
	ltr(GD_TSS);

	// Load the IDT
	asm volatile("lidt idt_pd");
}

void print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s - %d\n", tf->tf_trapno, trapname(tf->tf_trapno), tf->tf_trapno);
	cprintf("  err  0x%08x\n", tf->tf_err);
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	cprintf("  esp  0x%08x\n", tf->tf_esp);
	cprintf("  ss   0x----%04x\n", tf->tf_ss);
}

void print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here.

	if(tf->tf_trapno == T_PGFLT)
	{
		//print_trapframe(tf);

		fault_handler(tf);
	}
	else if (tf->tf_trapno == T_SYSCALL)
	{
		uint32 ret = syscall(tf->tf_regs.reg_eax
				,tf->tf_regs.reg_edx
				,tf->tf_regs.reg_ecx
				,tf->tf_regs.reg_ebx
				,tf->tf_regs.reg_edi
				,tf->tf_regs.reg_esi);
		tf->tf_regs.reg_eax = ret;
	}
	else if(tf->tf_trapno == T_DBLFLT)
	{
		panic("double fault!!");
	}
	else if (tf->tf_trapno == IRQ0_Clock)
	{
		clock_interrupt_handler() ;
	}

	else
	{
		// Unexpected trap: The user process or the kernel has a bug.
		//print_trapframe(tf);
		if (tf->tf_cs == GD_KT)
		{
			panic("unhandled trap in kernel");
		}
		else {
			//env_destroy(curenv);
			return;
		}
	}
	return;
}

void trap(struct Trapframe *tf)
{
	kclock_stop();

	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		assert(curenv);
		curenv->env_tf = *tf;
		tf = &(curenv->env_tf);
		userTrap = 1;
	}
	if(tf->tf_trapno == IRQ0_Clock)
	{
		//		uint16 cnt0 = kclock_read_cnt0() ;
		//		cprintf("CLOCK INTERRUPT: Counter0 Value = %d\n", cnt0 );

		if (userTrap)
		{
			assert(curenv);
			curenv->nClocks++ ;
		}
	}
	else if (tf->tf_trapno == T_PGFLT){
		//2016: Bypass the faulted instruction
		if (bypassInstrLength != 0){
			if (userTrap){
				curenv->env_tf.tf_eip = (uint32*)((uint32)(curenv->env_tf.tf_eip) + bypassInstrLength);
				env_run(curenv);
			}
			else{
				tf->tf_eip = (uint32*)((uint32)(tf->tf_eip) + bypassInstrLength);
				kclock_resume();
				env_pop_tf(tf);
			}
		}
	}
	trap_dispatch(tf);
	if (userTrap)
	{
		assert(curenv && curenv->env_status == ENV_RUNNABLE);
		env_run(curenv);
	}
	/* 2019
	 * If trap from kernel, then return to the called kernel function using the passed param "tf"
	 * not the user one that's stored in curenv
	 */
	else
	{
		env_pop_tf((tf));
	}
}

//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}

void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint32 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint32 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}


void detect_modified_loop()
{
	struct  Frame_Info * slowPtr = LIST_FIRST(&modified_frame_list);
	struct  Frame_Info * fastPtr = LIST_FIRST(&modified_frame_list);


	while (slowPtr && fastPtr) {
		fastPtr = LIST_NEXT(fastPtr); // advance the fast pointer
		if (fastPtr == slowPtr) // and check if its equal to the slow pointer
		{
			cprintf("loop detected in modiflist\n");
			break;
		}

		if (fastPtr == NULL) {
			break; // since fastPtr is NULL we reached the tail
		}

		fastPtr = LIST_NEXT(fastPtr); //advance and check again
		if (fastPtr == slowPtr) {
			cprintf("loop detected in modiflist\n");
			break;
		}

		slowPtr = LIST_NEXT(slowPtr); // advance the slow pointer only once
	}
	cprintf("finished modi loop detection\n");
}


void print_page_working_set_or_LRUlists(struct Env *e)
{
	if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		int i = 0;
		cprintf("ActiveList:\n============\n") ;
		struct WorkingSetElement * ptr_WS_element ;
		LIST_FOREACH(ptr_WS_element, &(e->ActiveList))
		{
			cprintf("%d:	%x\n", i++, ptr_WS_element->virtual_address);
		}
		cprintf("\nSecondList:\n============\n") ;
		LIST_FOREACH(ptr_WS_element, &(e->SecondList))
		{
			cprintf("%d:	%x\n", i++, ptr_WS_element->virtual_address);
		}
	}
	else
	{
		env_page_ws_print(e);
	}
}

void fault_handler(struct Trapframe *tf)
{
	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		userTrap = 1;
	}
	//print_trapframe(tf);
	uint32 fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	//2017: Check stack overflow for Kernel
	if (!userTrap)
	{
		if (fault_va < KERNEL_STACK_TOP - KERNEL_STACK_SIZE && fault_va >= USER_LIMIT)
			panic("Kernel: stack overflow exception!");
	}
	//2017: Check stack underflow for User
	else
	{
		if (fault_va >= USTACKTOP)
			panic("User: stack underflow exception!");
	}

	//get a pointer to the environment that caused the fault at runtime
	struct Env* faulted_env = curenv;

	//check the faulted address, is it a table or not ?
	//If the directory entry of the faulted address is NOT PRESENT then
	if ( (curenv->env_page_directory[PDX(fault_va)] & PERM_PRESENT) != PERM_PRESENT)
	{
		// we have a table fault =============================================================
		//		cprintf("[%s] user TABLE fault va %08x\n", curenv->prog_name, fault_va);
		faulted_env->tableFaultsCounter ++ ;

		table_fault_handler(faulted_env, fault_va);
	}
	else
	{
		// we have normal page fault =============================================================
		faulted_env->pageFaultsCounter ++ ;

		//cprintf("[%08s] user PAGE fault va %08x\n", curenv->prog_name, fault_va);
		//cprintf("\nPage working set BEFORE fault handler...\n");
		//print_page_working_set_or_LRUlists(curenv);

		if(isBufferingEnabled())
		{
			__page_fault_handler_with_buffering(faulted_env, fault_va);
		}
		else
		{
			page_fault_handler(faulted_env, fault_va);
			//page_fault_handler_old(faulted_env, fault_va);
		}
//		cprintf("\nPage working set AFTER fault handler...\n");
//		print_page_working_set_or_LRUlists(curenv);


	}

	/*************************************************************/
	//Refresh the TLB cache
	tlbflush();
	/*************************************************************/

}


//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
	#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
	#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
	#endif
}

//Handle the page fault

//// HELPERS ///
void* findInSecondChance(struct Env* e, uint32 va);
void addElementToLists(struct Env* e, uint32 va);
void saveWsElementToPageFile(struct Env* e, struct WorkingSetElement *element);
bool workingSetIsEmpty(struct Env* e);
bool activeListIsFull(struct Env* e);
void* getVictimElement(struct Env* e);
void moveActiveListElementToSecondList(struct Env* e);


bool isStackPage(uint32 va);
int page_index(uint32 va);
void addToSCArr(struct WorkingSetElement *element);
void removeFromSCArr(struct WorkingSetElement *element);
void insertInSC(struct Env* e, struct WorkingSetElement* element);
void removeFromSC(struct Env* e, struct WorkingSetElement* element);
// ============== PRIORITY MANAGER ===============
void priorityManager(struct Env *env);
////////

char tmp[2];
#define ENABLE_DEBUG 0
#define LOG(text, vars) if(ENABLE_DEBUG){cprintf(text, vars);}
#define LOG_LISTS if(ENABLE_DEBUG){print_page_working_set_or_LRUlists(curenv);}
#define PAUSE readline("continue?", tmp);

struct WorkingSetElement *stackElementsInSCList[322330] = {};
struct WorkingSetElement *elementsInSCList[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE] = {};

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{

	//TODO: [PROJECT 2021 - BONUS4] Change WS Size according to “Program Priority”
	//priorityManager(curenv);

	//TODO: [DONE]  [PROJECT 2021 - [1] PAGE FAULT HANDLER]
	// Write your code here, remove the panic and write your code
	fault_va = ROUNDDOWN(fault_va, PAGE_SIZE);
	cprintf("fault va: %p\n", fault_va);

	struct WorkingSetElement *element = (struct WorkingSetElement*) findInSecondChance(curenv, fault_va);

	if (element != NULL)// if found in second chance list
	{

		if (activeListIsFull(curenv))
		{
			moveActiveListElementToSecondList(curenv);
		}
		removeFromSC(curenv, element);
		LIST_INSERT_HEAD(&(curenv->ActiveList), element);
	}
	else
	{

		struct Frame_Info *ptr_frame_info;

		if (allocate_frame(&ptr_frame_info) != 0)
			panic("Failed To Allocate Frame\n");

		map_frame(curenv->env_page_directory, ptr_frame_info, (void*)fault_va, PERM_PRESENT | PERM_USER | PERM_WRITEABLE);

		int isPageReadSuccess = pf_read_env_page(curenv, (uint32*) fault_va);
		if (isPageReadSuccess == 0){
			addElementToLists(curenv,fault_va);
			//cprintf("read from page file and added to lists successfully\n");
		}
		else
		{
			if(isStackPage(fault_va))
			{
				int err = pf_add_empty_env_page(curenv, fault_va, 1);

				if(err!=0)
					panic("Failed to add Environment page");
				else
					addElementToLists(curenv,fault_va);
			}
			else
			{
				panic("Page not found in Page File");
			}
		}
	}

	pt_set_page_permissions(curenv, fault_va, PERM_USER | PERM_PRESENT | PERM_WRITEABLE, 0);


	//TODO: [PROJECT 2021 - BONUS3] O(1) Implementation of Fault Handler



}


void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}

bool isStackPage(uint32 va)
{
	return(va>=USTACKBOTTOM && va<=USTACKTOP);
}
int page_index(uint32 va)
{
	return (isStackPage(va)? (va-USTACKBOTTOM)/PAGE_SIZE:(va-USER_HEAP_START)/PAGE_SIZE);
}
void addToSCArr(struct WorkingSetElement *element)
{
	unsigned int va = element->virtual_address;
	int page_no = page_index(va);
	bool stackPage = isStackPage(va);
	if(stackPage)
	{
		stackElementsInSCList[page_no] = element;
	}
	else
	{
		elementsInSCList[page_no] = element;
	}
}
void removeFromSCArr(struct WorkingSetElement *element)
{
	unsigned int va = element->virtual_address;
	int page_no = page_index(va);
	bool stackPage = isStackPage(va);
	if(stackPage)
		stackElementsInSCList[page_no] = NULL;
	else
		elementsInSCList[page_no] = NULL;
}
void* findInSecondChance(struct Env* e, uint32 va)
{
	int page_no = page_index(va);
	if(isStackPage(va))
		return stackElementsInSCList[page_no];
	else
		return elementsInSCList[page_no];
	return NULL;
}
void insertInSC(struct Env* e, struct WorkingSetElement* element)
{
	// insert in List and array
	LIST_INSERT_HEAD(&(e->SecondList), element);
	addToSCArr(element);
}
void removeFromSC(struct Env* e, struct WorkingSetElement* element)
{
	// remove from SC List and Array
	LIST_REMOVE(&(e->SecondList), element);
	removeFromSCArr(element);
}
void addElementToLists(struct Env* e, uint32 va)
{

	struct WorkingSetElement *element;

	if(!workingSetIsEmpty(e))
	{
		element = LIST_LAST(&(e->PageWorkingSetList));
		LIST_REMOVE(&(e->PageWorkingSetList), element);
		element->empty = 0;
		element->virtual_address = va;
		if(activeListIsFull(e))
		{
			// second list is guaranteed to have empty space if ws is not empty and active is full
			moveActiveListElementToSecondList(e);
		}
		LIST_INSERT_HEAD(&(e->ActiveList), element);

	}
	else //must take victim
	{
		// this sequence is guaranteed to happen since the WS list is empty
		// (active and second lists are full)
		element = getVictimElement(e);

		element->virtual_address = va;

		if (e->SecondListSize != 0)
			moveActiveListElementToSecondList(e);
		LIST_INSERT_HEAD(&(e->ActiveList), element);
	}


}

void saveWsElementToPageFile(struct Env* e, struct WorkingSetElement *element)
{
	uint32 va = element->virtual_address;
	uint32 perms = pt_get_page_permissions(e, va);
	bool modified = (PERM_MODIFIED&perms);
	if(!modified)
	{
		unmap_frame(e->env_page_directory, (uint32*) va);
		return;
	}
	uint32 *ptr_page_table;
	struct Frame_Info *ptr_frame_info = get_frame_info(e->env_page_directory, (void*)va, &ptr_page_table);
	int success = pf_update_env_page(e, (void *)va, ptr_frame_info);
	if(success != 0)
		panic("Page to be updated doesn't exist in Page File");

	unmap_frame(e->env_page_directory, (uint32*) va);
}

bool workingSetIsEmpty(struct Env* e)
{
	return LIST_SIZE(&(e->PageWorkingSetList)) == 0;
}

bool activeListIsFull(struct Env* e)
{
	return LIST_SIZE(&(e->ActiveList)) == e->ActiveListSize;
}

bool secondListIsFull(struct Env* e)
{
	return LIST_SIZE(&(e->SecondList)) == e->SecondListSize;
}

// Only removes element from second list, doesn't update active list
void* getVictimElement(struct Env* e)
{
	struct WS_List *list;
	if (e->SecondListSize == 0)
	{
		list = &e->ActiveList;
	}
	else
	{
		list = &e->SecondList;
		removeFromSCArr(LIST_LAST(list));
	}

	struct WorkingSetElement *victimElement = LIST_LAST(list);

	saveWsElementToPageFile(e, victimElement);

	LIST_REMOVE(list, victimElement);

	pt_set_page_permissions(curenv, victimElement->virtual_address, 0, PERM_PRESENT);

	return victimElement;
}

// moves last element in active list to second list head
void moveActiveListElementToSecondList(struct Env* e)
{
	struct WorkingSetElement *activeListTail = LIST_LAST(&(e->ActiveList));
	LIST_REMOVE(&(e->ActiveList), activeListTail);
	insertInSC(e, activeListTail);
	pt_set_page_permissions(e, activeListTail->virtual_address, 0, PERM_PRESENT);

	return;
}

// ===============================================//
// ============== PRIORITY MANAGER ===============//
// ===============================================//
//void setPriorityBelowNormal(struct Env *env);
//void setPriorityAboveNormal(struct Env *env);
//void setPriorityHigh(struct Env *env);
//bool halfWsIsEmpty(struct Env *env);
//void resizeLRU(struct Env *env, int newActiveSize, int newSecondSize);
//void updateListsSizes(struct Env *env, int newActiveSize, int newSecondSize);
//
//
//void priorityManager(struct Env *env)
//{
//	switch(env->priority)
//		{
//		case PRIORITY_LOW:
//			break;
//		case PRIORITY_BELOWNORMAL:
//			if (env->isWSChanged && halfWsIsEmpty(env))
//			{
//				setPriorityBelowNormal(env);
//				env->isWSChanged = 0;
//			}
//			break;
//		case PRIORITY_NORMAL:
//			break;
//		case PRIORITY_ABOVENORMAL:
//			if (env->isWSChanged )
//			{
//				setPriorityAboveNormal(env);
//				env->isWSChanged = 0;
//			}
//			break;
//		case PRIORITY_HIGH:
//			setPriorityHigh(env);
//			break;
//		}
//
//}
//
//
//void setPriorityBelowNormal(struct Env *env)
//{
//	//FIXME: probably will need further thinking
//	int newActiveSize = ROUNDUP(env->ActiveListSize / 2, 2);
//	int newSecondSize = ROUNDUP(env->SecondListSize / 2, 2);
//
//	resizeLRU(env, newActiveSize, newSecondSize);
//
//}
//
//void setPriorityAboveNormal(struct Env *env)
//{
//	updateListsSizes(env, env->ActiveListSize * 2, env->SecondListSize * 2);
//}
//
//void setPriorityHigh(struct Env *env)
//{
//	uint32 Giga =  1024 * 1024 * 1024;
//	if (env->page_WS_max_size * PAGE_SIZE < 2 * Giga)
//	{
//		updateListsSizes(env, env->ActiveListSize * 2, env->SecondListSize * 2);
//	}
//}
//
//////////// PRIORITY HELPERS ///////////
//void resizeLRU(struct Env *env, int newActiveSize, int newSecondSize)
//{
//	//remove last half of second list
//		while(LIST_SIZE(&env->SecondList) > newSecondSize)
//			getVictimElement(env);
//
//		// move half of elements of active list to second list
//		while(LIST_SIZE(&env->ActiveList) > newActiveSize)
//		{
//			//if second is full remove a victim
//			if(LIST_SIZE(&env->SecondList) == newSecondSize)
//				getVictimElement(env);
//
//			moveActiveListElementToSecondList(env);
//		}
//
//	updateListsSizes(env, newActiveSize, newSecondSize);
//}
//
//void updateListsSizes(struct Env *env, int newActiveSize, int newSecondSize)
//{
//	env->ActiveListSize = newActiveSize;
//	env->SecondListSize = newSecondSize;
//	env->page_WS_max_size = newActiveSize + newSecondSize;
//}
//
//bool halfWSIsEmpty(struct Env *env)
//{
//	bool activeLessThanHalf =  (LIST_SIZE(&env->ActiveList) <= env->ActiveListSize / 2);
//	bool secondLessThanHalf =  (LIST_SIZE(&env->SecondList) <= env->SecondListSize / 2);
//	return activeLessThanHalf && secondLessThanHalf;
//}
