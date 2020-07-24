/*!
\file Systems.h
\brief ECS Systems header file
\author Michael Atchapero
\date 06/2018

Systems header file of the ECS architecture.
*/


#ifndef ECS_SYSTEMS_H_
#define ECS_SYSTEMS_H_

#pragma pack(1)

#include "Entities.h"

/*! \brief Time between each animation frame.

	Display frame time span in 1/60 of second.
	This is must be identical to the animation speed when using SGDK's rescomp to compile sprite resources,
	which is found in the sprites.res file in "Resource Compilation" folder.
	This value basically determines game speed, with lower values being faster and vice versa.
	*/	
#define ANIMATION_SPEED 5

#define ATTACK_FRAMES (4*ANIMATION_SPEED) /*!< Number of frames before attack hits. */
#define FOLLOWUP_FRAMES (2*ANIMATION_SPEED) /*!< Number of frames after attack hits. */
#define PARRY_FRAMES (2*ANIMATION_SPEED) /*!< Number of frames a parry can connect with an attack*/
#define STAGGERED_FRAMES (3*ANIMATION_SPEED) /*!< Number of frames a hit entity will be staggered for*/
#define DEATH_FRAMES (6*ANIMATION_SPEED) /*!< Number of frames for a death animation to fully play out*/

#define SFX_HIT 64	/*!< ID for hit sound effect*/
#define SFX_PARRYGUARD 65	 /*!< ID for parry and guard sound effect*/
#define SFX_SWING 66		/*!< ID for swing sound effect*/

	/*! \brief The ID of the sound effect to be played this frame
		
		If value is 0, no sound effect is played this frame.
		Value is initially 0. Various events may set it to a sound effect ID.
		The ID is retrieved by the renderSystem and sent as a return value as part of EventQueue structure.
		Then this value will be set to 0 for the next game update.
	*/
u8 eventSFX;

/*! \brief Every frame the SGDK kit provides a random value for the AI to use.*/
u16 randSGDK;


/*! \brief Enumeration with various buttons*/
typedef enum {
	Neutral,	/**< unused  */
	A,			/**< Button A  */
	B,			/**< Button B  */
	C,			/**< Button C  */
	Left,		/**< DPad Left  */
	Right,	/**< DPad Right */
	Up,			/**< DPad Up  */
	Down,	/**< DPad Down  */
	Start,		/**< Start Button  */
	Select,	/**< Select Button (unused) */
} Button;


/*! \brief Structure with a button and if it is pressed.
	\param isPressed If the button is pressed
	\param latestButtonPress The button most recently pressed
*/
typedef struct {
	u8 isPressed : 1;
	Button latestButtonPress;
} ButtonInput;


/*! \brief Structure with audio to play and graphic to draw
	\param spriteSheet[] Which spritesheet file to read (see SpriteSheet)
	\param animation[] Which animation to play from spritesheet
	\param sfx Sound effect ID to play (is 0 when no sound is to be played)
*/
typedef struct {
	SpriteSheet spriteSheet[2];
	u8 animation[2];
	u8 sfx : 7;
} EventQueue;


/*! \brief Enumeration with moves to execute, ordered the same as the animations on spritesheets. */
typedef enum {
	Idling,		/**< Idle  */
	A1,				/**< First basic attack */
	A2,				/**< Second basic attack */
	A3,				/**< Third basic attack */
	B1,				/**< First special attack (heavy) */
	B2,				/**< Second special attack (chain) */
	B3,				/**< Third special attack */
	Guarding,		/**< Guarding stance */
	Parrying,		/**< Parrying */
	Staggered,		/**< Staggered from surviving a hit */
	Dying				/**< Dying */
} AttackType;


/*! \brief System that makes the player character act by input.

	This system receives some input value from the SGDK project.
	If the game state allows it, the player character may act by the specified input.

	\param *world The game state as a World structure.
	\param *buttonInput The input state as a ButtonInput structure.
	\return void
*/
void inputSystem(World *world, ButtonInput *buttonInput);

/*! \brief System that may make enemy act depending on game state.

	This system will make the enemy character act as opposition to the player.
	The difficulty changes not what decision is made, only how active the enemy is.
	At perfect difficulty level, the AI will react at every single frame.
	Only this system uses randSGDK, a randomly generated value.
	It should be emphasized that the AI is not built through test-driven development. Therefore it has no unit-tests.

	\param *world The game state as a World structure.
	\param difficulty The difficulty as given by the engine.
	\return Potentially modified difficultyAIaccumulator value.
*/
u16 AISystem(World *world, u16 difficulty);

/*! \brief System that enact consequences to character actions.

	This system is built by unit-testing. 	Refer to use cases in appendix to view all logic encompassed by this function.
	Encompasses logic of guarding, parrying, dying, hitting.

	\param *world The game state as a World structure.
	\return void
*/
void combatSystem(World *world);


/*! \brief System that compiles animation and sfx data to return to engine.

	This system reads the game state and collects data such as which animations and sound effects to play,
	and returns them in the EventQueue structure.

	\param *world The game state as a World structure.
	\return EventQueue structure with which animations and sound effects to play.
*/
EventQueue renderSystem(World *world);

#endif /* ECS_SYSTEMS_H_ */
