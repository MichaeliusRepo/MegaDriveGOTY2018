/*!
\file Systems.c
\brief ECS Systems file
\author Michael Atchapero
\date 06/2018

Systems file of the ECS architecture.
*/

#ifndef ECS_SYSTEMS
#define ECS_SYSTEMS

#include "../inc/Systems.h"

// Forwarding helper (private) functions

static u8 timedRight(World *world, u8 entity);
static u8 isAttacking(World *world, u8 entity);
static void setMove(World *world, u8 entity, u8 move);
static u8 searchForNextPlayerCharacter(World *world, u8 entity);
static void DoIdle(World *world, u8 entity);
static void DoBasicAttack(World *world, u8 entity);
static void DoSpecialAttack(World *world, u8 entity);
static void DoCharacterSwitch(World *world, u8 entity);
static void DoParry(World *world, u8 entity);
static void DoGuard(World *world, u8 entity);
//static void DoEvade(World *world, u8 entity); // unused
static void DealDamage(World *world, u8 entity, u8 damage, u8 isFatal);


// Primary system functions

void inputSystem(World *world, ButtonInput *buttonInput) {
	if (world->move[currentPlayer].move == Dying || world->move[world->timing[currentPlayer].facing].move == Dying)
		return;

	if (buttonInput->isPressed == TRUE) {
		switch (buttonInput->latestButtonPress)
		{
		case A:
			DoBasicAttack(world, currentPlayer);
			break;

		case B:
			DoSpecialAttack(world, currentPlayer);
			break;

		case C:
			DoCharacterSwitch(world, currentPlayer);
			break;

		case Up:
			DoParry(world, currentPlayer);
			break;

		case Down:
			DoGuard(world, currentPlayer);
			break;

			//case Left:
			//	DoEvade(world, currentPlayer);
			//	break;

			//case Right:
			//	DoEvade(world, currentPlayer);
			//	break;

		case Left: case Right: default:
			break;
		}
	}
	else if (world->move[currentPlayer].move == Guarding) // release guard
		DoIdle(world, currentPlayer);
}



u16 AISystem(World *world, u16 difficulty) {
	// Note: The AI is not built through test-driven development. Therefore it has no unit-tests.

	// AI either makes no decision or the correct decision.
	if (difficulty < 100)
		return difficulty;

	u8 AIentity = world->timing[currentPlayer].facing;

	randSGDK %= 3; // Have a 2/3 chance to be true, 1/3 to be false.

	if (world->move[AIentity].move == Dying || world->move[currentPlayer].move == Dying)
		return 0; // Do nothing if dying

	// If player is staggered, attack.
	if (world->move[currentPlayer].move == Staggered) {
		if (randSGDK)
			DoSpecialAttack(world, AIentity);
		else
			DoBasicAttack(world, AIentity);
	}

	else	if (world->move[AIentity].move == Guarding && world->timing[AIentity].frames > PARRY_FRAMES)
		DoIdle(world, AIentity); // Stop guarding if player is vulnerable post-parrying.


	else	if (isAttacking(world, currentPlayer)) {
		if (world->timing[currentPlayer].frames >= (ATTACK_FRAMES - 4)) {
			if (randSGDK)
				DoParry(world, AIentity);
			else
				DoGuard(world, AIentity);
		}
	}

	else 	if (world->move[currentPlayer].move == Guarding)
		DoSpecialAttack(world, AIentity);	// Shave off 1 HP when player is guarding.

	else { // If in doubt, guard or attack.
		if (randSGDK)
			DoBasicAttack(world, AIentity);
		else
			DoGuard(world, AIentity);
	}

	return (difficulty + (randSGDK * difficulty)) % 100;
}



void combatSystem(World *world) {
	for (u8 entity = 0; entity < ENTITY_COUNT; ++entity)
	{
		if (world->health[entity].staggered > 0)
			world->health[entity].staggered--;

		// If move finished, set character as idle.
		if (isAttacking(world, entity) && world->timing[entity].frames > (ATTACK_FRAMES + FOLLOWUP_FRAMES))
			DoIdle(world, entity);
		if (world->move[entity].move == Parrying && world->timing[entity].frames > (PARRY_FRAMES + FOLLOWUP_FRAMES))
			DoIdle(world, entity);
		if (world->move[entity].move == Staggered && world->health[entity].staggered == 0)
			DoIdle(world, entity);

		// If idle, don't increase frame count.
		if (world->move[entity].move == Idling || world->move[entity].move == Staggered)
			continue;
		world->timing[entity].frames++;

		// If an attack lands this frame
		if (world->timing[entity].frames == ATTACK_FRAMES && isAttacking(world, entity)) {
			// Attempt to hit

			// Guarding
			if (world->move[world->timing[entity].facing].move == Guarding) {
				eventSFX = SFX_PARRYGUARD;
				if (world->timing[world->timing[entity].facing].frames > (PARRY_FRAMES / 2)) {
					if (world->move[entity].move == B1)
						DealDamage(world, world->timing[entity].facing, 2, FALSE); // Heavy Attacks deal extra damage to guards.
					else
						DealDamage(world, world->timing[entity].facing, 1, FALSE); // Normal guard
				}
				continue;
			}

			// Parrying
			if (world->move[world->timing[entity].facing].move == Parrying)
				if (world->timing[world->timing[entity].facing].frames < PARRY_FRAMES) {
					eventSFX = SFX_PARRYGUARD;
					world->health[entity].staggered = STAGGERED_FRAMES + 8;
					world->move[entity].move = Staggered;
					world->timing[entity].frames = 0;
					continue;
				}

			// Clean hit
			DealDamage(world, world->timing[entity].facing, 4, TRUE);
			eventSFX = SFX_HIT;
			if (world->health[world->timing[entity].facing].points == 0)
				setMove(world, world->timing[entity].facing, Dying);
			else {
				world->move[world->timing[entity].facing].move = Staggered;
				world->health[world->timing[entity].facing].staggered = STAGGERED_FRAMES;
			}
		}

		// Character finished dying animation -> switch char / end game
		if (world->move[entity].move == Dying && world->timing[entity].frames == DEATH_FRAMES)
		{
			if (entity == currentPlayer)	 return; // Dev kit project will react at Game Over
			if (entity == 0) return; // Dev Kit project will react appropriately at Stage Clear.

			destroyEntity(world, entity);
			entity = --world->timing[currentPlayer].facing;
			DoIdle(world, entity);
			world->timing[entity].facing = currentPlayer;
			return; // Don't loop over entities anymore.
		}
	}

}


EventQueue renderSystem(World *world) {
	EventQueue eventQueue;
	eventQueue.sfx = eventSFX;
	eventSFX = 0; // Set to not SFX for next frame.

	eventQueue.animation[0] = world->move[currentPlayer].move;
	eventQueue.animation[1] = world->move[world->timing[currentPlayer].facing].move;
	eventQueue.spriteSheet[0] = world->move[currentPlayer].spriteData;
	eventQueue.spriteSheet[1] = world->move[world->timing[currentPlayer].facing].spriteData;
	return eventQueue;
}


// Static (private) helper functions

/*! Returns 1 (TRUE) if the entity can chain a previous attack into another attack.

	When attacking, some frames have to play before the hit lands.
	Following the hit, within some frames the entity can chain the previous attack into another attack.
	If this function is called within those frames, the function returns true.

	\param *world The game state as a World structure.
	\param entity Entity slot attempting to chain attack.
	\return 1 (TRUE) if the entity can chain a previous attack into another attack.
	*/
static u8 timedRight(World *world, u8 entity) {
	if (ATTACK_FRAMES < world->timing[entity].frames && world->timing[entity].frames < (ATTACK_FRAMES + FOLLOWUP_FRAMES))
		return TRUE;
	return FALSE;
}


/*! Returns 1 (TRUE) if the entity is currently attacking.
	\param *world The game state as a World structure.
	\param entity Entity in question.
	\return 1 (TRUE) if the entity is currently attacking
*/
static u8 isAttacking(World *world, u8 entity) {
	switch (world->move[entity].move)
	{
	case Idling: case Guarding: case Parrying: case Staggered: case Dying:
		return FALSE;
	default:
		return TRUE;
	}
}


/*! Sets the move and frame data for an entity. Used by DoXYZ functions below.
\param *world The game state as a World structure.
\param entity Entity whose move to set.
\param move A move set by the AttackType enumeration.
\return void
*/
static void setMove(World *world, u8 entity, u8 move) {
	world->timing[entity].frames = 0;
	world->move[entity].move = move;
}


/*! Finds and returns the entity slot of the next playable character.
\param *world The game state as a World structure.
\param entity The entity slot of the current player character.
\return The entity slot in the World structure of the next playable character.
*/
static u8 searchForNextPlayerCharacter(World *world, u8 entity) {
	// If not last reserve character, pick next character in line.
	if (world->teamMember[entity + 1].id == (world->teamMember[entity].id + 1))
		if (world->mask[entity + 1] != COMPONENT_NONE)
			return (entity + 1);

	// If last reserve character, rotate and pick first character.
	for (u8 iterator = entity; entity > 0; iterator--) {
		if (iterator == 1)
			return 1;
		if (world->teamMember[iterator].id == 0)
			return iterator;
	}

	return 255; // Failure
}


/*! Has the entity go idle.
\param *world The game state as a World structure.
\param entity The entity to idle.
\return void
*/
static void DoIdle(World *world, u8 entity) {
	world->move[entity].move = Idling;
	world->timing[entity].frames = 0;
}


/*! Has the entity perform a basic attack.
\param *world The game state as a World structure.
\param entity The entity slot to initiate an attack.
\return void
*/
static void DoBasicAttack(World *world, u8 entity) {
	if (world->move[entity].move == Idling)
		world->move[entity].move = A1;
	else if (timedRight(world, entity) && isAttacking(world, entity)) {
		switch (world->move[entity].move)
		{
		case A1:
			setMove(world, entity, A2);
			break;
		case A2:
			setMove(world, entity, A3);
			break;
		case A3:
			setMove(world, entity, A1);
		default:
			break;
		}
	}

	if (world->timing[entity].frames == 0 && isAttacking(world, entity))
		eventSFX = SFX_SWING;
}


/*! Has the entity perform a special attack. Type depends on previous basic attacks, if any.
\param *world The game state as a World structure.
\param entity The entity slot to initiate an attack.
\return void
*/
static void DoSpecialAttack(World *world, u8 entity) {
	if (world->move[entity].move == Idling)
		world->move[entity].move = B1; // Heavy
	else if (timedRight(world, entity) && isAttacking(world, entity)) {
		eventSFX = SFX_SWING;
		switch (world->move[entity].move)
		{
		case A1:
			setMove(world, entity, B2); // Chain
			break;
		case A2:
			setMove(world, entity, B3); // Finisher
			break;
		case A3:
			setMove(world, entity, B1); // Heavy
		default:
			break;
		}
	}

	if (world->timing[entity].frames == 0 && isAttacking(world, entity))
		eventSFX = SFX_SWING;
}


/*! Switches to the next player character if possible (player characters only)
\param *world The game state as a World structure.
\param entity Entity slot of the current player character.
\return void
*/
static void DoCharacterSwitch(World *world, u8 entity) {
	// Search for the next available player character
	u8 nextChar = searchForNextPlayerCharacter(world, entity);

	// Only do something if there's a reserve player character.
	if (world->teamMember[entity].id != world->teamMember[nextChar].id) {
		if (world->move[entity].move == Idling) {
			DoIdle(world, nextChar);
			world->health[nextChar].staggered = STAGGERED_FRAMES;
			world->move[nextChar].move = Staggered;
		}
		else if (timedRight(world, entity) && world->move[entity].move == B2)
			world->move[nextChar].move = A3; // Chain Attack into Character Switch.
		else
			return; // If occupied, character cannot switch.
		world->teamMember[entity].isActive = FALSE;
		world->teamMember[nextChar].isActive = TRUE;
		world->timing[nextChar].frames = 0;
		world->timing[nextChar].facing = world->timing[entity].facing;
		currentPlayer = nextChar;
	}
}


/*! Has an entity perform a parry.
\param *world The game state as a World structure.
\param entity The entity to parry.
\return void
*/
static void DoParry(World *world, u8 entity) {
	if (world->move[entity].move == Idling)
		world->move[entity].move = Parrying;
}


/*! Has an entity assume a guarding stance.
\param *world The game state as a World structure.
\param entity Entity to guard.
\return void
*/
static void DoGuard(World *world, u8 entity) {
	if (world->move[entity].move == Idling)
		world->move[entity].move = Guarding;
}


/////*! UNUSED
////\param *world ECS World
////\param entity affected entity
////\return void
////*/
////static void DoEvade(World *world, u8 entity) { };


/*! Deals damage dealt to an enemy and determines fatality.

	Dealing damage to an entity reduces their hit points (Health structure).
	If 'isFatal' is set to 1 (TRUE), the attack is lethal and can defeat.
\param *world The game state as a World structure.
\param entity The entity that receives damage.
\param damage Number of hit points to remove.
\param isFatal set to 1 (TRUE) if the blow can defeat the entity.
\return void
*/
static void DealDamage(World *world, u8 entity, u8 damage, u8 isFatal) {
	if (world->health[entity].points > damage)
		world->health[entity].points -= damage;
	else
		world->health[entity].points = (isFatal) ? 0 : 1;
}

#endif // !ECS_SYSTEMS