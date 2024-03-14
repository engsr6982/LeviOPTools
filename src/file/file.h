#include <nlohmann/json.hpp>
#include <optional>


namespace tools::file {

std::optional<nlohmann::json> loadJsonFile(const std::filesystem::path&);


}