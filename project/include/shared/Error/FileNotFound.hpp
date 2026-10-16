
#ifndef FILE_NOT_FOUND_HPP
    #define FILE_NOT_FOUND_HPP
    #include <iostream>
    #include "IError.hpp"

class FileNotFound : public IError {
    std::string _FileNotFound;
public:
    FileNotFound(std::string msg) : _FileNotFound(msg) {};
    int code() const noexcept override {return 2;};
    const char *what() const noexcept override {return _FileNotFound.c_str();};
};

#endif
