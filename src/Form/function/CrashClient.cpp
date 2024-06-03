#include "include_all.h"
#include "mc/network/packet/RemoveActorPacket.h"

namespace tls::form {

void crashPlayerClient(Player& player) {
    AutoCheckPermission(player, perms::CrashPlayerClient);

    CustomForm fm;
    fm.setTitle("LeviOPTools - Crash Client"_tr());

    fm.appendLabel(
        "This feature can directly crash the player's client, this feature may not work on platforms such as Win10."_tr(
        )
    );

    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& p) {
            fm.appendToggle(p.getRealName(), p.getRealName(), false);
            return true;
        });
    }

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return sendMsg(pl, "form cancelled"_tr());
        DebugFormCallBack(dt);

        for (auto const& [name, value] : *dt) {
            if (std::holds_alternative<uint64>(value)) {
                auto isTrue = std::get<uint64>(value);
                if (isTrue) {
                    Player* playerPtr = ll::service::getLevel()->getPlayer(name);
                    if (playerPtr) {
                        RemoveActorPacket pkt = RemoveActorPacket(playerPtr->getAgentID());
                        playerPtr->sendNetworkPacket(pkt);
                    } else {
                        sendMsg(pl, "Failed to get player {} pointer"_tr(name));
                    }
                }
            }
        }
        sendMsg(pl, "Operation completed"_tr());
    });
}

} // namespace tls::form