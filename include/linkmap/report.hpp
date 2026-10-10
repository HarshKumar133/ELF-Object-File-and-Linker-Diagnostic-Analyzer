#pragma once

#include "linkmap/diagnostic.hpp"

#include <string>
#include <string_view>

namespace linkmap {
[[nodiscard]] std::string renderText(const Analysis& analysis, std::string_view command,
                                     std::string_view verbosity = "normal");
[[nodiscard]] std::string renderJson(const Analysis& analysis, std::string_view command);
[[nodiscard]] std::string renderDot(const Analysis& analysis);
[[nodiscard]] std::string renderSymbolDot(const Analysis& analysis, std::string_view symbol);
}
