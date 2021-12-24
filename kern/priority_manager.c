#include <inc/stdio.h>
#include <kern/priority_manager.h>
#include <inc/assert.h>
#include <kern/helpers.h>
#include <kern/memory_manager.h>
#include <kern/kheap.h>
#include <kern/file_manager.h>

/** Summary :
 * Priority manager sets the priority and a flag for the PFH
 *	LOW : the priority manager IMMEDIATELY resizes the WS
 *	BELOW NORMAL : just sets the flag and adjusts the priority for the PFH
 *  NORMAL : returns to original size
 *  ABOVE NORMAL : just sets the flag and adjusts the priority for the PFH
 *  High : just sets the flag and adjusts the priority for the PFH
 */

// PRIORITY MANAGEMENT HELPERS ////
void changeLRUToPriority(struct Env *e, int priority);
void resizeLRU(struct Env *e, int newActiveSize, int newSecondSize);
void updateListsSizes(struct Env* env,int newActiveSize, int newSecondSize);

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

	//first change so we must save the original Ws size
	if (env->priority == PRIORITY_NORMAL)
	{
		env->originalActiveListSize = env->ActiveListSize;
		env->originalSecondListSize = env->SecondListSize;

		changeLRUToPriority(env, priority);
		env->priority = priority;
	}

	//TODO: Handle switching from priorities other than NORMAL

	if (priority != PRIORITY_LOW)
		env->isWSChanged = 1;

}

void changeLRUToPriority(struct Env *env, int priority){
	switch(priority)
	{
	case PRIORITY_LOW:
		setPriorityLow(env);
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

 //remove half of active list and half of second list
void setPriorityLow(struct Env* env)
{
	//FIXME: probably will need further thinking
	int newActiveSize = ROUNDUP(env->ActiveListSize / 2, 2);
	int newSecondSize = ROUNDUP(env->SecondListSize / 2, 2);

	resizeLRU(env, newActiveSize, newSecondSize);
}


void resizeLRU(struct Env *env, int newActiveSize, int newSecondSize)
{
	//remove last half of second list
		while(LIST_SIZE(&env->SecondList) > newSecondSize)
			//getVictimElement(env);

		// move half of elements of active list to second list
		while(LIST_SIZE(&env->ActiveList) > newActiveSize)
		{
			//if second is full remove a victim
//			if(LIST_SIZE(&env->SecondList) == newSecondSize)
//				getVictimElement(env);

//			moveActiveListElementToSecondList(env);
		}

	updateListsSizes(env, newActiveSize, newSecondSize);
}

void updateListsSizes(struct Env *env, int newActiveSize, int newSecondSize)
{
	env->ActiveListSize = newActiveSize;
	env->SecondListSize = newSecondSize;
	env->page_WS_max_size = newActiveSize + newSecondSize;
}
