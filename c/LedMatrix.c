#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bin.h"
#include <signal.h>
#include <unistd.h>


#define SP {b00000000, b00000000, b00000000, b00000000, b00000000, b00000000, b00000000, b00000000, 4}
#define EM {b00000011, b00000011, b00000011, b00000011, b00000011, b00000000, b00000011, b00000011, 2}

#define FS {b00000000, b00000000, b00000000, b00000000, b00000000, b00000000, b00000011, b00000011, 2}

#define SC {b00000000, b00000000, b00000011, b00000011, b00000000, b00000000, b00000011, b00000011, 2}

#define QM {b00011110, b00111111, b00110011, b00000110, b00001100, b00000000, b00001100, b00001100, 6}

#define OP {b00000011, b00000111, b00001110, b00001100, b00001100, b00001110, b00000111, b00000011, 4}
#define CP {b00001100, b00001110, b00000111, b00000011, b00000011, b00000111, b00001110, b00001100, 4}

#define A {b00011100, b00111110, b01100011, b01100011, b01111111, b01111111, b01100011, b01100011, 7}
#define B {b01111110, b01111111, b01100011, b01111110, b01111111, b01100011, b01111111, b01111111, 7}
#define C {b00011100, b00111110, b01100011, b01100000, b01100000, b01100011, b00111110, b00011100, 7}
#define D {b01111100, b01111110, b01100011, b01100011, b01100011, b01100011, b01111110, b01111100, 7}
#define E {b01111111, b01111111, b01100000, b01111100, b01111100, b01100000, b01111111, b01111111, 7}
#define F {b01111111, b01111111, b01100000, b01111100, b01111100, b01100000, b01100000, b01100000, 7}
#define G {b00011100, b00111110, b01100011, b01100000, b01100111, b01100011, b00111110, b00011100, 7}
#define H {b01100011, b01100011, b01100011, b01111111, b01111111, b01100011, b01100011, b01100011, 7}
#define I {b00111111, b00111111, b00001100, b00001100, b00001100, b00001100, b00111111, b00111111, 6}
#define J {b00111111, b00111111, b00001100, b00001100, b00001100, b01101100, b01111100, b00111000, 7}
#define K {b01100011, b01100110, b01101100, b01111000, b01111000, b01101100, b01100110, b01100011, 7}
#define L {b00110000, b00110000, b00110000, b00110000, b00110000, b00110000, b00111111, b00111111, 6}
#define M {b01100011, b01110111, b01111111, b01101011, b01100011, b01100011, b01100011, b01100011, 7}
#define N {b01100011, b01110011, b01110011, b01111011, b01101111, b01100111, b01100111, b01100011, 7}
#define O {b00011100, b00111110, b01100011, b01100011, b01100011, b01100011, b00111110, b00011100, 7}
#define P {b01111100, b01111110, b01100011, b01100011, b01111110, b01111100, b01100000, b01100000, 7}
#define Q {b00011100, b00111110, b01100011, b01100011, b01100011, b01100111, b00111110, b00011101, 7}
#define R {b01111100, b01111110, b01100011, b01100011, b01111110, b01111100, b01101110, b01100111, 7}
#define S {b00111110, b01111111, b01110011, b00111000, b00001110, b01100111, b01111111, b00111110, 7}
#define T {b00111111, b00111111, b00001100, b00001100, b00001100, b00001100, b00001100, b00001100, 6}
#define U {b01100011, b01100011, b01100011, b01100011, b01100011, b01100011, b01111111, b00111110, 7}
#define V {b01100011, b01100011, b01100011, b01110111, b00110110, b00111110, b00011100, b00011100, 7}
#define W {b01100011, b01100011, b01100011, b01100011, b01101011, b01111111, b01110111, b01100011, 7}
#define X {b01100011, b01110111, b00111110, b00011100, b00011100, b00111110, b01110111, b01100011, 7}
#define Y {b11000011, b11000011, b01100110, b00111100, b00111100, b00011000, b00011000, b00011000, 8}
#define Z {b01111111, b01111111, b00000111, b00001110, b00011100, b00111000, b01111111, b01111111, 7}

#define UL {b00000000, b00000000, b00000000, b00000000, b00000000, b00000000, b00111111, b00111111, 6}
#define AP {b00000011, b00000011, b00000011, b00000000, b00000000, b00000000, b00000000, b00000000, 2}
#define MINUS {b00000000, b00000000, b00000000, b00001111, b00001111, b00000000, b00000000, b00000000, 4}
#define PLUS {b00000000, b00001100, b00001100, b00111111, b00111111, b00001100, b00001100, b00000000, 6}
#define EQ {b00000000, b00111111, b00111111, b00000000, b00000000, b00111111, b00111111, b00000000, 6}

#define d0 {b00011110, b00111111, b00110011, b00110011, b00110011, b00110011, b00111111, b00011110, 6}
#define d1 {b00011100, b00111100, b00001100, b00001100, b00001100, b00001100, b00111111, b00111111, 6}
#define d2 {b00011110, b00111111, b00110011, b00000011, b00000110, b00001100, b00011111, b00111111, 6}
#define d3 {b00011110, b00111111, b00110011, b00000111, b00000111, b00110011, b00111111, b00011110, 6}
#define d4 {b00000111, b00001111, b00011011, b00110011, b00111111, b00111111, b00000011, b00000011, 6}
#define d5 {b00111111, b00111111, b00110000, b00111110, b00000111, b00110011, b00111111, b00011110, 6}
#define d6 {b00011110, b00111111, b00110011, b00110000, b00111110, b00110011, b00111111, b00011110, 6}
#define d7 {b00111111, b00111111, b00000011, b00000110, b00001110, b00011100, b00011000, b00011000, 6}
#define d8 {b00011110, b00111111, b00110011, b00111111, b00111111, b00110011, b00111111, b00011110, 6}
#define d9 {b00011110, b00111111, b00110011, b00111111, b00000011, b00110011, b00111111, b00011110, 6}

#define arrow_left {b00011000, b00110000, b01100000, b11111111, b11111111, b01100000, b00110000, b00011000, 7}

unsigned int all_letters[128][9] = {SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP,		// 0 - 15
				    SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP,		// 16 - 31
				    SP, EM, SP, SP, SP, SP, SP, AP, OP, CP, SP, PLUS, SP, MINUS, FS, SP,	// 32 - 47
				    d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, SC, SP, SP, EQ, SP, QM,		// 48 - 63
				    SP, A, B, C, D, E, F, G, H, I, J, K, L , M, N, O,				// 64 - 79
				    P, Q, R, S, T, U, V, W, X, Y, Z, SP, SP, SP, SP, UL,			// 80 - 95
				    SP, SP};									// 96 - 111

int dataPin = 0;
int clockPin = 1;
int latchPin = 2;
int switchPin = 3;
int resetPin = 4;

unsigned int display[8];
unsigned int display_on[8] = {16777215, 16777215, 16777215, 16777215, 16777215, 16777215, 16777215, 16777215};

volatile sig_atomic_t stop = 0;

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

void pulse(int pin) {
    digitalWrite(pin, 0);
    delayMicroseconds(1);
    digitalWrite(pin, 1);
}

void SIPO(unsigned int serialData) {
    unsigned int mask = 0x000001;
    for (int i = 0; i < 24; i++) {
        digitalWrite(dataPin, (serialData & (mask << i)) > 0);
        pulse(clockPin);
    }
}

void init(void) {
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(switchPin, OUTPUT);
    pinMode(resetPin, OUTPUT);

    SIPO((unsigned int) 0);
    pulse(latchPin);

    digitalWrite(dataPin, 0);
    digitalWrite(clockPin, 0);
    digitalWrite(latchPin, 0);
    digitalWrite(switchPin, 0);
    digitalWrite(resetPin, 0);
}

void multiplexing(unsigned int display[8], int delay_v) {
    int i = 0;
    for (int count = 0; count < delay_v && !stop; count++) {
	SIPO(display[i]);
	pulse(latchPin);

	delayMicroseconds(200);

	SIPO((unsigned int) 0);
	pulse(latchPin);

	if (i < 7) {
		pulse(switchPin);
		i++;
	}
	else {
		digitalWrite(resetPin, 1);
		delayMicroseconds(1);
		digitalWrite(resetPin, 0);
		i = 0;
	}
    }

    if (stop) {
    	SIPO((unsigned int) 0);
	pulse(latchPin);
	printf("Interrupted\n");
	exit(1);
    }
}

void shiftingWord(unsigned int letters[][9], int numberOfLetters) {
    int delay_v = 80;

    for (int i = 0; i < 8; i++)
        display[i] = (unsigned int) 0;

    multiplexing(display_on, 1800);

    digitalWrite(resetPin, 1);
    delayMicroseconds(1);
    digitalWrite(resetPin, 0);

    for (int letter = 0; letter < numberOfLetters; letter++) {
	for (int bit = letters[letter][8] + 1; bit >= 0; bit--) {
	    for (int row = 0; row < 8; row++)
		display[row] = display[row] << 1 | ((unsigned int) (letters[letter][row] >> bit));

	    multiplexing(display, delay_v);
	}
    }

    for (int i = 0; i < 24; i++) {
	for (int row = 0; row < 8; row++)
		display[row] <<= 1;

	multiplexing(display, delay_v);
    }
}


void testLetter(void) {
	unsigned int zero = 0;
	unsigned int letter[1][9] = {arrow_left};

	for (int i = 0; i < 8; i++)
	    display[i] = zero;

	for (int i = 0; i < 8; i++)
		display[i] += letter[0][i];

	for (;;)
		multiplexing(display, 160);
}


void spiral(void) {
    int rows = 8;
    int columns = 24;
    int delay_v = 80;

    for (int i = 0; i < 8; i++)
	display[i] = (unsigned int) 0;

    int  i, j, k, l;

    for (int loop = 0; loop < rows / 2; loop++) {
	for (i = columns - loop - 1; i >= loop; i--) {
	    setBit(&display[loop], i);
	    multiplexing(display, delay_v);
	}

	for (j = 1 + loop; j < rows - loop; j++) {
	    setBit(&display[j], loop);
	    multiplexing(display, delay_v);
	}

	for (k = 1 + loop; k < columns - loop; k++) {
	    setBit(&display[rows - loop - 1], k);
	    multiplexing(display, delay_v);
	}

	for (l = rows - loop - 2; l >= 1 + loop; l--) {
	    setBit(&display[l], columns - loop - 1);
	    multiplexing(display, delay_v);
	}
    }

    multiplexing(display, 2500);
}

void copyLetter(unsigned int to[9], unsigned int letter[9]) {
    for (int i = 0; i < 9; i++)
	to[i] = letter[i];
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    int i, bit, index;

    if (wiringPiSetup() == -1) {
        printf("Error setting up Pi pins");
        fflush(stdout);
        return 1;
    }

    init();
    
    if (argc == 2) {
	removeSlash(argv[1]);
	int size = strlen(argv[1]);
	unsigned int letters[100][9];
	for (i = 0; i < size; i++) {
	    index = toupper(argv[1][i]);
	    copyLetter(letters[i], all_letters[index]);
	}

	shiftingWord(letters, size);
    }
    else if (argc == 1) {
	unsigned int xmas[][9] = {M, E, R, R, Y, SP, C, H, R, I, S, T, M, A, S, EM};
	int xmasSize = sizeof(xmas) / sizeof(xmas[0]);


	unsigned int arrow_test[][9] = {arrow_left, SP, SP, arrow_left};
	int arrow_testSize = sizeof(arrow_test) / sizeof(arrow_test[0]);


	unsigned int triangles[][8] = {{1, 3, 7, 15, 31, 63, 127, 255}};

	//  for (i = 0; i < 1; i++) {
		//  shiftingWord(xmas, xmasSize);
		shiftingWord(arrow_test, arrow_testSize);
		spiral();
	//  }
    
    //  testLetter();
    }

    return 0;
}
