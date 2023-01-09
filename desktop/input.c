#include "input.h"
#include "termios.h"
#include <unistd.h>

static struct termios oldterm, newterm;

void input_setup(void) {
  tcgetattr(STDIN_FILENO, &oldterm);
  newterm = oldterm;
  newterm.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newterm);
}

void input_restore(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
}


