#include "sig.hpp"

void sigint(int sig) {
    (void)sig;
    SERVER_RUN = false;
}
