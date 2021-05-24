#ifndef ARG_HPP
#define ARG_HPP

#include<string>
#include<iostream>
#include<vector>

class arg
{
public:
    static arg make_arg(int _arg, unsigned char _type);
    static arg make_arg(std::string _arg, unsigned char _type);
    arg(){}
    arg(int _arg)
    {
        literal = true;
        argl = _arg;
    }
    arg(std::string _arg)
    {
        literal = false;
        argi = _arg;
    }
    int get_argl() { return argl; }
    void set_type(unsigned char _type) { type = _type; }
    unsigned char get_type() { return type; }
    std::string get_argi() { return argi; }
    void add_offset(int _arg){ offsets.push_back(arg(_arg));}
    void add_offset(std::string _arg){ offsets.push_back(arg(_arg));}
    arg & get_offset(){ return offsets[0]; }
    bool has_offset(){ return offsets.size()>0; }
    bool get_literal(){ return literal; }
private:
    bool literal;
    int argl;
    std::string argi;
    unsigned char type;
    std::vector<arg> offsets;
    friend std::ostream &operator<<(std::ostream &os, const arg &_arg)
    {
        return os << "arg";
    }
};

#endif