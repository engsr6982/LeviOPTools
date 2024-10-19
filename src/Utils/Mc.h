#pragma once
#include "fmt/format.h"
#include "ll/api/service/Bedrock.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/enums/CurrentCmdVersion.h"
#include "mc/locale/I18n.h"
#include "mc/locale/Localization.h"
#include "mc/server/ServerLevel.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandOutputType.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/ServerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/player/Player.h"
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
#include <string>


namespace tls::mc {


inline void executeCommand(Player& player, std::string const& cmd) {
    CommandContext context = CommandContext(cmd, std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(player)));
    ll::service::getMinecraft()->getCommands().executeCommand(context);
}

inline std::pair<bool, std::string> executeCommandEx(std::string const& cmd) {
    auto result = std::make_pair<bool, std::string>(false, "");

    ServerCommandOrigin origin =
        ServerCommandOrigin("Server", ll::service::getLevel()->asServer(), CommandPermissionLevel::Internal, 0);

    Command* command = ll::service::getMinecraft()->getCommands().compileCommand(
        HashedString(cmd),
        origin,
        (CurrentCmdVersion)CommandVersion::CurrentVersion,
        [](std::string const&) {}
    );
    CommandOutput output(CommandOutputType::AllOutput);

    auto& outputStr = result.second;
    if (command) {
        command->run(origin, output);
        for (auto msg : output.getMessages()) {
            std::string tmp;
            getI18n().getCurrentLanguage()->get(msg.getMessageId(), tmp, msg.getParams());
            outputStr += tmp.append("\n");
        }
        if (output.getMessages().size()) {
            outputStr.pop_back();
        }
        result.first = output.getSuccessCount() ? true : false;
    }
    return result;
}


} // namespace tls::mc