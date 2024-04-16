#include "Command.h"
#include "File/Config.h"
#include "Form/index.h"
#include "Permission/Permission.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/SetTimePacket.h"
#include <Entry/PluginInfo.h>
#include <PermissionCore/Group.h>
#include <PermissionCore/PermissionCore.h>
#include <PermissionCore/PermissionManager.h>
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
using ll::command::CommandRegistrar;

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

#define CHECK_COMMAND_TYPE(__output, __originType, ...)                                                                \
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

bool checkCallbackPermission(CommandOrigin const& origin, CommandOutput& output, int const& permission) {
    if (origin.getOriginType() == CommandOriginType::DedicatedServer) return true;
    Actor* entity = origin.getEntity();
    if (entity) {
        auto& player = *static_cast<Player*>(entity);
        return perm::PermissionManager::getInstance()
            .getPermissionCore(PLUGIN_NAME)
            ->checkUserPermission(player.getUuid().asString().c_str(), permission);
    } else {
        output.error("get entity failed!"_tr());
        return false;
    };
}

// ----------------------------------------------------------------------

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
struct Arg_SetMaxPlayers {
    int maxPlayers;
};

void regCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.tools.commandName,
        config::cfg.command.tools.commandDescription
    );

    // tools
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        if (!checkCallbackPermission(origin, output, tls::perms::indexForm)) {
            output.error("You don't have permission to use this command!"_tr());
            return;
        }
        Actor* entity = origin.getEntity();
        auto&  player = *static_cast<Player*>(entity); // entity* => Player&
        tls::form::index(player);
    }>();

    // tools reload
    cmd.overload().text("reload").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::DedicatedServer);
        tls::config::loadConfig();
        output.success("Config reloaded!"_tr());
    }>();

    // tools kill <Player>
    cmd.overload<Arg_Player>()
        .text("kill")
        .required("player")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Player const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkCallbackPermission(origin, output, tls::perms::KillPlayer)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            auto item = param.player.results(origin).data;
            for (Player* target : *item) {
                if (target) {
                    target->kill();
                    player.sendMessage("try kill player: {}"_tr(target->getRealName()));
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
            if (!checkCallbackPermission(origin, output, tls::perms::KickPlayer)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            auto item = param.player.results(origin).data;
            for (Player* target : *item) {
                if (target) {
                    target->disconnect(param.message.empty() ? "server disconnect" : param.message);
                    player.sendMessage("try kick player: {}"_tr(target->getRealName()));
                }
            }
        }>();

    // tools crash <Player>
    cmd.overload<Arg_Player>()
        .text("crash")
        .required("player")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Player const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkCallbackPermission(origin, output, tls::perms::CrashPlayerClient)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
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
        }>();

    // tools talkas <Player> <msg>
    cmd.overload<Args_Kick>()
        .text("talkas")
        .required("player")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Args_Kick const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkCallbackPermission(origin, output, tls::perms::UsePlayerIdentitySay)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
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
        }>();

    // tools broadcast <msg>
    cmd.overload<Arg_Message>()
        .text("broadcast")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Message const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkCallbackPermission(origin, output, tls::perms::BroadCastMessage)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            TextPacket pkt = TextPacket::createChat("Server", param.message.empty() ? "" : param.message, "", "");
            if (ll::service::getLevel().has_value()) {
                ll::service::getLevel()->forEachPlayer([&pkt](Player& player) {
                    player.sendNetworkPacket(pkt); // send to all player
                    return true;
                });
            }
            player.sendMessage("try broadcast message: {}"_tr(param.message));
        }>();

    // tools setmaxplayers <int>
    cmd.overload<Arg_SetMaxPlayers>()
        .text("setmaxplayers")
        .required("maxPlayers")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_SetMaxPlayers const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkCallbackPermission(origin, output, tls::perms::SetServerMaxPlayer)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            // processing
            int back = ll::service::getServerNetworkHandler()->setMaxNumPlayers(param.maxPlayers);
            ll::service::getServerNetworkHandler()->updateServerAnnouncement();
            output.success("Max players set to {}, previous value is {}"_tr(param.maxPlayers, back));
        }>();

    tls::command::registerGamemodeCommand();
}

// ------------------------------ gamemode command ---------------------------------

#include <mc/enums/GameType.h>

enum class GameTypeStringBrief : int {
    s = static_cast<int>(GameType::Survival),
    c = static_cast<int>(GameType::Creative),
    a = static_cast<int>(GameType::Adventure),
    d = static_cast<int>(GameType::Default)
};

struct GameMode_String_Brief {
    GameTypeStringBrief     gameType;
    CommandSelector<Player> player;
};

struct GameMode_String_Full {
    GameType                gameType;
    CommandSelector<Player> player;
};

struct GameMode_Int {
    int                     gameType;
    CommandSelector<Player> player;
};

#define Gm_String_Full_CallBack                                                                                        \
    [&](CommandOrigin const& origin, CommandOutput& output, GameMode_String_Full const& param) {                       \
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);                                 \
        if (!checkCallbackPermission(origin, output, tls::perms::ChangeGameMode)) {                                    \
            output.error("You don't have permission to use this command!"_tr());                                       \
            return;                                                                                                    \
        }                                                                                                              \
        if (param.player.results(origin).empty()) {                                                                    \
            auto& player = *static_cast<Player*>(origin.getEntity());                                                  \
            player.setPlayerGameType(param.gameType);                                                                  \
            output.success("try set game mode: {}"_tr(param.gameType));                                                \
        } else {                                                                                                       \
            auto player = param.player.results(origin).data;                                                           \
            for (Player * target : *player) {                                                                          \
                if (target) {                                                                                          \
                    target->setPlayerGameType(param.gameType);                                                         \
                    output.success("try set game mode: {} to player: {}"_tr(param.gameType, target->getRealName()));   \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }

#define Gm_String_Brief_CallBack                                                                                       \
    [&](CommandOrigin const& origin, CommandOutput& output, GameMode_String_Brief const& param) {                      \
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);                                 \
        if (!checkCallbackPermission(origin, output, tls::perms::ChangeGameMode)) {                                    \
            output.error("You don't have permission to use this command!"_tr());                                       \
            return;                                                                                                    \
        }                                                                                                              \
        if (param.player.results(origin).empty()) {                                                                    \
            auto& player = *static_cast<Player*>(origin.getEntity());                                                  \
            player.setPlayerGameType((GameType)param.gameType);                                                        \
            output.success("try set game mode: {}"_tr(param.gameType));                                                \
        } else {                                                                                                       \
            auto player = param.player.results(origin).data;                                                           \
            for (Player * target : *player) {                                                                          \
                if (target) {                                                                                          \
                    target->setPlayerGameType((GameType)param.gameType);                                               \
                    output.success("try set game mode: {} to player: {}"_tr(param.gameType, target->getRealName()));   \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }

#define Gm_Int_CallBack                                                                                                \
    [&](CommandOrigin const& origin, CommandOutput& output, GameMode_Int const& param) {                               \
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);                                 \
        if (!checkCallbackPermission(origin, output, tls::perms::ChangeGameMode)) {                                    \
            output.error("You don't have permission to use this command!"_tr());                                       \
            return;                                                                                                    \
        }                                                                                                              \
        if (param.gameType == -1 || param.gameType == 0 || param.gameType == 1 || param.gameType == 2                  \
            || param.gameType == 5 || param.gameType == 6) {                                                           \
            if (param.player.results(origin).empty()) {                                                                \
                auto& player = *static_cast<Player*>(origin.getEntity());                                              \
                player.setPlayerGameType((GameType)param.gameType);                                                    \
                output.success("try set game mode: {}"_tr(param.gameType));                                            \
            } else {                                                                                                   \
                auto player = param.player.results(origin).data;                                                       \
                for (Player * target : *player) {                                                                      \
                    if (target) {                                                                                      \
                        target->setPlayerGameType((GameType)param.gameType);                                           \
                        output.success("try set game mode: {} to player: {}"_tr(param.gameType, target->getRealName()) \
                        );                                                                                             \
                    }                                                                                                  \
                }                                                                                                      \
            }                                                                                                          \
        } else {                                                                                                       \
            output.error("Invalid game mode!"_tr());                                                                   \
        }                                                                                                              \
    }

void registerGamemodeCommand() {
    // ------------------------------ tools ---------------------------------

    auto& tools = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.tools.commandName,
        config::cfg.command.tools.commandDescription
    );
    // tools gm <int> [player]
    tools.overload<GameMode_Int>().text("gm").required("gameType").optional("player").execute<Gm_Int_CallBack>();

    // tools gm <s|c|a|d> [player]
    tools.overload<GameMode_String_Brief>()
        .text("gm")
        .required("gameType")
        .optional("player")
        .execute<Gm_String_Brief_CallBack>();

    // tools gm <GameType> [player]
    tools.overload<GameMode_String_Full>()
        .text("gm")
        .required("gameType")
        .optional("player")
        .execute<Gm_String_Full_CallBack>();

    // ------------------------------ gm ---------------------------------

    if (config::cfg.command.gm.enable) {
        auto& gm = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
            config::cfg.command.gm.commandName,
            config::cfg.command.gm.commandDescription
        );
        // gm <int> [player]
        tools.overload<GameMode_Int>().required("gameType").optional("player").execute<Gm_Int_CallBack>();

        // gm <s|c|a|d> [player]
        gm.overload<GameMode_String_Brief>()
            .required("gameType")
            .optional("player")
            .execute<Gm_String_Brief_CallBack>();

        // gm <GameType> [player]
        gm.overload<GameMode_String_Full>().required("gameType").optional("player").execute<Gm_String_Full_CallBack>();
    }
}

} // namespace tls::command
