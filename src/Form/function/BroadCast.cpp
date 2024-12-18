#include "include_all.h"
#include "mc/enums/TextPacketType.h"

namespace tls::form {

void broadCastMessage(Player& player) {
    AutoCheckPermission(player, perms::BroadCastMessage);

    CustomForm fm;
    fm.setTitle("LeviOPTools - BroadCast"_tr());

    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& p) {
            fm.appendToggle(p.getRealName(), p.getRealName(), false);
            return true;
        });
    }

    fm.appendInput("Message", "Message"_tr(), "string");

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return Utils::sendMsg(pl, "form cancelled"_tr());
        DebugFormCallBack(dt);

        string Message = std::get<string>(dt->at("Message"));

        TextPacket pkt{};
        pkt.mType    = TextPacketType::Chat;
        pkt.mAuthor  = "Server";
        pkt.mMessage = Message;

        for (auto const& [name, value] : *dt) {
            if (std::holds_alternative<uint64_t>(value)) {
                auto isTrue = std::get<uint64_t>(value);
                if (isTrue) {
                    Player* playerPtr = ll::service::getLevel()->getPlayer(name);
                    if (playerPtr) {
                        playerPtr->sendNetworkPacket(pkt); // send to all player
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