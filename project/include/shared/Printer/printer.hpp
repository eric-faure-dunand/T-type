
#ifndef PRINTER_HPP
    #define PRINTER_HPP
    #include <iostream>
    #include <cstdint>

namespace Color {
    constexpr const char* BLACK   = "\033[30m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* BLUE    = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* WHITE   = "\033[37m";

    constexpr const char* BRIGHT_BLACK   = "\033[90m";
    constexpr const char* BRIGHT_RED     = "\033[91m";
    constexpr const char* BRIGHT_GREEN   = "\033[92m";
    constexpr const char* BRIGHT_YELLOW  = "\033[93m";
    constexpr const char* BRIGHT_BLUE    = "\033[94m";
    constexpr const char* BRIGHT_MAGENTA = "\033[95m";
    constexpr const char* BRIGHT_CYAN    = "\033[96m";
    constexpr const char* BRIGHT_WHITE   = "\033[97m";

    constexpr const char* BG_BLACK   = "\033[40m";
    constexpr const char* BG_RED     = "\033[41m";
    constexpr const char* BG_GREEN   = "\033[42m";
    constexpr const char* BG_YELLOW  = "\033[43m";
    constexpr const char* BG_BLUE    = "\033[44m";
    constexpr const char* BG_MAGENTA = "\033[45m";
    constexpr const char* BG_CYAN    = "\033[46m";
    constexpr const char* BG_WHITE   = "\033[47m";

    constexpr const char* BG_BRIGHT_BLACK   = "\033[100m";
    constexpr const char* BG_BRIGHT_RED     = "\033[101m";
    constexpr const char* BG_BRIGHT_GREEN   = "\033[102m";
    constexpr const char* BG_BRIGHT_YELLOW  = "\033[103m";
    constexpr const char* BG_BRIGHT_BLUE    = "\033[104m";
    constexpr const char* BG_BRIGHT_MAGENTA = "\033[105m";
    constexpr const char* BG_BRIGHT_CYAN    = "\033[106m";
    constexpr const char* BG_BRIGHT_WHITE   = "\033[107m";

    constexpr const char* BOLD      = "\033[1m";
    constexpr const char* DIM       = "\033[2m";
    constexpr const char* ITALIC    = "\033[3m";
    constexpr const char* UNDERLINE = "\033[4m";
    constexpr const char* BLINK     = "\033[5m";
    constexpr const char* REVERSE   = "\033[7m";
    constexpr const char* STRIKE    = "\033[9m";

    constexpr const char* RESET = "\033[0m";
}

class Printer {
    std::uint32_t LineWrite = 0;
public:
    Printer() = default;
    ~Printer() = default;

    void MoveCursorUp(int n) {std::cout << "\033[" << n << "A";};
    void ClearLine() {std::cout << "\033[2K\r";};
    void PrintLine(std::string line) {std::cout << line << std::endl; LineWrite++;};
    void PrintLine(std::string line, int nb_line) {std::cout << line << std::endl; LineWrite += nb_line;};

    void ClearNbLine(int nb) {for (int i = nb; i > 0; i--) {std::cout << "\033[1A"; std::cout << "\033[2K\r";}};
    void ClearToStart() {this->ClearNbLine(LineWrite); LineWrite = 0;};
    void SetStart() {LineWrite = 0;};
};

#endif
