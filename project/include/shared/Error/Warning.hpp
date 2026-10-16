
#ifndef WARNING_HPP
    #define WARNING_HPP
    #include <iostream>
    #include "IError.hpp"

class Warning : public IError {
    std::string _Warning;
public:
    Warning(std::string msg) : _Warning(msg) {};
    int code() const noexcept override {return 0;};
    const char *what() const noexcept override {return _Warning.c_str();};
};

#endif
