#include "Command.h"
#include "File/Config.h"
#include "Form/Form.h"
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
        return output.error("This command is available to [{}] only!"_tr(targetTypeStr));                              \
    }
// clang-format on

using string = std::string;

struct Arg_Player {
    CommandSelector<Player> player;
};
struct Args_Kick {
    CommandSelector<Player> player;
    string                  message;
};

bool regCommand() {
    using ll::command::CommandRegistrar;
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
            CHECK_COMMAND_TYPE(origin.getOriginType(), CommandOriginType::Player);
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
            CHECK_COMMAND_TYPE(origin.getOriginType(), CommandOriginType::Player);
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
            CHECK_COMMAND_TYPE(origin.getOriginType(), CommandOriginType::Player);
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
            CHECK_COMMAND_TYPE(origin.getOriginType(), CommandOriginType::Player);
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


    return true;
}

} // namespace tls::command
