#ifndef EMULATOR_EXCEPTIONS_HPP
#define EMULATOR_EXCEPTIONS_HPP

#include<exception>
#include<string>

class emulator_exception : public std::exception
{
public:
    emulator_exception(std::string _message): message(_message){}
protected:
    virtual const char * what() const throw()
    {
        return message.c_str();
    }
    std::string message;
};

#endif