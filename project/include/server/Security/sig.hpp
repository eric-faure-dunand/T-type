#ifndef SIG_HPP
    #define SIG_HPP

    #include <atomic>

inline std::atomic_bool SERVER_RUN = true;

void sigint(int sig);

#endif