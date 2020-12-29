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

const int rows = 8;
const int columns = 24;
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
    for (int column = 0; column < columns; column++) {
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
    for (int row = 0; row < rows; row++)
	display[row] <<= columns;
}


void illuminateDisplay(void) {
    for (int row = 0; row < rows; row++) {
	display[row] = (unsigned int) 16777215;
    }
}


void cleanUp(void) {
    SIPO((unsigned int) 0);

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
    for (int i = 0; i < rows && !stop; i++) {
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


void shiftingWord(unsigned int letters[][9], int numberOfLetters) {
    int delayBy = 16;

    illuminateDisplay();
    multiplexingDelayed(225);

    for (int letter = 0; letter < numberOfLetters; letter++) {
	for (int bit = letters[letter][8] + 1; bit >= 0; bit--) {
	    for (int row = 0; row < rows; row++)
		display[row] = display[row] << 1 | ((unsigned int) (letters[letter][row] >> bit));

	    multiplexingDelayed(delayBy);
	}
    }

    for (int column = 0; column < columns; column++) {
	for (int row = 0; row < rows; row++)
		display[row] <<= 1;

	multiplexingDelayed(delayBy);
    }
}


void testLetter(void) {
    clearDisplay();

    unsigned int letter[1][9] = {arrow_left};
    for (int row = 0; row < rows; row++)
	display[row] |= letter[0][row];

    for (;;)
	multiplexing();
}


void spiral(void) {
    clearDisplay();

    int delayBy = 10;

    int  top, right, bottom, left;
    for (int loop = 0; loop < rows / 2; loop++) {
	for (top = columns - loop - 1; top >= loop; top--) {
	    setBit(&display[loop], top);
	    multiplexingDelayed(delayBy);
	}

	for (right = 1 + loop; right < rows - loop; right++) {
	    setBit(&display[right], loop);
	    multiplexingDelayed(delayBy);
	}

	for (bottom = 1 + loop; bottom < columns - loop; bottom++) {
	    setBit(&display[rows - loop - 1], bottom);
	    multiplexingDelayed(delayBy);
	}

	for (left = rows - loop - 2; left >= 1 + loop; left--) {
	    setBit(&display[left], columns - loop - 1);
	    multiplexingDelayed(delayBy);
	}
    }

    multiplexingDelayed(313);
}


void copyLetter(unsigned int to[9], unsigned int letter[9]) {
    for (int i = 0; i < 9; i++)
	to[i] = letter[i];
}


void display_clock(void) {
    clearDisplay();

    int row, hour, minute;

    time_t now;
    time(&now);
    struct tm* time_now = localtime(&now);

    do {
	hour = time_now -> tm_hour;
	minute = time_now -> tm_min;

	for (row = 0; row < rows; row++) {
	    // Clear display
	    display[row] <<= columns;

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

	time(&now);
	localtime(&now);
    } while (1);
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

	display_clock();
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
