#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>

const int CONST_ROWS = 8;

int main(int argc, char* argv[]) {
    if (argc - 1 != CONST_ROWS) return -1;

    FILE *input_frame = fopen("/var/www/html/pidisplay/c/input_frame.txt", "w");

    if (input_frame) {
        if (flock(fileno(input_frame), LOCK_EX) == 0) {
            for (int index = 1; index < argc; index++)
                fprintf(input_frame, "%s ", argv[index]);

            flock(fileno(input_frame), LOCK_UN);
        }
    }

    fclose(input_frame);

    return 0;
}