#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include<vector>
#include<fstream>
#include"terminal.hpp"
#include<bitset>

class emulator
{
public:
    static const unsigned short
        term_out = 0xFF00,
        term_in = 0xFF02,
        timer_cfg = 0xFF10,
        interrupt_invalid = 1,
        interrupt_timer = 2,
        interrupt_terminal = 3;
    static const unsigned short instruction_length[];
    static const size_t mem_size = 0x10000;
    emulator(): _terminal(*this), memory(mem_size), pc(r[7]), sp(r[6])
    {
        short i = 8;
        while(i)
        {
            interrupt_flags[i] = false;
            r[i] = 0;
        }
        //memory = std::vector<unsigned char>(mem_size);
    }
    void load_memory(std::ifstream & file);
    void write_memory_byte(unsigned short location, unsigned char val)
    {
        memory[location] = val;
    }
    void write_memory_word(unsigned short location, unsigned short val)
    {
        memory[location] = val;
        memory[location + 1] = (val >> 8);
    }
    unsigned char read_memory_byte(unsigned short location){ return memory[location]; }
    unsigned short read_memory_word(unsigned short location)
        { return (((unsigned short)memory[location]) << 8) + (unsigned short)memory[location + 1]; }
    void interrupt(unsigned short entry){ interrupt_flags[entry] = true; }
    bool interrupt_is_set(unsigned short entry){ return interrupt_flags[entry]; }
    void parse_instruction();
    void check_interrupts();
    void emulate();
    void tick();

private:
    bool running;
    unsigned short max_mem;
    std::vector<unsigned char> memory;
    terminal _terminal;
    bool interrupt_flags[8];

    unsigned short r[8];
    unsigned short & pc, & sp;
    std::bitset<16> psw;

    long long int time_current, time_previous, time_interval = 500;
};

#endif
