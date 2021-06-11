#ifndef TERMINAL_HPP
#define TERMINAL_HPP
#include <queue>

class emulator;

class terminal
{
public:
  terminal(emulator & _emulator) : _emulator(_emulator) {}
  void setup();
  void clean();
  void read_char();
  ~terminal() {
    if (started)
      clean();
  }
private:
  emulator & _emulator;
  std::queue<char> buffer;
  bool started = false;
};
#endif