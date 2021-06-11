#include"emulator.hpp"
#include"emulator_exceptions.hpp"
#include<sstream>
#include<iostream>
#include<chrono>
#include<unistd.h>

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

void emulator::push(unsigned short val)
{
    sp -= 2;
    write_memory_word(sp, val);
}

unsigned short emulator::pop()
{
    unsigned short t = read_memory_word(sp);
    sp += 2;
    return t;
}

void emulator::emulate()
{
    psw[I] = 0;
    write_memory_word(timer_cfg, 0);
    _terminal.setup();
    running = true;
    sp = term_out;
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
        instruction[1] = byte;
        if(--length)
        {
            byte = read_memory_byte(pc++);
            instruction[2] = byte;
            byte &= 0xF;
            unsigned short payload = 0;
            switch (byte)
            {
            case 0: case 3: case 4: case 5:
                payload = 1;
                break;
            default:
                payload = 0;
                break;
            }
            if(payload)
            {
                instruction[3] = read_memory_byte(pc++);
                instruction[4] = read_memory_byte(pc++);
            }
        }
    }
    try
    {
        execute_instruction(instruction);
    }
    catch(emulator_exception e)
    {
        pc = pc_backup;
        interrupt(interrupt_invalid);
    }
}

void emulator::execute_instruction(unsigned char instruction[])
{
    
    
    unsigned char code = (instruction[0] >> 4) & 0xF;
    unsigned short entry, t;
    bool c;
    std::bitset<16> bits;
    switch (code)
    {
    case HALT:
        
        running = false;
        break;
    case INT:
        
        entry = ( r[(instruction[1] >> 4)] & 0x111 );
        push(pc);
        push(psw.to_ulong());
        pc = read_memory_word(entry * 2);
        psw[I] = 0;
        break;
    case IRET:
        
        psw = pop();
        pc = pop();
        
        break;
    case CALL:
        
        t = get_operand(instruction);
        push(pc);
        pc = t;
        break;
    case RET:
        
        pc = pop();
        break;
    case JMP:
        
        switch_jmp(instruction);
        break;
    case XCHG:
        
        t = r[instruction[1] & 0xF];
        r[instruction[1] & 0xF] = r[(instruction[1] >> 4) & 0xF];
        r[(instruction[1] >> 4) & 0xF] = t;
        break;
    case ARIT:
        
        switch_arit(instruction);
        break;
    case LOG:
        
        switch_log(instruction);
        break;
    case SH:
        
        c = false;
        if((instruction[0] & 0xF) == 0)
        {
            bits = r[(instruction[1] >> 4) & 0xF];
            r[(instruction[1] >> 4) & 0xF] <<= r[instruction[1] & 0xF];
            if(r[instruction[1] & 0xF] != 0 && r[instruction[1] & 0xF] <= 16)
                c = bits[16 - r[instruction[1] & 0xF]];
        }
        else if((instruction[0] & 0xF) == 0)
        {
            bits = r[(instruction[1] >> 4) & 0xF];
            r[(instruction[1] >> 4) & 0xF] >>= r[instruction[1] & 0xF];
            if(r[instruction[1] & 0xF] != 0 && r[instruction[1] & 0xF] <= 16)
                c = bits[r[instruction[1] & 0xF] - 1];
        }
        else
        {
            throw emulator_exception("Invalid instruction");
        }
        psw[N] = 0;
        psw[Z] = 0;
        psw[C] = 0;
        if(r[(instruction[1] >> 4) & 0xF] & 0x8000)
            psw[N] = 1;
        else if(r[(instruction[1] >> 4) & 0xF] == 0)
            psw[Z] = 1;
        if(c)
            psw[C] = 1;
        break;
    case LDR:
        
        t = get_operand(instruction);
        r[(instruction[1] >> 4) & 0xF] = t;
        break;
    case STR:
        
        
        set_operand(instruction);
        
        break;
    default:
        break;
    }
    //for(long long slp = 0; slp < 500000000L; ++slp);
}

void emulator::check_interrupts()
{
    if(psw[I])
        return;
    unsigned short entry = 0xFFFF;
    for(unsigned short i = 0; i < 8; ++i)
    {
        if(interrupt_flags[i])
        {
            entry = i;
            break;
        }
    }
    if(entry != 0xFFFF)
    {
        if(entry == interrupt_terminal)
        {
            if(psw[Tl])
                return;
        }
        if(entry == interrupt_timer)
        {
            if(psw[Tr])
                return;
        }
        
        
        interrupt_flags[entry] = false;
        push(pc);
        push(psw.to_ulong());
        pc = read_memory_word(entry * 2);
        
        psw[I] = 1;
        if(entry == interrupt_terminal)psw[Tl] = 1;
        if(entry == interrupt_timer)psw[Tr] = 1;
    }
}

unsigned short emulator::get_operand(unsigned char instruction[])
{
    unsigned short ret;
    unsigned short update = (instruction[2] >> 4) & 0xF;
    unsigned char code = (instruction[0] >> 4) & 0xF;
    switch (instruction[2] & 0xF)
    {
    case IMM:
        ret = instruction[3];
        ret += ((unsigned short)instruction[4]) << 8;
        break;
    case REGDIR:
        ret = r[instruction[1] & 0xF];
        break;
    case REGIND:
        if(update == 1)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 2)
            r[instruction[1] & 0xF] += 2;
        ret = r[instruction[1] & 0xF];
        ret = read_memory_word(ret);
        if(update == 3)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 4)
            r[instruction[1] & 0xF] += 2;
        break;
    case REGINDOFF:
        if(update == 1)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 2)
            r[instruction[1] & 0xF] += 2;
        ret = r[instruction[1] & 0xF];
        ret += instruction[3];
        ret += ((unsigned short)instruction[4]) << 8;
        ret = read_memory_word(ret);
        if(update == 3)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 4)
            r[instruction[1] & 0xF] += 2;
        break;
    case MEM:
        ret = instruction[3];
        ret += ((unsigned short)instruction[4]) << 8;
        ret = read_memory_word(ret);
        break;
    case REGDIROFF:
        ret = r[instruction[1] & 0xF];
        
        ret += instruction[3];
        ret += ((unsigned short)instruction[4]) << 8;
        if(code == LDR)
        {
            
            ret = read_memory_word(ret);
            
        }
        break;
    default:
        throw emulator_exception("Invalid instruction");
        break;
    }
    return ret;
}

void emulator::set_operand(unsigned char instruction[])
{
    unsigned short ret;
    unsigned short update = (instruction[2] >> 4) & 0xF;
    switch (instruction[2] & 0xF)
    {
    case REGDIR:
        r[instruction[1] & 0xF] = r[(instruction[1] >> 4) & 0xF];
        break;
    case REGIND:
        if(update == 1)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 2)
            r[instruction[1] & 0xF] += 2;
        ret = r[instruction[1] & 0xF];
        write_memory_word(ret, r[(instruction[1] >> 4) & 0xF]);
        if(update == 3)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 4)
            r[instruction[1] & 0xF] += 2;
        break;
    case REGINDOFF:
        if(update == 1)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 2)
            r[instruction[1] & 0xF] += 2;
        ret = r[instruction[1] & 0xF];
        ret += instruction[3];
        ret += ((unsigned short)instruction[4]) << 8;
        write_memory_word(ret, r[(instruction[1] >> 4) & 0xF]);
        if(update == 3)
            r[instruction[1] & 0xF] -= 2;
        else if(update == 4)
            r[instruction[1] & 0xF] += 2;
        break;
    case MEM:
        ret = instruction[3];
        ret += ((unsigned short)instruction[4]) << 8;
        write_memory_word(ret, r[(instruction[1] >> 4) & 0xF]);
        break;
        //TODO: sefefs
    case REGDIROFF:
        ret = r[instruction[1] & 0xF];
        ret += instruction[3];
        ret += ((unsigned short)instruction[4]) << 8;
        write_memory_word(ret, r[(instruction[1] >> 4) & 0xF]);
        break;
    default: case IMM:
        throw emulator_exception("Invalid instruction");
        break;
    }
}

void emulator::switch_jmp(unsigned char instruction[])
{
    switch (instruction[0] & 0xF)
    {
    case 0:
        break;
    case 1:
        if(!psw[Z])
            return;
        break;
    case 2:
        if(psw[Z])
            return;
        break;
    case 3:
        if((psw[N]^psw[O]) || psw[Z])
            return;
        break;
    default:
        throw emulator_exception("Invalid instruction");
        return;
    }
    pc = get_operand(instruction);
}

void emulator::switch_arit(unsigned char instruction[])
{
    unsigned short t;
    switch (instruction[0] & 0xF)
    {
    case 0:
        r[(instruction[1] >> 4) & 0xF] += r[instruction[1] & 0xF];
        break;
    case 1:
        r[(instruction[1] >> 4) & 0xF] -= r[instruction[1] & 0xF];
        break;
    case 2:
        r[(instruction[1] >> 4) & 0xF] *= r[instruction[1] & 0xF];
        break;
    case 3:
        r[(instruction[1] >> 4) & 0xF] /= r[instruction[1] & 0xF];
        break;
    case 4:
        t = r[(instruction[1] >> 4) & 0xF] - r[instruction[1] & 0xF];
        
        
        
        psw[N] = 0;
        psw[Z] = 0;
        psw[O] = 0;
        psw[C] = 0;
        if(t & 0x8000)
            psw[N] = 1;
        else if(t == 0)
            psw[Z] = 1;
        if(r[(instruction[1] >> 4) & 0xF] < r[instruction[1] & 0xF])
            psw[C] = 1;
        if((r[(instruction[1] >> 4) & 0xF]>0 && r[instruction[1] & 0xF]<0 && t<0) || (r[(instruction[1] >> 4) & 0xF]<0 && r[instruction[1] & 0xF]>0 && t>0))
            psw[O] = 1;
        break;
    default:
        throw emulator_exception("Invalid instruction");
        return;
    }
}

void emulator::switch_log(unsigned char instruction[])
{
    unsigned short t;
    switch(instruction[0] & 0xF)
    {
    case 0:
        r[(instruction[1] >> 4) & 0xF] = ~r[(instruction[1] >> 4) & 0xF];
        break;
    case 1:
        r[(instruction[1] >> 4) & 0xF] &= r[instruction[1] & 0xF];
        break;
    case 2:
        r[(instruction[1] >> 4) & 0xF] |= r[instruction[1] & 0xF];
        break;
    case 3:
        r[(instruction[1] >> 4) & 0xF] ^= r[instruction[1] & 0xF];
        break;
    case 4:
        t = r[(instruction[1] >> 4) & 0xF] & r[instruction[1] & 0xF];
        psw[N] = 0;
        psw[Z] = 0;
        if(t & 0x8000)
            psw[N] = 1;
        else if(t == 0)
            psw[Z] = 1;
        break;
    default:
        throw emulator_exception("Invalid instruction");
        return;
    }
}

void emulator::tick()
{
    time_current = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
    if(time_current - time_previous >= time_interval)
    {
        interrupt(interrupt_timer);
        time_previous = time_current;
    }
    
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