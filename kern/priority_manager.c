#include <inc/stdio.h>
#include <kern/priority_manager.h>
#include <inc/assert.h>
#include <kern/helpers.h>
#include <kern/memory_manager.h>
#include <kern/kheap.h>
#include <kern/file_manager.h>



/// PRIORITY MANAGEMENT HELPERS ////
void changeLRUToPriority(struct Env *e);
void setPriorityLow(struct Env *e);
void setPriorityBelowNormal(struct Env *e);
void setPriorityNormal(struct Env *e);
void setPriorityAboveNormal(struct Env *e);
void setPriorityHigh(struct Env *e);
////////


void set_program_priority(struct Env* env, int priority)
{
	//TODO: [PROJECT 2021 - BONUS4] Set “Program Priority” and perform the corresponding action
	if (env->priority == priority)
		return;


}

void changeLRUToPriority(struct Env *e){
	switch(e->priority)
	{
	case PRIORITY_LOW:
		break;
	case PRIORITY_BELOWNORMAL:
		break;
	case PRIORITY_NORMAL:
		break;
	case PRIORITY_ABOVENORMAL:
		break;
	case PRIORITY_HIGH:
		break;
	}
}

// remove half of active list and half of second list
//void setLowPriority(struct Env* env)
//{
//	int newActiveSize = ROUNDUP(env->ActiveListSize / 2, 2);
//	int newSecondSize = ROUNDUP(env->SecondListSize / 2, 2);
//
//	//remove last half of second list
//	while(LIST_SIZE(&env->SecondList) > newSecondSize)
//		getVictimElement(env);
//
//	// move half of elements of active list to second list
//	while(LIST_SIZE(&env->ActiveList) > newActiveSize)
//	{
//		//if second is full remove a victim
//		if(LIST_SIZE(&env->SecondList) == newSecondSize)
//			getVictimElement(env);
//
//		moveActiveListElementToSecondList(env);
//	}
//
//	env->ActiveListSize = newActiveSize;
//	env->SecondListSize = newSecondSize;
//}
