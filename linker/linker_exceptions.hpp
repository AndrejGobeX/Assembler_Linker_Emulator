#ifndef LINKER_EXCEPTIONS_HPP
#define LINKER_EXCEPTIONS_HPP

#include<exception>
#include<string>

class linker_exception : public std::exception
{
public:
    linker_exception(std::string _message): message(_message){}
protected:
    virtual const char * what() const throw()
    {
        return message.c_str();
    }
    std::string message;
};

#endif