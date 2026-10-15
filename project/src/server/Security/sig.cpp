#include "sig.hpp"

void sigint(__attribute__((unused))int sig) {
    SERVER_RUN = false;
}
