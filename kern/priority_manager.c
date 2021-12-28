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
 *	BELOW NORMAL : if ws is half empty removes it
 *		else just sets the flag and adjusts the priority for the PFH
 *  NORMAL : returns to original size
 *  ABOVE NORMAL : if ws is full doubles it
 *		else just sets the flag and adjusts the priority for the PFH
 *  High : doubles the ws, sets the flag and adjusts the priority for the PFH
 */

char tmp[2];
#define ENABLE_DEBUG 1
#define LOG(text, vars) if(ENABLE_DEBUG){cprintf(text, vars);}
#define LOG_LISTS if(ENABLE_DEBUG){print_page_working_set_or_LRUlists(curenv);}
#define PAUSE readline("continue?", tmp);


// PRIORITY MANAGEMENT HELPERS ////
void resizeLRUwithReplacement(struct Env *env, int newActiveSize, int newSecondSize);
void updateListsSizes(struct Env* env,int newActiveSize, int newSecondSize);
bool halfWsIsEmpty(struct Env *env);

void fromNormal(struct Env *e, int priority);
void fromLow(struct Env *e, int priority);
void fromBelowNormal(struct Env *e, int priority);
void fromAboveNormal(struct Env *e, int priority);
void fromHigh(struct Env *e, int priority);

void setPriorityLow(struct Env *e);
void setPriorityBelowNormal(struct Env *e);
void setPriorityNormal(struct Env *e);
void setPriorityAboveNormal(struct Env *e);
void setPriorityHigh(struct Env *e);
////////



void set_program_priority(struct Env* env, int priority)
{
	//TODO: [PROJECT 2021 - BONUS4] Set “Program Priority” and perform the corresponding action

	if (env->priority == 0)
		env->priority = PRIORITY_NORMAL;


	//first change so we must save the original Ws size
	if (env->priority == PRIORITY_NORMAL)
	{
		env->originalActiveListSize = env->ActiveListSize;
		env->originalSecondListSize = env->SecondListSize;
	}

	//TODO: Handle switching from priorities other than NORMAL
	switch (env->priority)
	{
		case PRIORITY_LOW:
			fromLow(env, priority);
			break;
		case PRIORITY_BELOWNORMAL:
			fromBelowNormal(env, priority);
			break;
		case PRIORITY_NORMAL:
			fromNormal(env, priority);
			break;
		case PRIORITY_ABOVENORMAL:
			fromAboveNormal(env, priority);
			break;
		case PRIORITY_HIGH:
			fromHigh(env, priority);
			break;
	}

	env->priority = priority;


}


//==================================//
//==== State Switching Handling ====//
//==================================//
//(lots of redundant code but easier modification)

void fromNormal(struct Env *env, int priority)
{
	switch(priority)
	{
	case PRIORITY_LOW:
		setPriorityLow(env);
		break;
	case PRIORITY_BELOWNORMAL:
		setPriorityBelowNormal(env);
		break;
	case PRIORITY_NORMAL:
		break;
	case PRIORITY_ABOVENORMAL:
		setPriorityAboveNormal(env);
		break;
	case PRIORITY_HIGH:
		setPriorityHigh(env);
		break;
	}
}


void fromLow(struct Env *env, int priority)
{
	switch(priority)
		{
		case PRIORITY_LOW:
			break;
		case PRIORITY_BELOWNORMAL:
			setPriorityNormal(env); //reset to original
			setPriorityBelowNormal(env);
			break;
		case PRIORITY_NORMAL:
			setPriorityNormal(env);
			break;
		case PRIORITY_ABOVENORMAL:
			setPriorityNormal(env);
			setPriorityAboveNormal(env);
			break;
		case PRIORITY_HIGH:
			setPriorityNormal(env);
			setPriorityHigh(env);
			break;
		}
}

void fromBelowNormal(struct Env *env, int priority)
{
	switch(priority)
	{
	case PRIORITY_LOW:
		setPriorityLow(env);
		break;
	case PRIORITY_BELOWNORMAL:
		break;
	case PRIORITY_NORMAL:
		setPriorityNormal(env);
		break;
	case PRIORITY_ABOVENORMAL:
		setPriorityNormal(env);
		setPriorityAboveNormal(env);
		break;
	case PRIORITY_HIGH:
		setPriorityNormal(env);
		setPriorityHigh(env);
		break;
	}
}

void fromAboveNormal(struct Env *env, int priority)
{
	switch(priority)
	{
	case PRIORITY_LOW:
		setPriorityNormal(env);
		setPriorityLow(env);
		break;
	case PRIORITY_BELOWNORMAL:
		setPriorityNormal(env);
		setPriorityBelowNormal(env);
		break;
	case PRIORITY_NORMAL:
		setPriorityNormal(env);
		break;
	case PRIORITY_ABOVENORMAL:
		break;
	case PRIORITY_HIGH:
		setPriorityHigh(env);
		break;
	}
}
void fromHigh(struct Env *env, int priority)
{
	switch(priority)
	{
	case PRIORITY_LOW:
		setPriorityNormal(env);
		setPriorityLow(env);
		break;
	case PRIORITY_BELOWNORMAL:
		setPriorityNormal(env);
		setPriorityBelowNormal(env);
		break;
	case PRIORITY_NORMAL:
		setPriorityNormal(env);
		break;
	case PRIORITY_ABOVENORMAL:
		setPriorityNormal(env);
		setPriorityAboveNormal(env);
		break;
	case PRIORITY_HIGH:
		break;
	}
}





//==============================//
//========== HELPERS ===========//
//==============================//




