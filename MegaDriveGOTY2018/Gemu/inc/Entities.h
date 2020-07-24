/*!
\file Entities.h
\brief ECS Entities header file
\author Michael Atchapero
\date 06/2018

Entities header file of the ECS architecture.
*/

#ifndef ECS_ENTITIES_H_
#define ECS_ENTITIES_H_

#pragma pack(1)

#include "Components.h"


/*! \brief Maximum number of entities in game world.*/
#define ENTITY_COUNT 20


/*! \brief Entity slot of the currently controlled player character.

	Variable that keeps track of the entity slot belonging to the currently controlled player character.
	This avoid looping through all entities when only two entities are active in-game.
	A variable currentEnemy is not needed
	because current player entity will
	reference current enemy entity slot through Timing structure, 'facing' variable. 
*/
u8 currentPlayer;

/*! \brief Structure of current game world state.

	The game world contains for each component an array of components of length of maximum entities in game.
	Since the game contains 4 components, 4 arrays are needed.
	mask refers to component masks. See Components.h

\param mask[] An array of component masks, one for each entity.
\param health[] An array of Health components, one for each entity.
\param timing[] An array of Timing components, one for each entity.
\param move[] An array of Move components, one for each entity.
\param teamMember[] An array of TeamMember components, one for each entity
*/
typedef struct {
	u16 mask[ENTITY_COUNT];

	Health health[ENTITY_COUNT];
	Timing timing[ENTITY_COUNT];
	Move move[ENTITY_COUNT];
	TeamMember teamMember[ENTITY_COUNT];

} World;

/*! \brief Finds the next unused entity slot.

	This function is used to find entity slots where we can safely write data without overwriting current data.
	\param *world The game state as a World structure.
	\return Next empty entity slot value.
*/
u8 nextEmptyEntitySlot(World *world);

/*! \brief Flags an entity slot as unused.
	\param *world The game state as a World structure.
	\param entity The entity slot to flag as unused.
	\return void.
*/
void destroyEntity(World *world, u8 entity);

/*! \brief Flags all entity slots as unused.

	Run this function once before using a World structure for anything.
	\param *world The game state as a World structure.
	\return void
*/
void destroyAllEntities(World *world);

/*! \brief Creates a player character.

	Creating a player character involves assigning component data suitable for a player character.
	\param *world The game state as a World structure.
	\param spriteCharacter The animation spritesheet to use with this entity.
	\param memberID Set to 0 if 1st character, 1 if 2nd character, ...
	\return entity Number of entity slot written to.
*/
u8 createPlayerChar(World *world, SpriteSheet spriteCharacter, u8 memberID);

/*! \brief Creates an enemy character.

	Creating an enemy character involves assigning component data suitable for an enemy character.
	\param *world The game state as a World structure.
	\param spriteCharacter The animation spritesheet to use with this entity.
	\return entity Number of entity slot written to.
*/
u8 createEnemyChar(World *world, SpriteSheet spriteCharacter);

#endif /* ECS_ENTITIES_H_ */
