#include "file/file.h"
#include "nlohmann/json_fwd.hpp"
#include <fstream>
#include <optional>

namespace tools::file {

std::optional<nlohmann::json> loadJsonFile(const std::filesystem::path& path) {
    try {
        std::ifstream file(path);
        if (file.good()) {
            return nlohmann::json::parse(file);
        }
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
    return std::nullopt;
}


} // namespace tools::file