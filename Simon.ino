//#Includes
#include "pitches.h"
//#Definitions
#define CHOICE_OFF      0 //Used to control LEDs
#define CHOICE_NONE     0 //Used to check buttons
#define CHOICE_1  (1 << 0)
#define CHOICE_2  (1 << 1)
#define CHOICE_3  (1 << 2)
#define CHOICE_4  (1 << 3)
#define CHOICE_5  (1 << 4)

#define LED_1  12
#define LED_2  11
#define LED_3  10
#define LED_4  9
#define LED_5  8

#define BUTTON_1    7
#define BUTTON_2    6
#define BUTTON_3    5
#define BUTTON_4    4
#define BUTTON_5    2

#define SPEAKERPIN 13

//Game related definitions
#define MODE_MEMORY 1
#define MODE_BATTLE 2

#define MAX_ROUNDS 15
#define TIME_LIMIT 5000 //Amount of time to press a button before time out

//##Variables
byte gameMode = MODE_MEMORY;
byte gameBoard[32];
byte gameRound;

void setup()
{
  /* add setup code here */
	Serial.begin(9600);
	//Setup inputs with PULL-UPS
	pinMode(BUTTON_1, INPUT_PULLUP);
	pinMode(BUTTON_2, INPUT_PULLUP);
	pinMode(BUTTON_3, INPUT_PULLUP);
	pinMode(BUTTON_4, INPUT_PULLUP);
	pinMode(BUTTON_5, INPUT_PULLUP);
	
	//Setup outputs
	pinMode(LED_1, OUTPUT);
	pinMode(LED_2, OUTPUT);
	pinMode(LED_3, OUTPUT);
	pinMode(LED_4, OUTPUT);
	pinMode(LED_5, OUTPUT);
	pinMode(SPEAKERPIN, OUTPUT);

	gameMode = MODE_MEMORY; //Set memory game by default
	Serial.println("Ready to go");
}

void loop()
{
  /* add main program code here */
	baitMode();

	//Start of game play
	// Indicate the start of game play
	setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5); // Turn all LEDs on
	delay(1000);
	setLEDs(CHOICE_OFF); // Turn off LEDs
	delay(500);
	Serial.println("Let's play");
	if (gameMode == MODE_MEMORY)
	{
		// Play memory game and handle result
		if (play_memory() == true)
			play_winner(); // Player won, play winner tones
		else
			play_loser(); // Player lost, play loser tones
	}

}
//---------------------------------------------------------
//Gameplay related functions

//Play memory mode
boolean play_memory(void)
{
	randomSeed(millis()); // Seed the random generator with random amount of millis()

	gameRound = 0; // Reset the game to the beginning
	
	Serial.println("Let's test your memory");
	while (gameRound < MAX_ROUNDS)
	{
		add_to_moves(); // Add a button to the current moves, then play them back

		playMoves(); // Play back the current game board

		// Then require the player to repeat the sequence.
		for (byte currentMove = 0; currentMove < gameRound; currentMove++)
		{
			byte choice = wait_for_button(); // See what button the user presses

			if (choice == 0) return false; // If wait timed out, player loses

			if (choice != gameBoard[currentMove]) return false; // If the choice is incorect, player loses
		}

		delay(1000); // Player was correct, delay before playing moves
	}

	return true; // Player made it through all the rounds to win!
}

// Plays the current contents of the game moves
void playMoves(void)
{
	Serial.println("Repeat after me");
	for (byte currentMove = 0; currentMove < gameRound; currentMove++)
	{
		playTone(gameBoard[currentMove]); //Consider adding duration as a parameter

		// Wait some amount of time between button playback
		// Shorten this to make game harder
		delay(250); // 150 works well. 75 gets fast.
	}
}

// Adds a new random button to the game sequence
void add_to_moves(void)
{
	Serial.println("Adding one more");
	byte newButton = random(0, 5); //min (included), max (exluded)

	// We have to convert this number, 0 to 3, to CHOICEs
	if (newButton == 0) newButton = CHOICE_1;
	else if (newButton == 1) newButton = CHOICE_2;
	else if (newButton == 2) newButton = CHOICE_3;
	else if (newButton == 3) newButton = CHOICE_4;
	else if (newButton == 4) newButton = CHOICE_5;

	gameBoard[gameRound++] = newButton; // Add this new button to the game array
}

// --------------------------------------------------------
// Hardware related functions

// Lights a given LEDs
// Pass in a byte that is made up from CHOICE_1, CHOICE_2, etc
void setLEDs(byte leds)
{
	if ((leds & CHOICE_1) != 0)
		digitalWrite(LED_1, HIGH);
	else
		digitalWrite(LED_1, LOW);

	if ((leds & CHOICE_2) != 0)
		digitalWrite(LED_2, HIGH);
	else
		digitalWrite(LED_2, LOW);

	if ((leds & CHOICE_3) != 0)
		digitalWrite(LED_3, HIGH);
	else
		digitalWrite(LED_3, LOW);

	if ((leds & CHOICE_4) != 0)
		digitalWrite(LED_4, HIGH);
	else
		digitalWrite(LED_4, LOW);

	if ((leds & CHOICE_5) != 0)
		digitalWrite(LED_5, HIGH);
	else
		digitalWrite(LED_5, LOW);
}

// Wait for a button to be pressed. 
// Returns one of LED colors (LED_RED, etc.) if successful, 0 if timed out
byte wait_for_button(void)
{
	long startTime = millis(); // Remember the time we started the this loop
	
	Serial.println("Press a button");
	
	while ((millis() - startTime) < TIME_LIMIT) // Loop until too much time has passed
	{
		byte button = checkButton();

		if (button != CHOICE_NONE)
		{
			playTone(button); // Play the button the user just pressed

			while (checkButton() != CHOICE_NONE);  // Now let's wait for user to release button

			delay(10); // This helps with debouncing and accidental double taps

			return button;
		}

	}
	return CHOICE_NONE; //timed out
	Serial.println("Are you there?");
}

// Returns a '1' bit in the position corresponding to CHOICE_1, CHOICE_2, etc.
byte checkButton(void)
{
	if (digitalRead(BUTTON_1) == 0) return(CHOICE_1);
	else if (digitalRead(BUTTON_2) == 0) return(CHOICE_2);
	else if (digitalRead(BUTTON_3) == 0) return(CHOICE_3);
	else if (digitalRead(BUTTON_4) == 0) return(CHOICE_4);
	else if (digitalRead(BUTTON_5) == 0) return(CHOICE_5);

	return(CHOICE_NONE); // If no button is pressed, return none
}

// Throws the bait for players while waiting for user to press button.
void baitMode(void)
{
	Serial.println("Calling all players");
	while (1)
	{
		setLEDs(CHOICE_1);
		delay(250);
		if (checkButton() != CHOICE_NONE) return;

		setLEDs(CHOICE_2);
		delay(250);
		if (checkButton() != CHOICE_NONE) return;

		setLEDs(CHOICE_3);
		delay(250);
		if (checkButton() != CHOICE_NONE) return;

		setLEDs(CHOICE_4);
		delay(250);
		if (checkButton() != CHOICE_NONE) return;

		setLEDs(CHOICE_5);
		delay(250);
		if (checkButton() != CHOICE_NONE) return;

		setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5);
		delay(250);
		if (checkButton() != CHOICE_NONE) return;

		setLEDs(CHOICE_OFF);
		delay(250);
		if (checkButton() != CHOICE_NONE) return;
	}
}

void playTone(byte button){
	setLEDs(button);
	if (button == CHOICE_1) {
		tone(SPEAKERPIN, NOTE_C4,125);
		delay(150);
	}
	else if (button == CHOICE_2) {
		tone(SPEAKERPIN, NOTE_D4,125);
		delay(150);
	}
	else if (button == CHOICE_3) {
		tone(SPEAKERPIN, NOTE_E4,125);
		delay(150);
	}
	else if (button == CHOICE_4) {
		tone(SPEAKERPIN, NOTE_F4,125);
		delay(150);
	}
	else if (button == CHOICE_5) {
		tone(SPEAKERPIN, NOTE_G4,125);
		delay(150);
	}
	setLEDs(CHOICE_OFF);
}

void play_winner(void){
	Serial.println("Winner song");
	setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5); // Turn all LEDs on
	delay(250);
	setLEDs(CHOICE_2 | CHOICE_4);
	tone(SPEAKERPIN, NOTE_E4, 125);
	delay(150);
	setLEDs(CHOICE_1 |  CHOICE_3  | CHOICE_5);
	tone(SPEAKERPIN, NOTE_E4, 125);
	delay(150);
	setLEDs(CHOICE_1 | CHOICE_5);
	tone(SPEAKERPIN, NOTE_F4, 125);
	delay(150);
	setLEDs(CHOICE_3);
	tone(SPEAKERPIN, NOTE_E4, 125);
	delay(150);
	setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5);
	tone(SPEAKERPIN, NOTE_D4, 375);
	delay(425);
	setLEDs(CHOICE_OFF);
	tone(SPEAKERPIN, NOTE_G4, 125);
	delay(150);
	setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5);
	tone(SPEAKERPIN, NOTE_G4, 125);
	delay(150);
	setLEDs(CHOICE_OFF);
	tone(SPEAKERPIN, NOTE_A4, 250);
	delay(300);
	setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5);
	tone(SPEAKERPIN, NOTE_G4, 625);
	delay(725);
}

void play_loser(void){
	Serial.println("Loser song");
	setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5);
	delay(250);
	setLEDs(CHOICE_3 | CHOICE_5);
	tone(SPEAKERPIN, NOTE_B5, 250);
	delay(300);
	setLEDs(CHOICE_1 | CHOICE_3);
	tone(SPEAKERPIN, NOTE_A5, 250);
	delay(300);
	setLEDs(CHOICE_2 | CHOICE_4);
	tone(SPEAKERPIN, NOTE_G5, 250);
	delay(300);
	setLEDs(CHOICE_1 | CHOICE_2 | CHOICE_3 | CHOICE_4 | CHOICE_5);
	tone(SPEAKERPIN, NOTE_G5, 1000);
	delay(1250);
}
