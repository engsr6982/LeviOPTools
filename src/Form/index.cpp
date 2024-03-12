#include <ll/api/form/SimpleForm.h>

#include "Form/index.h"
#include "mc/world/actor/player/Player.h"

namespace tools::form {

void index(Player& player) {
    using SimpleForm = ll::form::SimpleForm;

    SimpleForm fm;
    fm.setTitle("default");
    fm.appendButton("call", [&](Player& player) { player.sendMessage("button 1"); });
    fm.sendTo(player, [](Player& player, int id) {
        player.sendMessage("send"); // def
    });
}


} // namespace tools::form