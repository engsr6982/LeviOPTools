#include "include_all.h"

namespace tls::form {

void usePlayerIdentitySay(Player& player) {
    AutoCheckPermission(player, perms::UsePlayerIdentitySay);

    CustomForm fm;
    fm.setTitle("LeviOProTools - Use Player Identity Say"_tr());

    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& p) {
            fm.appendToggle(p.getRealName(), p.getRealName(), false);
            return true;
        });
    }

    fm.appendInput("Message", "Message"_tr(), "string");

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return sendMsg(pl, "form cancelled"_tr());
        DebugFormCallBack(dt);

        string Message = std::get<string>(dt->at("Message"));

        for (auto const& [name, value] : *dt) {
            if (std::holds_alternative<uint64_t>(value)) {
                auto isTrue = std::get<uint64_t>(value);
                if (isTrue) {
                    Player* playerPtr = ll::service::getLevel()->getPlayer(name);
                    if (playerPtr) {
                        TextPacket pkt =
                            TextPacket::createChat(playerPtr->getRealName(), Message, playerPtr->getXuid(), "");
                        playerPtr->sendNetworkPacket(pkt); // send to all player
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