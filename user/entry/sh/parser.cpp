#include "parser.hpp"

#include <ranges>

std::string_view trim(std::string_view str) {
    auto left = str.find_first_not_of(" \n\t");
    if (left == std::string_view::npos) {
        return {};
    }
    auto right = str.find_last_not_of(" \n\t");
    return str.substr(left, right - left + 1);
}

std::vector<Command> parseCommand(const std::string& expr) {
    std::vector<Command> result;
    for (auto __sub : expr | std::views::split('|')) {
        std::string_view subExpr{__sub.begin(), __sub.end()};
        Command cmd;

        if (auto pos = subExpr.find("2>"); pos != std::string_view::npos) {
            cmd.__stderr = trim(subExpr.substr(pos + 2));
            subExpr = subExpr.substr(0, pos - 1);
        }

        if (auto pos = subExpr.find(">"); pos != std::string_view::npos) {
            cmd.__stdout = trim(subExpr.substr(pos + 1));
            subExpr = subExpr.substr(0, pos - 1);
        }

        if (auto pos = subExpr.find("<"); pos != std::string_view::npos) {
            cmd.__stdin = trim(subExpr.substr(pos + 1));
            subExpr = subExpr.substr(0, pos - 1);
        }

        for (auto __item : subExpr | std::views::split(' ')) {
            std::string_view item{__item.begin(), __item.end()};
            if (item.empty()) {
                continue;
            }
            cmd.__argv.push_back(std::string{item});
        }

        if (cmd.__argv.empty()) {
            continue;
        }

        result.push_back(std::move(cmd));
    }
    return result;
}
