/*!
\file main.h
\brief Engine header file
\author Michael Atchapero
\date 06/2018

Model header file of the ECS architecture.
*/

#ifndef MAIN_H
#define MAIN_H

// Adjust the inclusion of genesis.h if not compiling properly.

#include "c:\SGDK\inc\genesis.h" // This include makes IntelliSense load genesis.h
#include "genesis.h"
#include "..\res\sprites.h"
#include "..\res\images.h"
#include "..\res\audio.h"

/*! \brief Enumeration with screens, each governing different input responses*/
typedef enum {
	StartScreen,	/**< Intro/Splash screen with artwork */
	DifficultySelect, /**< Difficulty select screen  */
	InGame	/**< Game screen  */
} Screen;


/*! \brief Transition into start screen, showing the splash art.
	\return void
*/
void startScreen();


/*! \brief Transition into difficulty select screen.
\return void
*/
void selectDifficulty();


/*! \brief Transition into game screen.

	The functions runs the game by initializing the model and necessary SGDK functions.
	\return void
*/
void startGame();


/*! \brief Setups parameters for the starting game state.
	\return void
*/
void initializeModel();


/*! \brief Prepares music, background and sprite engine.
\return void
*/
void initializeMegaDrive();


/*! \brief Ends the game. Soft resets game afterwards.
\return void
*/
void gameOver();


/*! \brief Reads input from controller.

	This function is not called manually.
	SGDK/Mega Drive calls this function automatically every frame.
	This is done by setting this as the input function in the main function in main.c.
	The function writes to the ButtonInput structure, which the model can read.
\return void
*/
void joyHandler(u16 joy, u16 changed, u16 state);


/*! \brief Resets the sprite engine and displays character sprites.
	\parameter setPAL Set to 1 (TRUE) if the sprites should load colour palettes.
	\return void
*/
void spawnSprites(u8 setPAL);


/*! \brief Reads EventQueue and displays correct animations. Can also play SFX.
\return void
*/
void updateAnim();


/*! \brief Sets the background to the given two images.
\return void
*/
void setBackground(const Image PlanA_img, const Image PlanB_img);


/*! \brief Reads game state to potentially trigger Game Over or a background change.

	Every frame, this checks whether the player character has won or been defeated.
	If the right number of enemies has been defeated, this will pause the game and change backgrounds.
	\return void
*/
void checkProgression();

#endif // !MAIN_H
