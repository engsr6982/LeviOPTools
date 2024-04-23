#include "include_all.h"
#include "ll/api/base/StdInt.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/item/ItemStackBase.h"

namespace tls::form {

void kickPlayer(Player& player) {
    AutoCheckPermission(player, perms::KickPlayer);

    CustomForm fm;
    fm.setTitle("LeviOProTools - Kick Player"_tr());

    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& p) {
            fm.appendToggle(p.getRealName(), p.getRealName(), false);
            return true;
        });
    }

    fm.appendInput("reason", "Kick Reason"_tr(), "string");

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return sendMsg(pl, "form cancelled"_tr());
        DebugFormCallBack(dt);
        string reason = std::get<string>(dt->at("reason"));
        for (auto const& [name, value] : *dt) {
            if (std::holds_alternative<uint64_t>(value)) {
                auto isTrue = std::get<uint64_t>(value);
                if (isTrue) {
                    Player* playerPtr = ll::service::getLevel()->getPlayer(name);
                    if (playerPtr) {
                        reason += " "; // 修复传入empty字符串导致玩家断开连接但依然停留在假的世界中
                        playerPtr->disconnect(reason);
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