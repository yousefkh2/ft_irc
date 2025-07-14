#pragma once
#include <vector>
#include "Command.hpp"

class Parser {
public:
    std::vector<Command> parse(std::string& data, size_t& used);
};
