// my
#include "Command.h"
#include "File/Config.h"
#include "Form/index.h"
#include <Entry/PluginInfo.h>

// ll
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

// mc
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/SetTimePacket.h"
#include <mc/entity/utilities/ActorType.h>
#include <mc/enums/GameType.h>
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


// library
#include "Permission/Permission.h"
#include <PermissionCore/Group.h>
#include <PermissionCore/PermissionCore.h>
#include <PermissionCore/PermissionManager.h>

// stl
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// test
#include "ll/api/service/Bedrock.h"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"


namespace tls::command {

using ll::i18n_literals::operator""_tr;
using string = std::string;
using ll::command::CommandRegistrar;


bool checkPlayerPermission(CommandOrigin const& origin, CommandOutput& output, int const& permission) {
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

void registerDebugCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.tools.commandName,
        config::cfg.command.tools.commandDescription
    );

    cmd.overload().text("chunkpos").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        try {
            auto* player = static_cast<Player*>(origin.getEntity());
            if (!player) {
                output.error("get entity failed!"_tr());
                return;
            }
            auto playerVec3 = player->getPosition();

            Dimension const& dimid       = player->getDimension();
            BlockSource&     blockSource = dimid.getBlockSourceFromMainChunkSource();
            BlockPos         blockPos{playerVec3.x, playerVec3.y, playerVec3.z};
            LevelChunk*      levelChunk = blockSource.getChunkAt(blockPos);
            ChunkPos const&  chunkPos   = levelChunk->getPosition();

            std::cout << "ChunkPos::toString = " << chunkPos.toString() << std::endl;
            std::cout << "ChunkPos::size = " << chunkPos.size() << std::endl;
            std::cout << "ChunkPos::length = " << chunkPos.length() << std::endl;
            std::cout << "ChunkPos::lengthSqr = " << chunkPos.lengthSqr() << std::endl;

            std::cout << "ChunkPos::b = " << chunkPos.b << std::endl;
            std::cout << "ChunkPos::g = " << chunkPos.g << std::endl;
            std::cout << "ChunkPos::p = " << chunkPos.p << std::endl;
            std::cout << "ChunkPos::r = " << chunkPos.r << std::endl;
            std::cout << "ChunkPos::s = " << chunkPos.s << std::endl;
            std::cout << "ChunkPos::t = " << chunkPos.t << std::endl;
            std::cout << "ChunkPos::x = " << chunkPos.x << std::endl;
            std::cout << "ChunkPos::y = " << chunkPos.y << std::endl;
            std::cout << "ChunkPos::z = " << chunkPos.z << std::endl;

            std::cout << "LevelChunk::getMax = " << levelChunk->getMax().toString() << std::endl;
            std::cout << "LevelChunk::getMin = " << levelChunk->getMin().toString() << std::endl;
            std::cout << "LevelChunk::getMaxY = " << levelChunk->getMaxY() << std::endl;
            std::cout << "LevelChunk::getMinY = " << levelChunk->getMinY() << std::endl;

            // ChunkBlockPos cbp = ChunkBlockPos(pos, dimid.getMinHeight());
            // Block const&  bl  = levelChunk->getBlock(cbp);

        } catch (...) {
            output.error("unknown error!"_tr());
        }
    }>();
}

void registerGamemodeCommand() {
#define Gm_String_Full_CallBack                                                                                        \
    [&](CommandOrigin const& origin, CommandOutput& output, GameMode_String_Full const& param) {                       \
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);                                 \
        if (!checkPlayerPermission(origin, output, tls::perms::ChangeGameMode)) {                                      \
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
        if (!checkPlayerPermission(origin, output, tls::perms::ChangeGameMode)) {                                      \
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
        if (!checkPlayerPermission(origin, output, tls::perms::ChangeGameMode)) {                                      \
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

void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.tools.commandName,
        config::cfg.command.tools.commandDescription
    );

#ifdef DEBUG
    registerDebugCommand();
#endif

    registerGamemodeCommand();

    // tools
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        if (!checkPlayerPermission(origin, output, tls::perms::indexForm)) {
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
            if (!checkPlayerPermission(origin, output, tls::perms::KillPlayer)) {
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
    cmd.overload<Args_Player_Msg>()
        .text("kick")
        .required("player")
        .optional("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Args_Player_Msg const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::KickPlayer)) {
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
            if (!checkPlayerPermission(origin, output, tls::perms::CrashPlayerClient)) {
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
    cmd.overload<Args_Player_Msg>()
        .text("talkas")
        .required("player")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Args_Player_Msg const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::UsePlayerIdentitySay)) {
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
    cmd.overload<Arg_Msg>()
        .text("broadcast")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Msg const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::BroadCastMessage)) {
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
    cmd.overload<SetServerMaxPlayers>()
        .text("setmaxplayers")
        .required("maxPlayers")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, SetServerMaxPlayers const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::SetServerMaxPlayer)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            // processing
            int back = ll::service::getServerNetworkHandler()->setMaxNumPlayers(param.maxPlayers);
            ll::service::getServerNetworkHandler()->updateServerAnnouncement();
            output.success("Max players set to {}, previous value is {}"_tr(param.maxPlayers, back));
        }>();
}

} // namespace tls::command
