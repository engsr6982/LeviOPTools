#include "include_all.h"

namespace tls::form {

void killPlayer(Player& player) {
    AutoCheckPermission(player, perms::KillPlayer);

    CustomForm fm;
    fm.setTitle("LeviOPTools - Kill Player"_tr());

    fm.appendLabel("This feature can kill any player, regardless of game mode."_tr());

    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& p) {
            fm.appendToggle(p.getRealName(), p.getRealName(), false);
            return true;
        });
    }

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return Utils::sendMsg(pl, "form cancelled"_tr());
        DebugFormCallBack(dt);

        for (auto const& [name, value] : *dt) {
            if (std::holds_alternative<uint64_t>(value)) {
                auto isTrue = std::get<uint64_t>(value);
                if (isTrue) {
                    Player* playerPtr = ll::service::getLevel()->getPlayer(name);
                    if (playerPtr) {
                        playerPtr->kill();
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