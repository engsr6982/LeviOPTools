#include <functional>
#include <string>
#include <unordered_map>
#include "mc/world/actor/player/Player.h"

namespace tls::form {

extern std::unordered_map<std::string, std::function<void(Player&)>> mapping;
bool                                                                 initMapping();

}