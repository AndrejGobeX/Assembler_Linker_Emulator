#ifndef LINE_HPP
#define LINE_HPP

#include<string>
#include<vector>

class arg;

class line
{
public:
    line(std::string _name);
    void add_arg(arg _arg);
    void clear(){ args.clear(); jmp = false; }
    void set_name(std::string _name){ name = _name; }
    void set_jmp(){ jmp = true; }
    bool get_jmp(){ return jmp; }
    std::string get_name() { return name; }
    std::vector<arg> & get_args() { return args; }
private:
    std::string name;
    std::vector<arg> args;
    bool jmp = false;
};

#endif