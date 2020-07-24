/*!
\file Model.h
\brief Model header file
\author Michael Atchapero
\date 06/2018

Model header file containing the ECS architecture.
*/

#ifndef  _MODEL_H_
#define _MODEL_H_

#pragma pack(1)

#include "Systems.h"

#define DIFF_EASY 3  /*!< Difficulty values define how often enemies will act. Will react every 33 frames on average. */
#define DIFF_NORMAL 8  /*!< Difficulty values define how often enemies will act. Will react every 12.5 frames on average. */
#define DIFF_HARD 11  /*!< Difficulty values define how often enemies will act. Will react every 9 frames on average. */
#define DIFF_NIGHTMARE 23  /*!< Difficulty values define how often enemies will act. Will react every 4 frames on average. */
#define DIFF_PERFECT 97  /*!< Difficulty values define how often enemies will act. Will react every frame. */

/*! \brief Accumulating value that if high enough, enemy AI will act.

	Every frame, the difficulty value will be added to this number.
	If the number is less than 100, AI will not act on that frame.
	That means on easy, AI will on average act once every 100/3 = 33.3th frame.
*/
u16 difficultyAIaccumulator;

/*! \brief Runs all systems once with given controller input.
	\param *world The game state as a World structure.
	\param *buttonInput The input state as a ButtonInput structure.
	\return EventQueue structure with which animations and sound effects to play.
*/
static EventQueue updateWorld(World *w, ButtonInput *buttonInput);

#endif // ! _MODEL_H_
