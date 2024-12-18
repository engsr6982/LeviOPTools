#pragma once
#include "Config/Config.h"
#include "Form/index.h"
#include "Permission/Permission.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/common/wrapper/optional_ref.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/SetTimePacket.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include <Entry/PluginInfo.h>
#include <PermissionCore/Group.h>
#include <PermissionCore/PermissionCore.h>
#include <PermissionCore/PermissionManager.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <ll/api/Logger.h>
#include <ll/api/Utils/HashUtils.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/service/PlayerInfo.h>
#include <ll/api/service/Service.h>
#include <map>
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
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>


namespace tls::command {

using string = std::string;
using ll::i18n_literals::operator""_tr;
using ll::command::CommandRegistrar;

void registerCommand();
void registerChunkCommand();
void registerGamemodeCommand();
void registerTeleportCommand();

// ------------------------------ structs ----------------------------------

struct Arg_Player {
    CommandSelector<Player> player;
};
struct Args_Player_Msg {
    CommandSelector<Player> player;
    string                  message;
};
struct Arg_Msg {
    string message;
};
struct SetServerMaxPlayers {
    int maxPlayers;
};

enum class GameTypeStringBriefEnum : int {
    s = static_cast<int>(GameType::Survival),
    c = static_cast<int>(GameType::Creative),
    a = static_cast<int>(GameType::Adventure),
    d = static_cast<int>(GameType::Default)
};

struct GameMode_String_Brief {
    GameTypeStringBriefEnum gameType;
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

// ------------------------------ tools ----------------------------------

inline bool checkPlayerPermission(CommandOrigin const& origin, CommandOutput& output, int const& permission) {
    if (origin.getOriginType() == CommandOriginType::DedicatedServer) return true;
    Actor* entity = origin.getEntity();
    if (entity) {
        auto& player = *static_cast<Player*>(entity);
        return pmc::PermissionManager::getInstance()
            .getPermissionCore(PLUGIN_NAME)
            ->checkUserPermission(player.getUuid().asString().c_str(), permission);
    } else {
        output.error("get entity failed!"_tr());
        return false;
    };
}

inline string CommandOriginTypeToString(CommandOriginType type) { return string(magic_enum::enum_name(type)); }

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

} // namespace tls::command