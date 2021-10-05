#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sprites.h"
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <time.h>
#include <wiringPi.h>

#define STREAM_FILE "/var/www/html/pidisplay/c/input_frame.txt";


volatile sig_atomic_t stop = 0;

const int dataPin = 0;
const int clockPin = 1;
const int latchPin = 2;
const int switchPin = 3;
const int resetPin = 4;

const int CONST_ROWS = 8;
const int CONST_COLUMNS = 24;
const unsigned int bitMask = 0x000001;

// Period constants reduce column ghosting when only one LED is on but indirectly affect displaying quality. (period defined in microseconds)
const int mux_switch_period_high = 800;     // Little to no ghosting.
const int mux_switch_period_average = 450;  // Less ghosting, good image.
const int mux_switch_period_low = 100;      // Most clear image.

unsigned int display[8];

unsigned int ASCII[128][9] = {SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP,		// 0 - 15
                              SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP,		// 16 - 31
                              SP, EM, SP, SP, SP, SP, SP, AP, OP, CP, SP, PLUS, SP, MINUS, FS, SP,	// 32 - 47
                              d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, SC, SP, SP, EQ, SP, QM,		// 48 - 63
                              SP, A, B, C, D, E, F, G, H, I, J, K, L , M, N, O,				        // 64 - 79
                              P, Q, R, S, T, U, V, W, X, Y, Z, SP, SP, SP, SP, UL,			        // 80 - 95
                              SP, SP};									                            // 96 - 97

unsigned int clk[11][9] = {clk_0, clk_1, clk_2, clk_3, clk_4, clk_5, clk_6, clk_7, clk_8, clk_9, clk_sc};
unsigned int year2021[4][9] = {y2, y0, y2, y1};

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


void invertDisplay(void) {
	for (int row = 0; row < CONST_ROWS; row++)
		display[row] = ~display[row];
}


void cleanUp(void) {
	SIPO(0x000000);

	digitalWrite(dataPin, 0);
	digitalWrite(clockPin, 0);
	digitalWrite(latchPin, 0);
	digitalWrite(switchPin, 0);
	digitalWrite(resetPin, 0);
}


void interrupt(void) {
	cleanUp();
	printf(" Interrupted\n");
	exit(1);
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


void multiplexing(int switchPeriod) {
	for (int i = 0; i < CONST_ROWS && !stop; i++) {
		SIPO(display[i]);

		delayMicroseconds(switchPeriod);

		cleanUp();

		if (i < 7)
			pulsePosEdge(switchPin);
		else
			pulseNegEdge(resetPin);
	}

	if (stop) {
		interrupt();
	}
}


void multiplexingDelayed(int switchPeriod, int delayBy) {
	for (int tick = 0; tick < delayBy; tick++)
		multiplexing(switchPeriod);
}


void brightnessControll(void) {
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

			multiplexing(mux_switch_period_low);
		}
	}
}


void shiftingWord(unsigned int letters[][9], int numberOfLetters, int delayBy) {
	illuminateDisplay();
	multiplexingDelayed(mux_switch_period_low, 480);
	clearDisplay();

	for (int letter = 0; letter < numberOfLetters; letter++) {
		for (int bit = letters[letter][8] + 1; bit >= 0; bit--) {
			for (int row = 0; row < CONST_ROWS; row++)
			display[row] = display[row] << 1 | letters[letter][row] >> bit;

			multiplexingDelayed(mux_switch_period_low, delayBy);
		}
	}

	for (int column = 0; column < CONST_COLUMNS; column++) {
		for (int row = 0; row < CONST_ROWS; row++)
			display[row] <<= 1;

		multiplexingDelayed(mux_switch_period_low, delayBy);
	}
}


void testLetter(void) {
	clearDisplay();

	unsigned int letter[1][9] = {d9};
	for (int row = 0; row < CONST_ROWS; row++)
		display[row] |= letter[0][row];

	for (;;)
		multiplexing(mux_switch_period_low);
}


void spiral(void) {
	clearDisplay();

	int delayBy = 3;

	int  top, right, bottom, left;
	for (int loop = 0; loop < CONST_ROWS / 2; loop++) {
		for (top = CONST_COLUMNS - loop - 1; top >= loop; top--) {
			setBit(&display[loop], top);
			multiplexingDelayed(mux_switch_period_high, delayBy);
		}

		for (right = 1 + loop; right < CONST_ROWS - loop; right++) {
			setBit(&display[right], loop);
			multiplexingDelayed(mux_switch_period_high, delayBy);
		}

		for (bottom = 1 + loop; bottom < CONST_COLUMNS - loop; bottom++) {
			setBit(&display[CONST_ROWS - loop - 1], bottom);
			multiplexingDelayed(mux_switch_period_high, delayBy);
		}

		for (left = CONST_ROWS - loop - 2; left >= 1 + loop; left--) {
			setBit(&display[left], CONST_COLUMNS - loop - 1);
			multiplexingDelayed(mux_switch_period_high, delayBy);
		}
	}

	multiplexingDelayed(mux_switch_period_high, 80);
}


void clearDisplayAnimation(void) {
	int delayBy = 4;

	for (int row = 0; row < CONST_ROWS; row++) {
		for (int column = CONST_COLUMNS - 1; column >= 0; column--) {
			setBit(&display[row], column);
			multiplexingDelayed(mux_switch_period_high, delayBy);
			clearBit(&display[row], column);
		}
	}
}


void copyLetter(unsigned int to[9], unsigned int letter[9]) {
	for (int i = 0; i < 9; i++)
		to[i] = letter[i];
}


void displayYear(void) {
	for (int row = 0; row < CONST_ROWS; row++) {
		display[row] |= year2021[0][row] << 18;
		display[row] |= year2021[1][row] << 12;
		display[row] |= year2021[2][row] << 6;
		display[row] |= year2021[3][row] << 1;
	}
}


void flashYear(void) {
	for (int i = 0; i < 3; i++) {
		displayYear();
		multiplexingDelayed(mux_switch_period_high, 440);
		clearDisplay();
		multiplexingDelayed(mux_switch_period_high, 440);
	}
}


void displaySecondsCounter(int count) {
	int shift;
	for (int row = 0; row < CONST_ROWS; row++) {
		// Clear display
		display[row] <<= CONST_COLUMNS;

		// First digit
		if (count > 9) {
			display[row] |= ASCII[(count / 10 + 48)][row] << 13;
			shift = 5;
		}
		else
			shift = 9;

		// Second digit
		display[row] |= ASCII[(count % 10 + 48)][row] << shift;
	}
}


void tearDropAnimation(void) {
	clearDisplay();

	int column, length;

	int upper1 = 23;
	int lower1 = 0;

	int upper2 = 3;
	int lower2 = 1;

	srandom((unsigned int) time(NULL));
	int count = 0;
	while (count < 1000) {
		for (int row = CONST_ROWS - 2; row >= 0; row--) {
			display[row + 1] = display[row];
		}

		length = (random() % (upper2 - lower2 + 1)) + lower2;

		display[0] <<= 24;
		for (int no = 0; no < length; no++) {
			column = (random() % (upper1 - lower1 + 1)) + lower1;
			setBit(&display[0], column);
		}

		multiplexingDelayed(mux_switch_period_high, 8);
		count++;
	}
}


double easeInQuadratic (double time_v, double base, double change, double duration) {
	time_v /= duration;
	return change * time_v * time_v + base;
}


double easeOutQuadratic(double t, double b, double c, double d) {
	t /= d;
	return -c * t * (t - 2) + b;
}


void easeInEaseOutAnimation(unsigned int symbols[][9], int size) {
	clearDisplay();
	
	double time_v = 0.0;
	int delayBy;

	for (int symbol = 0; symbol < size; symbol++) {
		time_v = 0;
		for (int bit = symbols[symbol][8] - 1; bit >= 0; bit--) {
			for (int row = 0; row < CONST_ROWS; row++)
				display[row] = display[row] << 1 | symbols[symbol][row] >> bit;

			delayBy = easeInQuadratic((time_v / 14) * 1000, 11, 10.0, 1000);
			multiplexingDelayed(mux_switch_period_high, delayBy);
			time_v++;
		}

		for (int column = 0; column < 8; column++) {
			for (int row = 0; row < CONST_ROWS; row++)
				display[row] <<= 1;

			delayBy = easeInQuadratic((time_v / 14) * 1000, 11, 10.0, 1000);
			multiplexingDelayed(mux_switch_period_high, delayBy);
			time_v++;
		}

		multiplexingDelayed(mux_switch_period_high, 80);

		time_v = 0;
		for (int column = 0; column < 15; column++) {
			for (int row = 0; row < CONST_ROWS; row++)
				display[row] <<= 1;

			delayBy = easeOutQuadratic((time_v / 14) * 1000, 11, 10.0, 1000);
			multiplexingDelayed(mux_switch_period_high, delayBy);
			time_v++;
		}
	}
}


void displayClock(int hour, int minute) {
	for (int row = 0; row < CONST_ROWS; row++) {
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
}


void timedControll(void) {
	int countdown, option;
	int year, month, day, hour, minute, second;

	int ok = 0;
	int display_at_minute = 0;
	int display_for_minutes = 1;

	unsigned int new_year_message[][9] = {H, A, P, P, Y, SP, N, E, W, SP, Y, E, A, R, SP, d2, d0, d2, d1, EM, EM, EM};
	unsigned int new_year_message2[][9] = {L, A, SP, M, U, L, T, I, SP, A, N, I, SP, d2, d0, d2, d1, EM, EM, EM};
	unsigned int startup_message[][100] = {P, I, D, I, S, P, L, A, Y, SP, R, E, A, D, Y};
	unsigned int arrows[][9] = {arrow_left, arrow_left, arrow_left, arrow_left, arrow_left};

	int ny_size = sizeof(new_year_message) / sizeof(new_year_message[0]);
	int ny_size2 = sizeof(new_year_message2) / sizeof(new_year_message2[0]);
	int arrows_size = sizeof(arrows) / sizeof(arrows[0]);

	clearDisplay();

	time_t now;
	time(&now);
	struct tm* time_now = localtime(&now);

	do {
		year = time_now -> tm_year + 1900;
		month = time_now -> tm_mon + 1;
		day = time_now -> tm_mday;

		hour = time_now -> tm_hour;
		minute = time_now -> tm_min;
		second = time_now -> tm_sec;

		if (year == 2020 && month == 12 && day == 31 && hour == 23) {
			if (minute == 59 && second > 47) {
				countdown = 60 - second;
				displaySecondsCounter(countdown);
				multiplexing(mux_switch_period_high);
			}
			else if (minute > 54) {
				displayClock(hour, minute);
				multiplexing(mux_switch_period_high);
			}
		}
		else if (year == 2021 && month == 1 && day == 1 && hour == 0 && minute == display_at_minute) {
			shiftingWord(new_year_message2, ny_size2, 20);
			flashYear();
		}
		else if (minute == display_at_minute) {
			if (ok == 0) {
				displayClock(hour, minute);
				ok = 1;
			}

			multiplexing(mux_switch_period_high);
		}
		else if (minute == display_at_minute + display_for_minutes && ok == 1) {
			clearDisplayAnimation();
			ok = 0;
		}
		else {
			option = minute % 5;
			if (option == 2 && second == 0)
				easeInEaseOutAnimation(arrows, arrows_size);
			else if (option == 3 && second == 0) {
				spiral();
				clearDisplayAnimation();
			}
			else if (option == 4 && second == 0)
				tearDropAnimation();
		}

		time(&now);
		localtime(&now);
	} while (!stop);

	if (stop) {
		interrupt();
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

	if (argc < 3 || argc > 4) return 1;

	int index = 1;
	char option[10];
	char* data;
	while (index < argc) {
		if (strcmp(argv[index], "-o") == 0) {
			strcpy(option, argv[index + 1]);
			index++;
		}
		else
			data = argv[index];

		index++;
	}

	printf("Option: %s\n", option);
	printf("Data: %s\n", data);
	fflush(stdout);
	if (strcmp(option, "scheduled") == 0) {
		unsigned int triangles[][8] = {{1, 3, 7, 15, 31, 63, 127, 255}};
		unsigned int xmas[][9] = {M, E, R, R, Y, SP, C, H, R, I, S, T, M, A, S, EM};
		unsigned int arrows[][9] = {arrow_left, arrow_left, arrow_left, arrow_left, arrow_left};
		unsigned int faces[][9] = {smiley_face, smiley_face, smiley_face, smiley_face, smiley_face, smiley_face};

		int xmas_size = sizeof(xmas) / sizeof(xmas[0]);
		int arrows_size = sizeof(arrows) / sizeof(arrows[0]);

		int faces_size = sizeof(faces) / sizeof(faces[0]);

		timedControll();
	}
	else if (strcmp(option, "text") == 0) {
		removeSlash(data);

		int index;
		int size = strlen(data);
		unsigned int letters[100][9];

		for (int i = 0; i < size; i++) {
			index = toupper(data[i]);
			copyLetter(letters[i], ASCII[index]);
		}

		shiftingWord(letters, size, 18);
	}
	else if (strcmp(option, "stream") == 0) {
		char fileName[42] = STREAM_FILE;
		FILE *input_frame;
		int fileDescriptor;

		int row;
		char hexStr[9];

		do {
			input_frame = fopen(fileName, "r");
			if (input_frame != NULL) {
				if (flock(fileno(input_frame), LOCK_SH | LOCK_NB) == 0) {
					for (row = 0; row < CONST_ROWS; row++) {
						fscanf(input_frame, "%s ", hexStr);
						display[row] = (unsigned int) strtol(hexStr, NULL, 0);
					}

					flock(fileno(input_frame), LOCK_UN);
				}
			}

			fclose(input_frame);
			input_frame = NULL;

			multiplexing(mux_switch_period_high);

		} while (!stop);
	}
	else if (strcmp(option, "animation") == 0) {
		unsigned int startup_message[][9] = {P, I, D, I, S, P, L, A, Y, SP, R, E, A, D, Y};
		int startup_message_size = sizeof(startup_message) / sizeof(startup_message[0]);
		shiftingWord(startup_message, startup_message_size, 18);
	}
	else {
		spiral();
	}

	return 0;

}
