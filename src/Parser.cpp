#include "../include/Parser.hpp"
#include <vector>
#include <iostream>

std::vector<Command> Parser::parse(std::string &data, size_t &used) {
  std::vector<Command> commands;
  size_t pos = 0;
  while (true) {
    auto line_end = data.find("\r\n", pos);
    if (line_end == std::string::npos)
      break; // no complete line left

    std::string line = data.substr(pos, line_end - pos); // extract the line (without "\r\n")
    pos = line_end + 2;                   // skip past "\r\n"

    if (line.empty())
      continue; // skip blank lines

    Command cmd;
    size_t i = 0;
    // 1. optional prefix
    if (line[i] == ':') {
		size_t prefix_start = i + 1;
		auto space_pos = line.find(' ', prefix_start);

      if (space_pos == std::string::npos || space_pos == prefix_start) { // no space found. e.g. "":myserverNOTICE:Hello"
		std::cerr << "Malformed prefix in line: '" << line << "'\n";
        continue;
	   }
      cmd.prefix = line.substr(prefix_start, space_pos - prefix_start);
      i = space_pos + 1;
    }

    // 2. command name
    auto space = line.find(' ', i);
    bool has_space = (space != std::string::npos);
    size_t end_of_command = has_space ? space : line.size();
    cmd.name = line.substr(i, end_of_command - i);
    i = has_space ? end_of_command + 1 : line.size(); // update the index

    // 3. parameters
    while (i < line.size()) {
      if (line[i] == ':') {
        // trailing parameter: consume the rest as one param
        cmd.params.push_back(line.substr(i + 1));
        break;
      }
      auto next_space = line.find(' ', i);
	  size_t end_of_param = (next_space == std::string::npos) ? line.size() : next_space;
	  std::string param = line.substr(i, end_of_param - i);
      cmd.params.push_back(param);
      i = (next_space == std::string::npos) ? line.size() : next_space + 1;
    }
    commands.push_back(cmd);
  }

  used = pos;
  return commands;
}