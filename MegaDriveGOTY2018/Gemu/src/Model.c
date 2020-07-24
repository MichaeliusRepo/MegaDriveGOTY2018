/*!
\file Model.c
\brief Model file
\author Michael Atchapero
\date 06/2018

Model file containing ECS architecture.
*/

#ifndef _MODEL
#define _MODEL

#pragma pack(1)

#include "../inc/Model.h"



/*! \brief Runs all systems once with given controller input.
\param *world The game state as a World structure.
\param *buttonInput The input state as a ButtonInput structure.
\return EventQueue structure with which animations and sound effects to play.
*/
static EventQueue updateWorld(World *w, ButtonInput *buttonInput) {
	eventSFX = 0; // default value
	inputSystem(w, buttonInput);

	u16 nextAcc;
	nextAcc = AISystem(w, difficultyAIaccumulator);
	difficultyAIaccumulator = nextAcc;
	
	combatSystem(w);
	return renderSystem(w);
}

#endif // !_MODEL_



