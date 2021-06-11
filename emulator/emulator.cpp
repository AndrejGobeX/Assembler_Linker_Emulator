#include"emulator.hpp"
#include<sstream>
#include<iostream>
#include<chrono>

const unsigned short emulator::instruction_length[] = {1, 2, 1, 3, 1, 3, 2, 2, 2, 2, 3, 3};

void emulator::load_memory(std::ifstream & file)
{
    std::string line;
    max_mem = 0;
    while(std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string lineno;
        ss >> lineno;
        unsigned short byte;
        while(ss >> std::hex >> byte)
        {
            write_memory_byte(max_mem, byte);
            ++max_mem;
        }
    }
}

void emulator::emulate()
{
    _terminal.setup();
    running = true;
    pc = read_memory_word(0);

    while(running)
    {
        parse_instruction();
        tick();
        _terminal.read_char();
        check_interrupts();
    }

    _terminal.clean();
}

void emulator::parse_instruction()
{
    unsigned short pc_backup = pc;
    unsigned char byte = read_memory_byte(pc++);
    unsigned char instruction[5];
    instruction[0] = byte;
    if(byte > 0xB0)
    {
        --pc;
        interrupt(interrupt_invalid);
        return;
    }
    unsigned short length = instruction_length[byte >> 4];
    if(--length)
    {
        byte = read_memory_byte(pc++);

    }
}

void emulator::check_interrupts()
{

}

void emulator::tick()
{
    time_current = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
    if(time_current - time_previous >= time_interval)
        interrupt(interrupt_timer);
    
    unsigned short cfg = memory[timer_cfg];
    switch (cfg)
    {
    case 0:
        time_interval = 500;
        break;
    
    case 1:
        time_interval = 1000;
        break;
    
    case 2:
        time_interval = 1500;
        break;

    case 3:
        time_interval = 2000;
        break;

    case 4:
        time_interval = 5000;
        break;

    case 5:
        time_interval = 10000;
        break;

    case 6:
        time_interval = 30000;
        break;

    case 7:
        time_interval = 60000;
        break;

    default:
        break;
    }
}