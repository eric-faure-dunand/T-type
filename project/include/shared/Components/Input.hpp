#ifndef INPUT_HPP
    #define INPUT_HPP

namespace r_type {

// Rempli par le client (clavier) ou par le serveur (paquet reçu)
struct Input {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
};

}

#endif
