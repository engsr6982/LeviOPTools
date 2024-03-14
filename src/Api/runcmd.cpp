#include "api/Global.h"
#include <ll/api/service/Bedrock.h>
#include <ll/api/service/ServerInfo.h>
#include <ll/api/service/Service.h>
#include <ll/api/service/ServiceManager.h>
#include <mc/common/wrapper/optional_ref.h>
#include <mc/server/commands/CommandContext.h>
#include <mc/server/commands/MinecraftCommands.h>
#include <mc/server/commands/PlayerCommandOrigin.h>
#include <mc/world/Minecraft.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Command.h>


namespace tools::api {

bool runCmd(Player& player, const std::string& cmd) {
    try {
        CommandContext context =
            CommandContext(cmd, std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(player)));
        ll::service::getMinecraft()->getCommands().executeCommand(context);
        return true;
    } catch (...) {
        return false;
    }
}


} // namespace tools::api