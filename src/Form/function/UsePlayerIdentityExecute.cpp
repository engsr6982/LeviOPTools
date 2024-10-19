#include "Utils/Mc.h"
#include "include_all.h"


namespace tls::form {

void usePlayerIdentityExecute(Player& player) {
    AutoCheckPermission(player, perms::UsePlayerIdentityExecute);

    CustomForm fm;
    fm.setTitle("LeviOPTools - Use Player Identity Execute"_tr());

    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& p) {
            fm.appendToggle(p.getRealName(), p.getRealName(), false);
            return true;
        });
    }

    fm.appendInput("Command", "Command"_tr(), "string");

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return Utils::sendMsg(pl, "form cancelled"_tr());
        DebugFormCallBack(dt);

        string Command = std::get<string>(dt->at("Command"));

        for (auto const& [name, value] : *dt) {
            if (std::holds_alternative<uint64_t>(value)) {
                auto isTrue = std::get<uint64_t>(value);
                if (isTrue) {
                    Player* playerPtr = ll::service::getLevel()->getPlayer(name);
                    if (playerPtr) {
                        mc::executeCommand(*playerPtr, Command);
                    } else {
                        Utils::sendMsg(pl, "Failed to get player {} pointer"_tr(name));
                    }
                }
            }
        }
        Utils::sendMsg(pl, "Operation completed"_tr());
    });
}

} // namespace tls::form