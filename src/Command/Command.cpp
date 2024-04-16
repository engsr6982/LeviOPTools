#include "Command.h"
#include "File/Config.h"
#include "Form/index.h"
#include <ll/api/Logger.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/service/PlayerInfo.h>
#include <ll/api/service/Service.h>
#include <ll/api/utils/HashUtils.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/network/packet/LevelChunkPacket.h>
#include <mc/network/packet/TextPacket.h>
#include <mc/server/ServerLevel.h>
#include <mc/server/ServerPlayer.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOriginType.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandParameterOption.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/server/commands/CommandRegistry.h>
#include <mc/server/commands/CommandSelector.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>
#include <string>


namespace tls::command {

using ll::i18n_literals::operator""_tr;
using string = std::string;

#define C_RESET  "\033[0m"
#define C_BLACK  "\033[30m" /* Black */
#define C_RED    "\033[31m" /* Red */
#define C_GREEN  "\033[32m" /* Green */
#define C_YELLOW "\033[33m" /* Yellow */
#define C_BLUE   "\033[34m" /* Blue */
#define C_PURPLE "\033[35m" /* Purple */
#define C_CYAN   "\033[36m" /* Cyan */
#define C_WHITE  "\033[37m" /* White */

std::string CommandOriginTypeToString(CommandOriginType type) {
    switch (type) {
    case CommandOriginType::Player:
        return "players";
    case CommandOriginType::CommandBlock:
        return "command blocks";
    case CommandOriginType::MinecartCommandBlock:
        return "minecart command blocks";
    case CommandOriginType::DevConsole:
        return "the developer console";
    case CommandOriginType::Test:
        return "test origins";
    case CommandOriginType::AutomationPlayer:
        return "automation players";
    case CommandOriginType::ClientAutomation:
        return "client automation";
    case CommandOriginType::DedicatedServer:
        return "dedicated servers";
    case CommandOriginType::Entity:
        return "entities";
    case CommandOriginType::Virtual:
        return "virtual origins";
    case CommandOriginType::GameArgument:
        return "game argument origins";
    case CommandOriginType::EntityServer:
        return "entity servers";
    case CommandOriginType::Precompiled:
        return "precompiled origins";
    case CommandOriginType::GameDirectorEntityServer:
        return "game director entity servers";
    case CommandOriginType::Scripting:
        return "scripting origins";
    case CommandOriginType::ExecuteContext:
        return "execute contexts";
    default:
        return "unknown";
    }
}

#define CHECK_COMMAND_TYPE(__output, __originType, ...)                                                        \
    {                                                                                                                  \
        std::initializer_list<CommandOriginType> __allowedTypes = {__VA_ARGS__};                                       \
        bool                                     __typeMatched  = false;                                               \
        for (auto _allowedType : __allowedTypes) {                                                                     \
            if (__originType == _allowedType) {                                                                        \
                __typeMatched = true;                                                                                  \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        if (!__typeMatched) {                                                                                          \
            std::stringstream __allowedTypesStr;                                                                       \
            bool              __first = true;                                                                          \
            for (auto __allowedType : __allowedTypes) {                                                                \
                if (!__first) __allowedTypesStr << ", ";                                                               \
                __allowedTypesStr << CommandOriginTypeToString(__allowedType);                                         \
                __first = false;                                                                                       \
            }                                                                                                          \
            __output.error("This command is available to '{}' only!"_tr(__allowedTypesStr.str()));                     \
            return;                                                                                                    \
        }                                                                                                              \
    }

struct Arg_Player {
    CommandSelector<Player> player;
};
struct Args_Kick {
    CommandSelector<Player> player;
    string                  message;
};
struct Arg_Message {
    string message;
};

bool regCommand() {
    using ll::command::CommandRegistrar;
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.commandName,
        config::cfg.command.commandDescription
    );

    // tools
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        Actor* entity = origin.getEntity();
        if (entity) {
            auto& player = *static_cast<Player*>(entity); // entity* => Player&
            if (player.isOperator()) {
                tls::form::index(player);
            } else {
                output.error("This command is available to [OP] only!"_tr());
            }
        }
    }>();

    // tools kill <Player>
    cmd.overload<Arg_Player>()
        .text("kill")
        .required("player")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Player const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            Actor* entity = origin.getEntity();
            if (entity) {
                auto& player = *static_cast<Player*>(entity);
                if (player.isOperator()) {
                    auto item = param.player.results(origin).data;
                    for (Player* target : *item) {
                        if (target) {
                            target->kill();
                            player.sendMessage("try kill player: {}"_tr(target->getRealName()));
                        }
                    }
                } else {
                    output.error("This command is available to [OP] only!"_tr());
                }
            }
        }>();

    // tools kick <Player> [Msg]
    cmd.overload<Args_Kick>()
        .text("kick")
        .required("player")
        .optional("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Args_Kick const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            Actor* entity = origin.getEntity();
            if (entity) {
                auto& player = *static_cast<Player*>(entity);
                if (player.isOperator()) {
                    auto item = param.player.results(origin).data;
                    for (Player* target : *item) {
                        if (target) {
                            target->disconnect(param.message.empty() ? "server disconnect" : param.message);
                            player.sendMessage("try kick player: {}"_tr(target->getRealName()));
                        }
                    }
                } else {
                    output.error("This command is available to [OP] only!"_tr());
                }
            }
        }>();

    // tools crash <Player>
    cmd.overload<Arg_Player>()
        .text("crash")
        .required("player")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Player const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            Actor* entity = origin.getEntity();
            if (entity) {
                auto& player = *static_cast<Player*>(entity);
                if (player.isOperator()) {
                    auto item = param.player.results(origin).data;
                    for (Player* target : *item) {
                        if (target) {
                            string           name = target->getRealName();
                            LevelChunkPacket pkt  = LevelChunkPacket();
                            pkt.mCacheEnabled     = true;
                            target->sendNetworkPacket(pkt);
                            player.sendMessage("try crash player: {}"_tr(name));
                        }
                    }
                } else {
                    output.error("This command is available to [OP] only!"_tr());
                }
            }
        }>();

    // tools talkas <Player> <msg>
    cmd.overload<Args_Kick>()
        .text("talkas")
        .required("player")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Args_Kick const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            Actor* entity = origin.getEntity();
            if (entity) {
                auto& player = *static_cast<Player*>(entity);
                if (player.isOperator()) {
                    auto item = param.player.results(origin).data;
                    for (Player* target : *item) {
                        if (target) {
                            TextPacket pkt = TextPacket::createChat(
                                target->getName(),
                                param.message.empty() ? "" : param.message,
                                target->getXuid(),
                                ""
                            );
                            if (ll::service::getLevel().has_value()) {
                                ll::service::getLevel()->forEachPlayer([&pkt](Player& player) {
                                    player.sendNetworkPacket(pkt); // send to all player
                                    return true;
                                });
                            }
                            player.sendMessage("try talkas player: {}"_tr(target->getRealName()));
                        }
                    }
                } else {
                    output.error("This command is available to [OP] only!"_tr());
                }
            }
        }>();

    // tools broadcast <msg>
    cmd.overload<Arg_Message>()
        .text("broadcast")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Message const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            Actor* entity = origin.getEntity();
            if (entity) {
                auto& player = *static_cast<Player*>(entity);
                if (player.isOperator()) {
                    TextPacket pkt =
                        TextPacket::createChat("Server", param.message.empty() ? "" : param.message, "", "");
                    if (ll::service::getLevel().has_value()) {
                        ll::service::getLevel()->forEachPlayer([&pkt](Player& player) {
                            player.sendNetworkPacket(pkt); // send to all player
                            return true;
                        });
                    }
                    player.sendMessage("try broadcast message: {}"_tr(param.message));
                } else {
                    output.error("This command is available to [OP] only!"_tr());
                }
            }
        }>();

    return true;
}

} // namespace tls::command
