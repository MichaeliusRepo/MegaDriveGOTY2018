#include "stdafx.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

// Include any source files directly here. Don't include anything in stdafx.h.
#include "../MegaDriveGOTY2018/Gemu/src/Model.c"
#include "../MegaDriveGOTY2018/Gemu/src/Entities.c"
#include "../MegaDriveGOTY2018/Gemu/src/Systems.c"



// If .c files are included in multiple .obj files, this will cause linking errors in Visual Studio.
// Long story short, distributing unit tests over multiple files causes errors.
// Yes, I am disgusted that I must include all my unit tests in one file.

namespace TestProject1337
{
	World world;
	EventQueue queue;
	ButtonInput buttonInput;

	u8 enemy1;
	u8 player1;

	[TestClass]
	public ref class UnitTest
	{
	private:
		TestContext ^ testContextInstance;
	public:
		/// <summary>
		///Gets or sets the test context which provides
		///information about and functionality for the current test run.
		///</summary>
		property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
		{
			Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
			{
				return testContextInstance;
			}
			System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
			{
				testContextInstance = value;
			}
		};

#pragma region Additional test attributes
		//
		//You can use the following additional attributes as you write your tests:
		//
		//Use ClassInitialize to run code before running the first test in the class
		//[ClassInitialize()]
		//static void MyClassInitialize(TestContext^ testContext) {};
		//
		//Use ClassCleanup to run code after all tests in a class have run
		//[ClassCleanup()]
		//static void MyClassCleanup() {};
		//
		//Use TestInitialize to run code before running each test
		//[TestInitialize()]
		//void MyTestInitialize() {};
		//
		//Use TestCleanup to run code after each test has run
		//[TestCleanup()]
		//void MyTestCleanup() {};
		//
#pragma endregion 



		[TestInitialize()]
		void MyTestInitialize() {
			destroyAllEntities(&world);

			// In practice, set stages should have a set number of enemies,
			// and the number of players varies.
			// [0..n] are enemies, [n-m] are teammates, where
			// n are # of enemies and m are # of teammates.
			enemy1 = createEnemyChar(&world, mockEnemy);
			player1 = createPlayerChar(&world, mockPlayer1, 0);

			//world.timing[player1].facing = enemy1; // set to lowest AI entity ID
			//world.timing[enemy1].facing = player1; // set to lowest player entity ID

			buttonInput = { FALSE, Neutral };
		};

		[TestMethod]
		void TestCreateWorld()
		{
			// *w is a pointer. &w is the opposite.
			u8 updatesToRun = 1;

			for (int i = 0; i < updatesToRun; ++i)
				queue = updateWorld(&world, &buttonInput);

			destroyEntity(&world, 0);
			u8 empty = nextEmptyEntitySlot(&world);
			Assert::AreEqual((u16)0, world.mask[0]);

		};

		[TestMethod]
		void TestCase1MainScenario() {
			// No moves are being made currently
			Assert::AreEqual((u8)10, world.health[player1].points);
			Assert::AreEqual((u8)10, world.health[enemy1].points);
			Assert::AreEqual((u16)0, world.timing[player1].frames);
			Assert::AreEqual((u8)Idling, world.move[player1].move);

			queue = updateWorld(&world, &buttonInput);

			// Still no moves are being made currently
			Assert::AreEqual((u8)10, world.health[player1].points);
			Assert::AreEqual((u8)10, world.health[enemy1].points);
			Assert::AreEqual((u8)Idling, world.move[player1].move);
			Assert::AreEqual((u16)0, world.timing[player1].frames);


			// Press A button.
			buttonInput = { TRUE, A };
			queue = updateWorld(&world, &buttonInput);

			// Fast attack is executing.
			Assert::AreEqual((u16)1, world.timing[player1].frames);
			Assert::AreEqual((u8)A1, world.move[player1].move);
			// HP still at maximum.
			Assert::AreEqual((u8)10, world.health[enemy1].points);

			buttonInput = { FALSE, A };

			// Let first move's frames pass.
			for (u8 i = 1; i < ATTACK_FRAMES; ++i) {
				Assert::AreEqual((u16)i, world.timing[player1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			//On the move's last frame, enemy receives clean hit losing 4 HP.
			Assert::AreEqual((u8)6, world.health[enemy1].points);

			for (u8 i = ATTACK_FRAMES; i <= (ATTACK_FRAMES + FOLLOWUP_FRAMES); ++i) {
				Assert::AreEqual((u16)i, world.timing[player1].frames);
				Assert::AreEqual((u8)1, world.move[player1].move); // Fast attack still active.
				queue = updateWorld(&world, &buttonInput);
			}

			// On the next frame, reset to no attack executing.
			queue = updateWorld(&world, &buttonInput);
			Assert::AreEqual((u8)Idling, world.move[player1].move);
			Assert::AreEqual((u16)0, world.timing[player1].frames);

			Assert::AreEqual((u8)10, world.health[player1].points);
			Assert::AreEqual((u8)6, world.health[enemy1].points);

		}

		[TestMethod]
		void TestCase1Alt1_1() {
			buttonInput = { TRUE, A };

			queue = updateWorld(&world, &buttonInput);
			buttonInput = { FALSE, A };

			for (u8 i = 1; i <= ATTACK_FRAMES; ++i)
				queue = updateWorld(&world, &buttonInput);

			Assert::AreEqual((u8)A1, world.move[player1].move);
			Assert::AreEqual((u8)6, world.health[enemy1].points);

			buttonInput = { TRUE, A };
			queue = updateWorld(&world, &buttonInput);
			buttonInput = { FALSE, A };

			Assert::AreEqual((u8)A2, world.move[player1].move);
			Assert::AreEqual((u8)6, world.health[enemy1].points);

			for (u8 i = 1; i <= ATTACK_FRAMES; ++i)
				queue = updateWorld(&world, &buttonInput);

			Assert::AreEqual((u8)A2, world.move[player1].move);
			Assert::AreEqual((u8)2, world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase1Alt1_2() {
			buttonInput = { TRUE, A }; // Just hold the button down forever.
			world.health[enemy1].points = 31; // some high value

			Assert::AreEqual((u8)Idling, world.move[player1].move);
			queue = updateWorld(&world, &buttonInput);

			//Do A1->A2->A3->A1... combo 6 times.
			for (int j = 0; j < 2; j++) {

				for (int i = 0; i <= ATTACK_FRAMES; i++) {
					Assert::AreEqual((u8)A1, world.move[player1].move);
					queue = updateWorld(&world, &buttonInput);
				}

				for (int i = 0; i <= ATTACK_FRAMES; i++) {
					Assert::AreEqual((u8)A2, world.move[player1].move);
					queue = updateWorld(&world, &buttonInput);
				}

				for (int i = 0; i <= ATTACK_FRAMES; i++) {
					Assert::AreEqual((u8)A3, world.move[player1].move);
					queue = updateWorld(&world, &buttonInput);
				}
			}

			u8 enemyHPLoss = 2 * 3 * 4;
			Assert::AreEqual((u8)(31 - enemyHPLoss), world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase2MainScenario() {
			buttonInput = { TRUE, B };

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)10, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)B1, world.move[player1].move);
			}

			// After 24 frames, enemy loses 4 HP.
			Assert::AreEqual((u8)6, world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase2Alt1() {
			buttonInput = { TRUE, A };

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)10, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);
			}

			buttonInput = { TRUE, B };
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)6, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)B2, world.move[player1].move);
			}

			// After 2 hits, enemy should've lost 8 HP.
			Assert::AreEqual((u8)2, world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase2Alt2() {
			buttonInput = { TRUE, A };

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)10, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);
			}

			buttonInput = { TRUE, A };
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)6, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A2, world.move[player1].move);
			}

			buttonInput = { TRUE, B };
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)2, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)B3, world.move[player1].move);
			}

			// After 3 hits, enemy should be dead.
			Assert::AreEqual((u8)0, world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase2Alt3() {
			world.health[enemy1].points = 31; // some high value

			buttonInput = { TRUE, A };

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)31, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);
			}

			buttonInput = { TRUE, A };
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)(31 - 4), world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A2, world.move[player1].move);
			}

			buttonInput = { TRUE, A };
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)(31 - 8), world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A3, world.move[player1].move);
			}

			buttonInput = { TRUE, B };
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)(31 - 12), world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)B1, world.move[player1].move);
			}

			// After 4 hits, enemy should've lost 16 HP.
			Assert::AreEqual((u8)(31 - 16), world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase3MainScenario() {

			buttonInput = { TRUE, C };

			Assert::IsTrue(world.teamMember[player1].isActive);

			for (u8 i = 0; i < 30; ++i)
				queue = updateWorld(&world, &buttonInput);

			// With only 1 player character, character switch does nothing.
			Assert::IsTrue(world.teamMember[player1].isActive);
			Assert::AreEqual(player1, searchForNextPlayerCharacter(&world, player1));

			// Create 2nd character.
			u8 player2 = createPlayerChar(&world, mockPlayer2, 1);
			Assert::AreEqual(player2, searchForNextPlayerCharacter(&world, player1));
			Assert::AreEqual(player1, searchForNextPlayerCharacter(&world, player2));

			Assert::IsTrue(world.teamMember[player1].isActive);

			for (u8 i = 0; i < STAGGERED_FRAMES; ++i) {
				queue = updateWorld(&world, &buttonInput);
				Assert::IsFalse(world.teamMember[player1].isActive);
				Assert::IsTrue(world.teamMember[player2].isActive);
			}

			for (u8 i = 0; i < STAGGERED_FRAMES; ++i) {
				queue = updateWorld(&world, &buttonInput);
				Assert::IsTrue(world.teamMember[player1].isActive);
				Assert::IsFalse(world.teamMember[player2].isActive);
			}

			Assert::AreEqual(enemy1, world.timing[player2].facing);

			// Create third character
			u8 player3 = createPlayerChar(&world, mockPlayer2, 2);
			Assert::AreEqual(player2, searchForNextPlayerCharacter(&world, player1));
			Assert::AreEqual(player3, searchForNextPlayerCharacter(&world, player2));
			Assert::AreEqual(player1, searchForNextPlayerCharacter(&world, player3));

			for (u8 i = 0; i < STAGGERED_FRAMES; ++i) {
				queue = updateWorld(&world, &buttonInput);
				Assert::IsTrue(world.teamMember[player2].isActive);
			}

			for (u8 i = 0; i < STAGGERED_FRAMES; ++i) {
				queue = updateWorld(&world, &buttonInput);
				Assert::IsTrue(world.teamMember[player3].isActive);
			}

			//queue = updateWorld(&world, &buttonInput);
			for (u8 i = 0; i < STAGGERED_FRAMES; ++i) {
				queue = updateWorld(&world, &buttonInput);
				Assert::IsTrue(world.teamMember[player1].isActive);
			}

			Assert::AreEqual(enemy1, world.timing[player3].facing);
		}

		[TestMethod]
		void TestCase3Alt1() {

			world.health[enemy1].points = 31;
			u8 player2 = createPlayerChar(&world, mockPlayer2, 1);

			buttonInput = { TRUE, A };

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)31, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);
			}

			buttonInput = { TRUE, B };
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)(31 - 4), world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)B2, world.move[player1].move);
			}

			buttonInput = { TRUE, C };
			queue = updateWorld(&world, &buttonInput);
			// Need one more iteration to switch to next character.
			queue = updateWorld(&world, &buttonInput);
			Assert::IsTrue(world.teamMember[player2].isActive);

			// Note: one less iteration in this loop, i = 1
			for (u8 i = 1; i < ATTACK_FRAMES; i++) {
				Assert::IsTrue(world.teamMember[player2].isActive);
				Assert::AreEqual((u8)(31 - 8), world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A3, world.move[player2].move);
			}

			// After 3 hits, enemy should've lost 12 HP.
			Assert::AreEqual((u8)(31 - 12), world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase4MainScenario() {
			buttonInput = { TRUE, A };

			for (u8 i = 1; i < ATTACK_FRAMES; i++) {
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);
				Assert::AreEqual((u8)10, world.health[enemy1].points);
			}

			DoParry(&world, enemy1);
			buttonInput = { FALSE, A };

			// Enemy should've taken no damage because of well-timed parry.
			for (u8 i = 1; i < PARRY_FRAMES; i++) {
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)10, world.health[enemy1].points);
				Assert::AreEqual((u8)Staggered, world.move[player1].move);
			}
		}

		[TestMethod]
		void TestCase4AlternateScenario() {
			buttonInput = { TRUE, A };

			for (u8 i = 0; i < PARRY_FRAMES; i++) {
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);
				Assert::AreEqual((u8)10, world.health[enemy1].points);
			}

			DoParry(&world, enemy1);
			buttonInput = { FALSE, A };

			for (u8 i = 0; i < PARRY_FRAMES; i++) {
				Assert::AreEqual((u8)10, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);

			}

			// Enemy only just missed timing by 1 frame and got hit for 4 HP.
			Assert::AreEqual((u8)6, world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase5MainScenario() {
			DoGuard(&world, enemy1);
			buttonInput = { FALSE, A };

			// Guard for a while
			for (u8 i = 0; i < ATTACK_FRAMES; i++)
				queue = updateWorld(&world, &buttonInput);

			buttonInput = { TRUE, A };

			for (u8 i = 0; i < ATTACK_FRAMES; i++) {
				Assert::AreEqual((u8)10, world.health[enemy1].points);
				Assert::AreEqual((u8)Guarding, world.move[enemy1].move);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);

			}

			// Normal guard shaves off 1 HP.
			Assert::AreEqual((u8)9, world.health[enemy1].points);

		}

		[TestMethod]
		void TestCase5AlternateScenario() {
			buttonInput = { TRUE, A };

			for (u8 i = 1; i < ATTACK_FRAMES; i++) {
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)A1, world.move[player1].move);
				Assert::AreEqual((u8)10, world.health[enemy1].points);
			}

			DoGuard(&world, enemy1);
			buttonInput = { FALSE, A };

			for (u8 i = 1; i < ATTACK_FRAMES; i++) {
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)10, world.health[enemy1].points);
			}

			// Enemy should've taken no damage because of well-timed guard.
			Assert::AreEqual((u8)10, world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase7MainScenario() {

			DoBasicAttack(&world, enemy1);

			// Let first move's frames pass.
			for (u8 i = 0; i < ATTACK_FRAMES; ++i) {
				Assert::AreEqual((u16)i, world.timing[enemy1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			//On the move's last frame, player receives clean hit losing 4 HP.
			Assert::AreEqual((u8)6, world.health[player1].points);

			// Let time pass
			for (u8 i = 0; i <= ATTACK_FRAMES; ++i)
				queue = updateWorld(&world, &buttonInput);

			// Attack again
			DoBasicAttack(&world, enemy1);

			// Let move frames pass.
			for (u8 i = 0; i < (ATTACK_FRAMES - 1); ++i) {
				Assert::AreEqual((u16)i, world.timing[enemy1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			//DoParry(&world, player1);
			buttonInput = { TRUE, Up };

			// Player parried successfully and won't receive damage.
			for (u8 i = 0; i < (PARRY_FRAMES); ++i) {
				Assert::AreEqual((u8)6, world.health[player1].points);
				queue = updateWorld(&world, &buttonInput);
			}
		}

		[TestMethod]
		void TestCase8MainScenario() {

			DoSpecialAttack(&world, enemy1);

			// Let first move's frames pass.
			for (u8 i = 0; i < ATTACK_FRAMES; ++i) {
				Assert::AreEqual((u16)i, world.timing[enemy1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			//On the move's last frame, player receives clean hit losing 4 HP.
			Assert::AreEqual((u8)6, world.health[player1].points);

			// Let time pass
			for (u8 i = 0; i <= ATTACK_FRAMES; ++i)
				queue = updateWorld(&world, &buttonInput);

			// Attack again
			DoSpecialAttack(&world, enemy1);

			// Let move frames pass.
			for (u8 i = 0; i < (ATTACK_FRAMES - 1); ++i) {
				Assert::AreEqual((u16)i, world.timing[enemy1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			DoParry(&world, player1);

			// Player parried successfully and won't receive damage.
			for (u8 i = 0; i < (PARRY_FRAMES); ++i) {
				Assert::AreEqual((u8)6, world.health[player1].points);
				queue = updateWorld(&world, &buttonInput);
			}

			// Let time pass
			for (u8 i = 0; i <= ATTACK_FRAMES; ++i)
				queue = updateWorld(&world, &buttonInput);

			// Attack again
			DoSpecialAttack(&world, enemy1);

			// Context: DoGuard works fine for enemies.
			// But for players, guarding only works while button down is held.
			buttonInput = { TRUE, Down };

			// Let move frames pass.
			for (u8 i = 0; i < (ATTACK_FRAMES); ++i) {
				Assert::AreEqual((u16)i, world.timing[enemy1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			// Blocking Heavy Attacks shaves off 2 HP.
			Assert::AreEqual((u8)4, world.health[player1].points);

			buttonInput = { FALSE, Down };

			queue = updateWorld(&world, &buttonInput);
			queue = updateWorld(&world, &buttonInput);

			Assert::AreEqual((u8)Idling, world.move[player1].move);

		}

		[TestMethod]
		void TestCase9MainScenario() {
			buttonInput = { TRUE, A };

			// Let move frames pass.
			for (u8 i = 0; i < (ATTACK_FRAMES - 1); ++i) {
				Assert::AreEqual((u16)i, world.timing[player1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			DoGuard(&world, enemy1);
			queue = updateWorld(&world, &buttonInput);

			// Enemy guarded perfectly and took no damage.
			Assert::AreEqual((u8)10, world.health[player1].points);
			queue = updateWorld(&world, &buttonInput);
			queue = updateWorld(&world, &buttonInput);

			for (u8 i = 1; i < (ATTACK_FRAMES); ++i) {
				Assert::AreEqual((u16)i, world.timing[player1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			// Player shaves off 1 HP from guarding enemy.
			Assert::AreEqual((u8)9, world.health[enemy1].points);
		}

		[TestMethod]
		void TestCase10MainScenario() {

			DoBasicAttack(&world, player1);

			// Let move frames pass.
			for (u8 i = 0; i < (ATTACK_FRAMES - 1); ++i) {
				Assert::AreEqual((u16)i, world.timing[player1].frames);
				queue = updateWorld(&world, &buttonInput);
			}

			DoParry(&world, enemy1);

			// Enemy parried successfully and won't receive damage.
			for (u8 i = 0; i < (PARRY_FRAMES); ++i) {
				Assert::AreEqual((u8)10, world.health[enemy1].points);
				queue = updateWorld(&world, &buttonInput);
			}
		}

		[TestMethod]
		void TestErrors() {
			destroyAllEntities(&world);
			for (u8 i = 0; i < ENTITY_COUNT; ++i)
				createEnemyChar(&world, mockEnemy);

			Assert::AreEqual((u8)255, nextEmptyEntitySlot(&world));
		}

		[TestMethod]
		void TestPlayerDying() {
			DealDamage(&world, player1, 25, TRUE);
			Assert::AreEqual((u8)0, world.health[player1].points);
			setMove(&world, player1, Dying);

			Assert::AreEqual((u16)0, world.timing[player1].frames);

			for (u16 i = 0; i < DEATH_FRAMES; ++i) {
				Assert::AreEqual((u16)i, world.timing[player1].frames);
				queue = updateWorld(&world, &buttonInput);
			}
		}

		[TestMethod]
		void TestEnemyDying() {
			DealDamage(&world, enemy1, 10, TRUE);
			Assert::AreEqual((u8)0, world.health[enemy1].points);
			setMove(&world, enemy1, Dying);

			Assert::AreEqual((u16)0, world.timing[enemy1].frames);

			for (u16 i = 0; i < DEATH_FRAMES; ++i) {
				Assert::AreEqual((u16)i, world.timing[enemy1].frames);
				queue = updateWorld(&world, &buttonInput);
			}
		}

		[TestMethod]
		void TestEnemySwitch() {
			destroyAllEntities(&world);

			enemy1 = createEnemyChar(&world, mockEnemy);
			u8 enemy2 = createEnemyChar(&world, mockEnemy);
			u8 enemy3 = createEnemyChar(&world, mockEnemy);
			player1 = createPlayerChar(&world, mockPlayer1, 0);
			u8 player2 = createPlayerChar(&world, mockPlayer2, 1);

			Assert::AreEqual(player1, world.timing[enemy3].facing);
			Assert::AreEqual(enemy3, world.timing[player1].facing);

			// Here enemy1 = 0, enemy2 = 1, enemy3 = 2;
			// The first enemy faced is enemy3, which makes enemy1 the last enemy (boss)
			for (int i = enemy3; i >= enemy1; --i) {
				//Using u8 i = enemy3; ... gives memory error for some reason.

				Assert::AreEqual(player1, world.timing[(u8)i].facing);
				Assert::AreEqual((u8)i, world.timing[player1].facing);
				Assert::AreEqual((u8)10, world.health[(u8)i].points);
				Assert::AreEqual((u8)Idling, world.move[(u8)i].move);

				DealDamage(&world, (u8)i, 10, TRUE);
				setMove(&world, (u8)i, Dying);

				for (u8 j = 0; j < DEATH_FRAMES; ++j) {
					Assert::AreEqual((u8)0, world.health[(u8)i].points);
					Assert::AreEqual((u16)j, world.timing[(u8)i].frames);
					Assert::AreEqual((u8)Dying, world.move[(u8)i].move);
					queue = updateWorld(&world, &buttonInput);
				}

				// Enemy should've switched automatically if next is available.
			}



		}

		[TestMethod]
		void TestSwitchNoDamage() {
			// The game had a bug where characters would take damage when defeating an enemy.
			// This test was written to debunk said bug.

			destroyAllEntities(&world);

			enemy1 = createEnemyChar(&world, mockEnemy);
			u8 enemy2 = createEnemyChar(&world, mockEnemy);
			u8 enemy3 = createEnemyChar(&world, mockEnemy);
			u8 enemy4 = createEnemyChar(&world, mockEnemy);
			player1 = createPlayerChar(&world, mockPlayer1, 0);

			Assert::AreEqual(player1, world.timing[enemy4].facing);
			Assert::AreEqual(enemy4, world.timing[player1].facing);

			buttonInput = { TRUE, A };

			while (world.health[enemy4].points != 0) {
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)10, world.health[player1].points);
			}


			while (world.timing[player1].facing == enemy4)
			{
				Assert::AreEqual((u8)Dying, world.move[enemy4].move);
				queue = updateWorld(&world, &buttonInput);
				Assert::AreEqual((u8)10, world.health[player1].points);
			}


		}
	};
}
