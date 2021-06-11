#include<iostream>
#include<string>
#include<fstream>
#include"emulator.hpp"

int main(int argc, char *argv[])
{
    if(argc < 2)
        return 1;
    std::string file_name = std::string(argv[1]);
    emulator _emulator;

    std::ifstream file(file_name);
    _emulator.load_memory(file);
    file.close();

    _emulator.emulate();

    return 0;
}