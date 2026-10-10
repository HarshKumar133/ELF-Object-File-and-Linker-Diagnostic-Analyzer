#include "linkmap/core.hpp"
#include "linkmap/version.hpp"

namespace linkmap {
std::string_view version() noexcept { return LINKMAP_VERSION; }
}
