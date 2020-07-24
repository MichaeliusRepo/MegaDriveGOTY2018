/*!
\file main.c
\brief Engine file
\author Michael Atchapero
\date 06/2018

Engine file of the project. Contains entry point in main().
*/

#include "inc\main.h"
#include "src\Model.c"


#define MAX_ENEMIES 12  /*!< Game will spawn this number of enemies.*/
#define MAX_ALLIES 4   /*!< Maximum allowed allies.*/

#define GOTO_COURTYARD 7	/*!< Entity slot index of last enemy in forest area before moving on to courtyard area. */ 
#define GOTO_GREAT_HALL 3	/*!< Entity slot index of last enemy in courtyard area before moving on to mansion interior area.*/


#define DEFAULT_PLAYER_HEALTH 25	/*!< HP of each player character. Enemy default is 10. */

Sprite* sprites[2];		/*!< Pointer of sprites */
SpriteSheet currentSpriteSheet[2];		/*!<  Spritesheet of player and enemy characters.  */
u16 palette[64];	/*!< A seperate palette used for fade effects. */
Screen currentScreen;	/*!< Keeps track of which screen is currently in use. */
World ECSWorld;	/*!< Game state by World structure. */
EventQueue globalQueue;		/*!<  Model writes here which SFX and animations to play. */
ButtonInput buttonInput;		/*!<  Input function writes here what buttons were pressed. */
u8 AILevel;		/*!<  Game difficulty here is set by difficulty select screen. */
u8 numAllies;			/*!<  Number of allies in-game is set by difficulty select screen. */

int main() {
	currentScreen = StartScreen;
	JOY_init();
	JOY_setEventHandler(&joyHandler); // Set joyHandler function as input function.

	// While this is the entire program loop, the functions have subloops.
	while (TRUE) {
		if (currentScreen == StartScreen)
			startScreen();
		else if (currentScreen == DifficultySelect)
			selectDifficulty();
		else 	if (currentScreen == InGame)
			startGame();
	}
	return 0;
}



void startScreen() {
	XGM_startPlay(heldonlyonce_music);
	setBackground(splash1_image, splash2_image);
	VDP_fadeIn(0, (4 * 16) - 1, palette, 60, FALSE); // fade in
	JOY_waitPressBtnTime(10 * 1000); // Wait 10 seconds or press button

	VDP_fadeOutAll(60, FALSE);
	VDP_clearPlan(PLAN_A, FALSE);
	VDP_clearPlan(PLAN_B, FALSE);
	currentScreen = DifficultySelect;
}



void selectDifficulty() {
	// setBackground uses 2 plans,
	// while here we only need one plan set because of text.
	// Therefore don't use setBackground(...) here, as this is a special case.

	VDP_resetScreen();
	u16 ind; // tile user index
	SYS_disableInts(); //disable interrupt when accessing VDP

	VDP_setPaletteColors(0, (u16*)palette_black, 64); // Sets all palette to black

	ind = TILE_USERINDEX;
	VDP_drawImageEx(PLAN_B, &stageSelect_image, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, ind), 0, 0, TRUE, TRUE);
	ind += stageSelect_image.tileset->numTile;

	VDP_setPaletteColor(15, 0xEEE); // Sets text color to white.
	SYS_enableInts(); // VDP process done, re-enable interrupts

	// prepare tile palettes
	palette[15] = RGB24_TO_VDPCOLOR(0xEEEEEE); // Set text colour to white.
	memcpy(&palette[16], stageSelect_image.palette->data, 16 * 2);
	VDP_fadeIn(0, (4 * 16) - 1, palette, 60, FALSE); // fade in

	VDP_drawTextBG(PLAN_B, "LEFT/RIGHT to select difficulty", 4, 5);
	VDP_drawTextBG(PLAN_B, "UP/DOWN to select number of allies", 3, 15);


	// Stage selection code (as opposed to backgrounds, tiles, text).
	u8 localAILevel;
	localAILevel = AILevel = numAllies = 1; // Set all values to 1.
	while (currentScreen == DifficultySelect) {
		VDP_clearPlan(PLAN_A, TRUE);
		switch (AILevel)
		{
		case 0:
			VDP_drawTextBG(PLAN_A, "Easy", 17, 8);
			localAILevel = DIFF_EASY;
			break;

		case 1:
			VDP_drawTextBG(PLAN_A, "Normal", 16, 8);
			localAILevel = DIFF_NORMAL;
			break;

		case 2:
			VDP_drawTextBG(PLAN_A, "Hard", 17, 8);
			localAILevel = DIFF_HARD;
			break;

		case 3:
			VDP_drawTextBG(PLAN_A, "Nightmare?", 14, 8);
			localAILevel = DIFF_NIGHTMARE;
			break;

		case 4:
			VDP_drawTextBG(PLAN_A, "PERFECT!", 15, 8);
			localAILevel = DIFF_PERFECT;
			break;

		default:
			break;
		}

		// Bad coding, I know. I don't know how to convert u16 to string. I know there is a SGDK function for it.
		if (numAllies == 1) VDP_drawTextBG(PLAN_A, "Allies: 1", 15, 17);
		else if (numAllies == 2) VDP_drawTextBG(PLAN_A, "Allies: 2", 15, 17);
		else if (numAllies == 3) VDP_drawTextBG(PLAN_A, "Allies: 3", 15, 17);
		else if (numAllies == 4) VDP_drawTextBG(PLAN_A, "Allies: 4", 15, 17);

		VDP_waitVSync();
	}
	XGM_stopPlay();

	VDP_fadeOutAll(20, FALSE);
	VDP_clearPlan(PLAN_A, FALSE);
	VDP_clearPlan(PLAN_B, FALSE);

	VDP_setPaletteColors(0, (u16*)palette_black, 64); // Sets all palette to black
	VDP_setPaletteColor(15, 0xEEE); // Sets text color to white.

	AILevel = localAILevel;
	currentScreen = InGame;
}



void startGame() {
	initializeModel();
	initializeMegaDrive();

	while (currentScreen == InGame)
	{
		randSGDK = random(); // AI uses random values from SGDK.
		difficultyAIaccumulator += AILevel;
		globalQueue = updateWorld(&ECSWorld, &buttonInput);
		checkProgression(); // inquires game state to cause events
		updateAnim(); // move sprites
		SPR_update(); // draw current screen
		VDP_waitVSync(); // wait for refresh
	}

	SYS_reset(); // Soft reset.
}



void initializeModel() {
	buttonInput.isPressed = FALSE;

	// sprite sheets are ordered in drawing order,
	// so that's why enemy1 is slot 0 in ECS and slot 1 in drawing order.
	currentSpriteSheet[0] = mockPlayer1;
	currentSpriteSheet[1] = mockEnemy;

	// Always clear slots for entities before creating them.
	destroyAllEntities(&ECSWorld);

	u8 i;
	for (i = 0; i < MAX_ENEMIES; ++i)
		createEnemyChar(&ECSWorld, currentSpriteSheet[1]);
	ECSWorld.health[0].points = 25; // Last enemy gets extra health because he's a boss.

	for (i = 0; i < numAllies; ++i)
		if (i % 2)
			createPlayerChar(&ECSWorld, mockPlayer2, i);
		else
			createPlayerChar(&ECSWorld, mockPlayer1, i);

	for (i = 0; i < numAllies; ++i)
		ECSWorld.health[MAX_ENEMIES + i].points = 25; // MAX_ENEMIES is also the index of first player character.
}



void initializeMegaDrive() {
	VDP_resetScreen();
	SYS_disableInts(); //disable interrupt when accessing VDP

	//init SFX
	XGM_setPCM(SFX_HIT, hit_sfx, sizeof(hit_sfx));
	XGM_setPCM(SFX_PARRYGUARD, parry_sfx, sizeof(parry_sfx));
	XGM_setPCM(SFX_SWING, swing_sfx, sizeof(swing_sfx));
	// start music
	XGM_startPlay(rosenroede_music);

	SPR_init(0, 0, 0); // 0 here means default values. Space for 40 sprites will be allocated.

	setBackground(forest1_image, forest0_image);

	// init sprite
	spawnSprites(FALSE); // also prepares sprite palettes
	SPR_setAnim(sprites[0], 0);
	SPR_setAnim(sprites[1], 0);
	SPR_update();

	VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE); // fade in
}



void gameOver() {
	VDP_resetScreen();

	if (ECSWorld.health[currentPlayer].points == 0)
		VDP_drawText("GAME OVER", 16, 5);
	else
		VDP_drawText("STAGE CLEAR!", 13, 5);

	spawnSprites(TRUE);

	waitMs(5 * 1000); // Let 5 seconds pass
	JOY_waitPressBtnTime(5 * 1000); // Wait 5 more seconds or press button

	SYS_reset();
}



void joyHandler(u16 joy, u16 changed, u16 state) {
	if (joy == JOY_1) {

		// STAGE SELECT
		if (currentScreen == DifficultySelect) {
			if (state & BUTTON_START)
				currentScreen = InGame;
			else if (state & BUTTON_C)
				currentScreen = StartScreen;
			else if (state & BUTTON_LEFT)
				AILevel = (AILevel + 4) % 5; // using -1 causes overflow (underflow?)
			else if (state & BUTTON_RIGHT)
				AILevel = (++AILevel) % 5;
			else if (state & BUTTON_DOWN)
				--numAllies;
			else if (state & BUTTON_UP)
				++numAllies;

			if (numAllies == 0) ++numAllies;
			else if (numAllies > MAX_ALLIES) --numAllies;
			return;
		}

		// IN GAME
		else {
			buttonInput.isPressed = (state) ? TRUE : FALSE;

			// Not the most elegant coding.
			if (changed & BUTTON_START)
				buttonInput.latestButtonPress = Start;
			else if (changed & BUTTON_MODE)
				currentScreen = StartScreen; // Resets game.
			else if (changed & BUTTON_A)
				buttonInput.latestButtonPress = A;
			else if (changed & BUTTON_B)
				buttonInput.latestButtonPress = B;
			else if (changed & BUTTON_C)
				buttonInput.latestButtonPress = C;
			else if (changed & BUTTON_UP)
				buttonInput.latestButtonPress = Up;
			else if (changed & BUTTON_DOWN)
				buttonInput.latestButtonPress = Down;
			else if (changed & BUTTON_LEFT)
				buttonInput.latestButtonPress = Left;
			else if (changed & BUTTON_RIGHT)
				buttonInput.latestButtonPress = Right;
			else
				buttonInput.latestButtonPress = Neutral;
		}

		// The most accurate joyHandling is found in the Input Sample.
	}
}



void spawnSprites(u8 setPAL) {
	SpriteDefinition pickedSprite;

	// Select correct player character
	switch (currentSpriteSheet[0])
	{
	case mockPlayer1:
		pickedSprite = mockPlayer_sprite;
		sprites[0] = SPR_addSprite(&mockPlayer_sprite, fix32ToInt(FIX32(100)), fix32ToInt(FIX32(125)), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
		break;

	case mockPlayer2:
		pickedSprite = mockPlayer2_sprite;
		sprites[0] = SPR_addSprite(&mockPlayer2_sprite, fix32ToInt(FIX32(100)), fix32ToInt(FIX32(125)), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
		break;

	default:
		break;
	}

	// Load player palette into sprite engine
	if (setPAL)
		VDP_setPalette(PAL2, pickedSprite.palette->data);
	memcpy(&palette[32], pickedSprite.palette->data, 16 * 2);

	// Select the correct enemy sprite
	switch (currentSpriteSheet[1])
	{
	case mockEnemy:
		pickedSprite = mockEnemy_sprite;
		sprites[1] = SPR_addSprite(&mockEnemy_sprite, fix32ToInt(FIX32(110)), fix32ToInt(FIX32(100)), TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
		break;

	default:
		break;
	}

	// Load enemy palette into sprite engine
	if (setPAL)
		VDP_setPalette(PAL3, pickedSprite.palette->data);
	memcpy(&palette[48], pickedSprite.palette->data, 16 * 2);
}



void updateAnim() {
	// If spritesheet changed, reset sprite engine.
	if (globalQueue.spriteSheet[0] != currentSpriteSheet[0] || globalQueue.spriteSheet[1] != currentSpriteSheet[1]) {
		currentSpriteSheet[0] = globalQueue.spriteSheet[0];
		currentSpriteSheet[1] = globalQueue.spriteSheet[1];
		SPR_reset();
		SPR_clear();
		spawnSprites(TRUE);
	}

	// Pick animation according to current move.
	SPR_setAnim(sprites[0], globalQueue.animation[0]);
	SPR_setAnim(sprites[1], globalQueue.animation[1]);

	// Play SFX
	if (globalQueue.sfx != 0)
		XGM_startPlayPCM(globalQueue.sfx, 1, SOUND_PCM_CH2);
}



void setBackground(const Image PlanA_img, const Image PlanB_img) {
	u16 ind; // tile user index

	if (SYS_isInInterrupt())
		SYS_disableInts(); //disable interrupt when accessing VDP
	VDP_setPaletteColors(0, (u16*)palette_black, 64); // Sets all palette to black

	ind = TILE_USERINDEX;
	VDP_drawImageEx(PLAN_A, &PlanA_img, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
	ind += PlanA_img.tileset->numTile;
	VDP_drawImageEx(PLAN_B, &PlanB_img, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
	ind += PlanB_img.tileset->numTile;

	SYS_enableInts(); // VDP process done, re-enable interrupts

	// prepare tile palettes
	memcpy(&palette[0], PlanA_img.palette->data, 16 * 2);
	memcpy(&palette[16], PlanB_img.palette->data, 16 * 2);
}



void checkProgression() {
	// Triggers 'Stage Clear!', or 'Game Over', or transitions backgrounds when appropriate.

	// If a player character died, Game Over.
	if (ECSWorld.move[currentPlayer].move == Dying && ECSWorld.timing[currentPlayer].frames == DEATH_FRAMES)
		gameOver();
	// If last enemy died, then Stage Clear! (First slot is always final enemy.)
	if (ECSWorld.move[0].move == Dying && ECSWorld.timing[0].frames == DEATH_FRAMES)
		gameOver();

	// From forest to courtyard
	if (ECSWorld.move[GOTO_COURTYARD].move == Dying && ECSWorld.timing[GOTO_COURTYARD].frames == DEATH_FRAMES) {
		VDP_fadeOutAll(20, FALSE);

		SYS_disableInts(); //disable interrupt when accessing VDP.
		VDP_resetScreen();
		SPR_init(0, 0, 0); // 0 here means default values.
		SPR_reset();
		//SND_startPlay_XGM(sonic_music); 		// start music
		setBackground(courtyard0_image, courtyard1_image);

		// init sprite
		spawnSprites(FALSE); // also prepares sprite palettes
		SPR_setAnim(sprites[0], 0);
		SPR_setAnim(sprites[1], 0);
		SPR_update();
		VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE); // fade in
	}

	if (ECSWorld.move[GOTO_GREAT_HALL].move == Dying && ECSWorld.timing[GOTO_GREAT_HALL].frames == DEATH_FRAMES) {
		VDP_fadeOutAll(20, FALSE);

		SYS_disableInts(); //disable interrupt when accessing VDP.
		VDP_resetScreen();
		SPR_init(0, 0, 0); // 0 here means default values.
		SPR_reset();
		//SND_startPlay_XGM(sonic_music); 		// start music
		setBackground(greatHall0_image, greatHall1_image);

		// init sprite
		spawnSprites(FALSE); // also prepares sprite palettes
		SPR_setAnim(sprites[0], 0);
		SPR_setAnim(sprites[1], 0);
		SPR_update();
		VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE); // fade in
	}
}