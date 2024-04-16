#include "mc/world/actor/player/Player.h"
#include <functional>
#include <string>
#include <unordered_map>


namespace tls::form {

extern std::unordered_map<std::string, std::function<void(Player&)>> mapping;
bool                                                                 initMapping();

} // namespace tls::form