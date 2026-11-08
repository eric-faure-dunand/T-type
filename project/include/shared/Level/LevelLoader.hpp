/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** LevelLoader.hpp
*/

#ifndef LEVEL_LEVEL_LOADER_HPP
    #define LEVEL_LEVEL_LOADER_HPP

    #include <filesystem>
    #include <stdexcept>
    #include <string>

    #include "LevelData.hpp"

namespace r_type::level {

class LevelError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

LevelData loadFromString(const std::string &json, const std::string &sourceName = "<memory>");

LevelData loadFromFile(const std::filesystem::path &path);

}

#endif