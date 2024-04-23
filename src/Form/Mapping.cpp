#include "Mapping.h"
#include "function/include_all.h"
#include "mc/world/actor/player/Player.h"
#include <functional>
#include <string>
#include <unordered_map>

namespace tls::form {

std::unordered_map<std::string, std::function<void(Player&)>> mapping;

bool initMapping() {
    mapping["kickPlayer"]     = kickPlayer;
    mapping["killPlayer"]     = killPlayer;
    mapping["changeWeather"]  = changeWeather;
    mapping["changeTime"]     = changeTime;
    mapping["changeGameRule"] = changeGameRule;
    mapping["terminal"]       = terminal;
    mapping["crashClient"]    = crashClient;
    return true;
}

} // namespace tls::form