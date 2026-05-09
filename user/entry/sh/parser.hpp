#pragma once

#include <string>
#include <vector>

struct Command {
    std::vector<std::string> __argv;
    std::optional<std::string> __stdin;
    std::optional<std::string> __stdout;
    std::optional<std::string> __stderr;
    bool __stdout_append{};
    bool __stderr_append{};
};

std::vector<Command> parseCommand(const std::string& expr);

// std::vector<
