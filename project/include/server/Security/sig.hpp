#ifndef SIG_HPP
    #define SIG_HPP

    #include <atomic>
    #include <csignal>

inline std::atomic_bool SERVER_RUN = true;

void sigint(int sig);

#endif