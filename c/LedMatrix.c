#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sprites.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <wiringPi.h>


volatile sig_atomic_t stop = 0;

const int dataPin = 0;
const int clockPin = 1;
const int latchPin = 2;
const int switchPin = 3;
const int resetPin = 4;

const int CONST_ROWS = 8;
const int CONST_COLUMNS = 24;
const unsigned int bitMask = 0x000001;

unsigned int display[8];

unsigned int all_letters[128][9] = {SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP,		// 0 - 15
				    SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP,		// 16 - 31
				    SP, EM, SP, SP, SP, SP, SP, AP, OP, CP, SP, PLUS, SP, MINUS, FS, SP,		// 32 - 47
				    d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, SC, SP, SP, EQ, SP, QM,		// 48 - 63
				    SP, A, B, C, D, E, F, G, H, I, J, K, L , M, N, O,				// 64 - 79
				    P, Q, R, S, T, U, V, W, X, Y, Z, SP, SP, SP, SP, UL,				// 80 - 95
				    SP, SP};									// 96 - 111

unsigned int clk[11][9] = {clk_0, clk_1, clk_2, clk_3, clk_4, clk_5, clk_6, clk_7, clk_8, clk_9, clk_sc};


void signalHandler(int signum) {
    stop = 1;
}


void removeSlash(char string[]) {
    int i = 0;
    while (i < strlen(string) - 1) {
	if (string[i] == '\\') {
	    switch (string[i + 1]) {
		case '\'': strcpy(string + i, string + i + 1); break;
		case '?':  strcpy(string + i, string + i + 1); break;
		case '(':  strcpy(string + i, string + i + 1); break;
		case ')':  strcpy(string + i, string + i + 1); break;
		default:   break;
	    }
	}

	i++;
    }
}


void setBit(unsigned int *value, int index) {
    unsigned int mask = 1 << index;
    *value |= mask;
}


void clearBit(unsigned int *value, int index) {
    unsigned int mask = ~(1 << index);
    *value &= mask;
}


unsigned short getBit(unsigned int value, int index) {
    unsigned int mask = ~(1 << index);
    return (value | mask) == (unsigned int) -1;
}


void pulsePosEdge(int pin) {
    digitalWrite(pin, 0);
    delayMicroseconds(1);
    digitalWrite(pin, 1);
}


void pulseNegEdge(int pin) {
    digitalWrite(pin, 1);
    delayMicroseconds(1);
    digitalWrite(pin, 0);
}


void SIPO(unsigned int serialData) {
    for (int column = 0; column < CONST_COLUMNS; column++) {
        digitalWrite(dataPin, (serialData & (bitMask << column)) > 0);
        pulsePosEdge(clockPin);
    }

    pulsePosEdge(latchPin);
}


void initialiseDisplay(void) {
    for (int row = 0; row < 8; row++)
        display[row] = (unsigned int) 0;
}


void clearDisplay(void) {
    for (int row = 0; row < CONST_ROWS; row++)
	display[row] <<= CONST_COLUMNS;
}


void illuminateDisplay(void) {
    for (int row = 0; row < CONST_ROWS; row++) {
	display[row] = 0xFFFFFF;
    }
}


void cleanUp(void) {
    SIPO(0x000000);

    digitalWrite(dataPin, 0);
    digitalWrite(clockPin, 0);
    digitalWrite(latchPin, 0);
    digitalWrite(switchPin, 0);
    digitalWrite(resetPin, 0);
}


void init(void) {
    initialiseDisplay();

    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(switchPin, OUTPUT);
    pinMode(resetPin, OUTPUT);

    cleanUp();
    pulseNegEdge(resetPin);
}


void multiplexing(void) {
    for (int i = 0; i < CONST_ROWS && !stop; i++) {
	SIPO(display[i]);

	delayMicroseconds(100);

	cleanUp();

	if (i < 7)
	    pulsePosEdge(switchPin);
	else
	    pulseNegEdge(resetPin);
    }

    if (stop) {
    	cleanUp();
	printf("Interrupted\n");
	exit(1);
    }
}


void multiplexingDelayed(int delayBy) {
    for (int tick = 0; tick < delayBy; tick++)
	multiplexing();
}


void brightnessControll() {
    clearDisplay();

    unsigned int sprite[1][9] = {bar_lg};
    int offset;
    int i = 0;
    for (;;) {
	for (int i = 1; i <= 512; i++) {
	    for (int j = 0; j < 2; j++) {
		offset = j * 12;
		if (j == 0 || i % 512 == 0) {
		    for (int k = 0; k < CONST_ROWS; k++)
			display[k] |= sprite[0][k] << offset;
		}
		else {
		    for (int k = 0; k < CONST_ROWS; k++)
			display[k] ^= sprite[0][k] << offset;
		}
	    }
	    multiplexing();
	}
    }
}


void shiftingWord(unsigned int letters[][9], int numberOfLetters) {
    int delayBy = 16;

    illuminateDisplay();
    multiplexingDelayed(225);

    for (int letter = 0; letter < numberOfLetters; letter++) {
	for (int bit = letters[letter][8] + 1; bit >= 0; bit--) {
	    for (int row = 0; row < CONST_ROWS; row++)
		display[row] = display[row] << 1 | ((unsigned int) (letters[letter][row] >> bit));

	    multiplexingDelayed(delayBy);
	}
    }

    for (int column = 0; column < CONST_COLUMNS; column++) {
	for (int row = 0; row < CONST_ROWS; row++)
		display[row] <<= 1;

	multiplexingDelayed(delayBy);
    }
}


void testLetter(void) {
    clearDisplay();

    unsigned int letter[1][9] = {arrow_left};
    for (int row = 0; row < CONST_ROWS; row++)
	display[row] |= letter[0][row];

    for (;;)
	multiplexing();
}


void spiral(void) {
    clearDisplay();

    int delayBy = 15;

    int  top, right, bottom, left;
    for (int loop = 0; loop < CONST_ROWS / 2; loop++) {
	for (top = CONST_COLUMNS - loop - 1; top >= loop; top--) {
	    setBit(&display[loop], top);
	    multiplexingDelayed(delayBy);
	}

	for (right = 1 + loop; right < CONST_ROWS - loop; right++) {
	    setBit(&display[right], loop);
	    multiplexingDelayed(delayBy);
	}

	for (bottom = 1 + loop; bottom < CONST_COLUMNS - loop; bottom++) {
	    setBit(&display[CONST_ROWS - loop - 1], bottom);
	    multiplexingDelayed(delayBy);
	}

	for (left = CONST_ROWS - loop - 2; left >= 1 + loop; left--) {
	    setBit(&display[left], CONST_COLUMNS - loop - 1);
	    multiplexingDelayed(delayBy);
	}
    }

    multiplexingDelayed(313);
}


void clearDisplayAnimation(void) {
    int delayBy = 12;

    for (int row = 0; row < CONST_ROWS; row++) {
	for (int column = CONST_COLUMNS - 1; column >= 0; column--) {
	    setBit(&display[row], column);
	    multiplexingDelayed(delayBy);
	    clearBit(&display[row], column);
	}
    }
}


void copyLetter(unsigned int to[9], unsigned int letter[9]) {
    for (int i = 0; i < 9; i++)
	to[i] = letter[i];
}


void display_clock(void) {
    int row, hour, minute;

    int ok = 0;
    int display_at_minute = 0;
    int display_for_minutes = 1;

    clearDisplay();

    time_t now;
    time(&now);
    struct tm* time_now = localtime(&now);

    do {
	hour = time_now -> tm_hour;
	minute = time_now -> tm_min;

	if (minute == display_at_minute) {
	    if (ok == 0) {
		clearDisplay();
		ok = 1;
	    }
	
	    for (row = 0; row < CONST_ROWS; row++) {
		// Clear display
		display[row] <<= CONST_COLUMNS;

		// Hour first digit
		display[row] |= clk[(hour / 10)][row] << 18;

		// Hour second digit
		display[row] |= clk[(hour % 10)][row] << 13;

		// Semi colon delimiter
		display[row] |= clk[10][row] << 11;

		// Minute first digit
		display[row] |= clk[(minute / 10)][row] << 6;

		// Minute second digit
		display[row] |= clk[(minute % 10)][row] << 1;
	    }
	    
	    multiplexing();
	}
	else if (minute == display_at_minute + display_for_minutes && ok == 1) {
	    clearDisplayAnimation();
	    ok = 0;
	}

	time(&now);
	localtime(&now);
    } while (!stop);
    
    if (stop) {
    	cleanUp();
	printf("Interrupted\n");
	exit(1);
    }
}


int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    piHiPri(1);

    if (wiringPiSetup() == -1) {
        printf("Error setting up Pi pins");
        fflush(stdout);

        return 1;
    }

    init();

    if (argc == 1) {
	unsigned int xmas[][9] = {M, E, R, R, Y, SP, C, H, R, I, S, T, M, A, S, EM};
	int xmasSize = sizeof(xmas) / sizeof(xmas[0]);

	unsigned int arrow_test[][9] = {arrow_left, SP, SP, arrow_left};
	unsigned int face_test[][9] = {face, SP, face, SP, face, SP, face, SP, face, SP, face};

	int arrow_testSize = sizeof(arrow_test) / sizeof(arrow_test[0]);
	int face_size = sizeof(face_test) / sizeof(face_test[0]);

	unsigned int triangles[][8] = {{1, 3, 7, 15, 31, 63, 127, 255}};

	//  brightnessControll();
	display_clock();
	clearDisplayAnimation();
	for (int i = 0; i < 1; i++) {
	    //  shiftingWord(xmas, xmasSize);
	    shiftingWord(face_test, face_size);
	    shiftingWord(arrow_test, arrow_testSize);
	    spiral();
	}

	//  testLetter();
    }
    else if (argc == 2) {
	removeSlash(argv[1]);

	int index;
	int size = strlen(argv[1]);
	unsigned int letters[100][9];

	for (int i = 0; i < size; i++) {
	    index = toupper(argv[1][i]);
	    copyLetter(letters[i], all_letters[index]);
	}

	shiftingWord(letters, size);
    }

    return 0;
}
