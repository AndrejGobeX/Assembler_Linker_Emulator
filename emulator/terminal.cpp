#include "emulator.hpp"
#include "emulator_exceptions.hpp"
#include "terminal.hpp"
#include "memory.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>

struct termios terminalBackup;
volatile int killed = 0;

void restoreFlags() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminalBackup);
}

void cleanTerminal(int sig){ 
  restoreFlags();
  killed = 1;
}

void terminal::setup() {
  if (tcgetattr(STDIN_FILENO, &terminalBackup) < 0) {
    throw emulator_exception("Terminal cannot be started");
  }

  static struct termios raw = terminalBackup;
  raw.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
  raw.c_cflag &= ~(CSIZE | PARENB);
  raw.c_cflag |= CS8;
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;

  if (atexit(restoreFlags) != 0) {
    throw emulator_exception("Cannot restore terminal");
  }

  atexit(restoreFlags);
  signal(SIGINT, cleanTerminal); 

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)) {
    throw emulator_exception("Terminal cannot be configured");
  }
}

void terminal::clean() { restoreFlags(); }

void terminal::read_char() {
  char c;
  if (read(STDIN_FILENO, &c, 1) == 1) {
    buffer.push(c);
  }

  if (buffer.size() > 0 && !_emulator.interrupt_is_set(emulator::interrupt_terminal)) {
    _emulator.write_memory_byte(emulator::term_in, buffer.front());
    buffer.pop();
    _emulator.interrupt(emulator::interrupt_terminal);
  }

  if (killed) throw emulator_exception("Terminal is not running");
}