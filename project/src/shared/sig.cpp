#include "sig.hpp"

void sigint(int sig) {
    (void)sig;
    APP_RUN = false;
}
