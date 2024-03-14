#include "form/Global.h"
#include "mc/world/actor/player/Player.h"
#include <functional>
#include <string>
#include <unordered_map>

namespace tools::form {

std::unordered_map<std::string, std::function<void(Player&)>> mapping;

void test(Player& player) { player.sendMessage("test"); }

bool initMapping() {
    mapping["test"] = test;
    return true;
}


} // namespace tools::form