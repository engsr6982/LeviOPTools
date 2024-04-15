#include <nlohmann/json.hpp>
#include <optional>


namespace tls::file {

std::optional<nlohmann::json> loadJsonFile(const std::filesystem::path&);

}