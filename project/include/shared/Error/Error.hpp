
#ifndef ERROR_HPP
    #define ERROR_HPP
    #include <iostream>
    #include "IError.hpp"

class Error : public IError {
    std::string _Error;
public:
    Error(std::string msg) : _Error(msg) {};
    int code() const noexcept override {return 84;};
    const char *what() const noexcept override {return _Error.c_str();};
};

#endif
