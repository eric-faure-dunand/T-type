
#ifndef IERROR_HPP
    #define IERROR_HPP
    #include <exception>
    #include <iostream>

    #include "printer.hpp"

class IError : public std::exception {
public:
    virtual int code() const noexcept = 0;
    virtual const char *what() const noexcept = 0;
};

inline std::ostream& operator<<(std::ostream& input, const IError& e) {
    if (e.code() == 84)
        input << Color::RED << "Error : " << Color::RESET;
    else if (e.code() == 2)
        input << Color::MAGENTA << "File not Found : " << Color::RESET;
    else
        input << Color::YELLOW << "Warning : " << Color::RESET;
    return input << e.what() << std::endl;
};

#endif
