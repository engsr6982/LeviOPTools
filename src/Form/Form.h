#include "mc/world/actor/player/Player.h"
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

namespace tls::form {

extern std::unordered_map<std::string, std::function<void(Player&)>> mapping;
bool                                                                 initMapping();

// ------------------------------------------- func

void index(Player&, std::filesystem::path);
void index(Player&);

} // namespace tls::form