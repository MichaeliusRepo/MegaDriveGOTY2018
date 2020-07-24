/*!
\file Entities.c
\brief ECS Entities file
\author Michael Atchapero
\date 06/2018

Entities file of the ECS architecture.
*/

#ifndef ECS_ENTITIES
#define ECS_ENTITIES


#include "../inc/Entities.h"

u8 nextEmptyEntitySlot(World *world) {
	for (u8 entity = 0; entity < ENTITY_COUNT; ++entity)
		if (world->mask[entity] == COMPONENT_NONE)
			return entity;
	return 255;
}

void destroyEntity(World *world, u8 entity) {
	world->mask[entity] = COMPONENT_NONE;
}

void destroyAllEntities(World *world) {
	for (u8 i = 0; i < ENTITY_COUNT; i++)
		world->mask[i] = COMPONENT_NONE;
}

// Helper functions to create template entities.

u8 createPlayerChar(World *world, SpriteSheet spriteCharacter, u8 memberID) {
	// WARNING: Set EVERY value of EVERY component!
	u8 entity = nextEmptyEntitySlot(world);
	world->mask[entity] = COMPONENT_HEALTH | COMPONENT_TIMING | COMPONENT_SPRITE | COMPONENT_MOVE | COMPONENT_TEAMMEMBER;
	world->teamMember[entity].id = memberID;
	world->teamMember[entity].isActive = (memberID == 0) ? TRUE : FALSE;
	if (world->teamMember[entity].isActive)
		currentPlayer = entity;
	world->health[entity].points = 10;
	world->health[entity].staggered = 0;
	world->timing[entity].frames = 0;
	world->move[entity].spriteData = spriteCharacter;
	world->move[entity].move = 0;
	world->timing[entity].facing = currentPlayer - 1;
	return entity;
}

u8 createEnemyChar(World *world, SpriteSheet spriteCharacter) {
	u8 entity = nextEmptyEntitySlot(world);
	world->mask[entity] = COMPONENT_HEALTH | COMPONENT_TIMING | COMPONENT_SPRITE | COMPONENT_MOVE;
	world->health[entity].points = 10;
	world->health[entity].staggered = 0;
	world->move[entity].spriteData = spriteCharacter;
	world->move[entity].move = 0;
	world->timing[entity].facing = entity + 1;
	world->timing[entity].frames = 0;
	return entity;
}

#endif // !ECS_ENTITIES