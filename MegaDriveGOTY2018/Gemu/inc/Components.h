/*!
\file Components.h
\brief ECS Components file
\author Michael Atchapero
\date 06/2018

Components file of the ECS architecture. No .c file needed.
*/

#ifndef ECS_COMPONENTS_H_
#define ECS_COMPONENTS_H_

#include "types.h"

#pragma pack(1)

/*! \brief Enumeration with every character sprite sheet.*/
typedef enum {
	mockPlayer1,	/**< Blue player character  */
	mockPlayer2,	/**< Green player character  */
	mockEnemy,	/**< Red enemy character  */
} SpriteSheet;

/*! \brief Structure with health points and time staggered.

	Character dies when hit points reaches 0.
	'staggered' refers to how many frames the character remains unable to move.
	If 'staggered' is 0, character is not staggered.
	When greater than 0, character remains staggered until value reaches 0.
*/
typedef struct {
	u8 points : 5;	/**< Hit points  */
	u8 staggered;	/**< How many frames character remains staggered  */
} Health;

/*! \brief Structure with data counting animation length.

	When a character is idle, 'frames' is 0.
	'frames' increments at every frame when animation is playing.
	'facing' is a reference to the enemy, so as to know whom to interact with in combat system.
*/
typedef struct {
	u16 frames;	/**< Number of frames animation has played for  */
	u8 facing : 4; /**< Entity slot of current opponent  */
} Timing;

/*! \brief Structure with data on current action and corresponding animation. */
typedef struct {
	SpriteSheet spriteData;		/**< Entity slot of current opponent  */
	u8 move : 4;		/**< ID signifying type of attack character is executing. Uses AttackType enum in Systems.h  */
} Move;

/*! \brief Structure with data if the entity is playable and currently controlled */
typedef struct {
	u8 isActive : 1;	/**< Set to 1 (TRUE) if player controls the character currently */
	u8 id : 3;	/**< Player party counter. Range: 1-4  */
} TeamMember;


/* \brief Enumerator with bit fields identifying a set of components.
	The implementation makes use of an enum for creating "component masks",
	represented by bit fields that identify a set of components.
 */
typedef enum {
	COMPONENT_NONE = 0,
	COMPONENT_HEALTH = 1 << 0,
	COMPONENT_TIMING = 1 << 1,
	COMPONENT_SPRITE = 1 << 2,
	COMPONENT_MOVE = 1 << 3,
	COMPONENT_TEAMMEMBER = 1 << 4,
} Component;

#endif // !ECS_COMPONENTS_H_