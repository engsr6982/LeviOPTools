#include "Form/index.h"
#include "entry/PluginInfo.h"

#include <ll/api/form/CustomForm.h>
#include <ll/api/form/ModalForm.h>
#include <ll/api/form/SimpleForm.h>
#include <mc/world/actor/player/Player.h>


namespace tools::form {

using SimpleForm = ll::form::SimpleForm;
using ModalForm  = ll::form::ModalForm;
using CustomForm = ll::form::CustomForm;

void index(Player& player) {
    SimpleForm fm;
    fm.setTitle(PLUGIN_NAME);
    fm.appendButton("call", [&](Player& player) { player.sendMessage("button 1"); });
    fm.sendTo(player);
}


} // namespace tools::form