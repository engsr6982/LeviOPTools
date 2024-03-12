#include "Command.h"
#include "Config/Config.h"

#include "form/index.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <ll/api/Logger.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/HashUtils.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOriginType.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandParameterOption.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/server/commands/CommandRegistry.h>
#include <mc/server/commands/CommandSelector.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>


namespace tools::command {

// clang-format off
#define CHECK_COMMAND_TYPE(currentType, targetType)                                                                    \
    if (currentType != targetType) {                                                                                   \
        std::string targetTypeStr;                                                                                     \
        switch (targetType) {                                                                                          \
            case CommandOriginType::Player: targetTypeStr = "players"; break;                                          \
            case CommandOriginType::CommandBlock: targetTypeStr = "command blocks"; break;                             \
            case CommandOriginType::MinecartCommandBlock: targetTypeStr = "minecart command blocks"; break;            \
            case CommandOriginType::DevConsole: targetTypeStr = "the developer console"; break;                        \
            case CommandOriginType::Test: targetTypeStr = "test origins"; break;                                       \
            case CommandOriginType::AutomationPlayer: targetTypeStr = "automation players"; break;                     \
            case CommandOriginType::ClientAutomation: targetTypeStr = "client automation"; break;                      \
            case CommandOriginType::DedicatedServer: targetTypeStr = "dedicated servers"; break;                       \
            case CommandOriginType::Entity: targetTypeStr = "entities"; break;                                         \
            case CommandOriginType::Virtual: targetTypeStr = "virtual origins"; break;                                 \
            case CommandOriginType::GameArgument: targetTypeStr = "game argument origins"; break;                      \
            case CommandOriginType::EntityServer: targetTypeStr = "entity servers"; break;                             \
            case CommandOriginType::Precompiled: targetTypeStr = "precompiled origins"; break;                         \
            case CommandOriginType::GameDirectorEntityServer: targetTypeStr = "game director entity servers"; break;   \
            case CommandOriginType::Scripting: targetTypeStr = "scripting origins"; break;                             \
            case CommandOriginType::ExecuteContext: targetTypeStr = "execute contexts"; break;                         \
            default: targetTypeStr = "unknown";                                                                        \
        }                                                                                                              \
        return output.error("This command is available to [" + targetTypeStr + "] only!");                             \
    }
// clang-format on

bool regCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.commandName,
        config::cfg.command.commandDescription
    );

    // tools
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(origin.getOriginType(), CommandOriginType::Player);
        Actor* entity = origin.getEntity();
        if (entity) {
            auto& player = *static_cast<Player*>(entity); // entity* => Player&
            tools::form::index(player);
        }
    }>();

    return true;
}

} // namespace tools::command
