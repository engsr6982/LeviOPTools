#include "Chunk/ChunkManager.h"
#include "Command.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/common/bedrock/typeid_t.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/server/commands/CommandPosition.h"
#include "mc/server/commands/CommandPositionFloat.h"
#include "mc/server/commands/CommandVersion.h"

namespace tls::command {

using string = std::string;
using ll::i18n_literals::operator""_tr;

struct ArgPosAndDimid {
    CommandPositionFloat pos;
    int                  dimentionId;
};

void registerChunkCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.tools.commandName,
        config::cfg.command.tools.commandDescription
    );

    // tools chunk debug
    cmd.overload().text("chunk").text("debug").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        try {
            auto* player = static_cast<Player*>(origin.getEntity());
            if (!player) {
                output.error("Failed to get player entity!"_tr());
                return;
            }
            auto playerVec3 = player->getPosition();

            auto levelChunk = chunk::ChunkManager::getChunkAt(playerVec3, player->getDimension());

            output.success("[Chunk] Current Chunk info:"_tr());
            output.success("[Chunk] ================================="_tr());
            output.success("[Chunk] DimensionId : {}"_tr(levelChunk->getDimension().getDimensionId().id));
            output.success("[Chunk] ChunkPos    : {}"_tr(levelChunk->getPosition().toString()));
            output.success("[Chunk] MinPos      : {}"_tr(levelChunk->getMin().toString()));
            output.success("[Chunk] MaxPos      : {}"_tr(levelChunk->getMax().toString()));
            output.success("[Chunk] MinY        : {}"_tr(levelChunk->getMinY()));
            output.success("[Chunk] MaxY        : {}"_tr(levelChunk->getMaxY()));
            output.success("[Chunk] ================================="_tr());
            output.success("[Chunk] You Current Position: {}"_tr(playerVec3.toString()));
        } catch (...) {
            output.error("unknown error!"_tr());
        }
    }>();

    // tools chunk backup [Vec3] <dimentionId>
    cmd.overload<ArgPosAndDimid>()
        .text("chunk")
        .text("backup")
        .optional("pos")
        .required("dimentionId")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ArgPosAndDimid const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );

            Vec3 pos;
            pos = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos);
            // get position
            if (origin.getOriginType() == CommandOriginType::Player && pos.x == 0 && pos.y == 0 && pos.z == 0) {
                auto* player = static_cast<Player*>(origin.getEntity());
                if (!player) {
                    output.error("Failed to get player entity!"_tr());
                    return;
                }
                pos = player->getPosition();
            } else if (origin.getOriginType() == CommandOriginType::DedicatedServer) {
                if (pos.x == 0 && pos.y == 0 && pos.z == 0) {
                    output.error("You must specify a position!"_tr());
                    return;
                }
            }
            // process dimension id
            auto dimension = ll::service::getLevel()->getDimension(param.dimentionId);
            if (!dimension) {
                output.error("Invalid dimension id!"_tr());
                return;
            }
            auto levelChunk = chunk::ChunkManager::getChunkAt(pos, *dimension);
            if (!levelChunk) {
                output.error("Failed to get chunk at position!"_tr());
                return;
            }
            bool isSuccess = chunk::ChunkManager::getInstance().saveChunk(levelChunk);
            if (isSuccess) {
                output.success("Chunk backup success!"_tr());
            } else {
                output.error("Failed to backup chunk!"_tr());
            }
        }>();

    // tools chunk restore [Vec3] <dimentionId>
    cmd.overload<ArgPosAndDimid>()
        .text("chunk")
        .text("restore")
        .optional("pos")
        .required("dimentionId")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ArgPosAndDimid const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );

            Vec3 pos;
            pos = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos);
            // get position
            if (origin.getOriginType() == CommandOriginType::Player && pos.x == 0 && pos.y == 0 && pos.z == 0) {
                auto* player = static_cast<Player*>(origin.getEntity());
                if (!player) {
                    output.error("Failed to get player entity!"_tr());
                    return;
                }
                pos = player->getPosition();
            } else if (origin.getOriginType() == CommandOriginType::DedicatedServer) {
                if (pos.x == 0 && pos.y == 0 && pos.z == 0) {
                    output.error("You must specify a position!"_tr());
                    return;
                }
            }
            // process dimension id
            auto dimension = ll::service::getLevel()->getDimension(param.dimentionId);
            if (!dimension) {
                output.error("Invalid dimension id!"_tr());
                return;
            }
            auto levelChunk = chunk::ChunkManager::getChunkAt(pos, *dimension);
            if (!levelChunk) {
                output.error("Failed to get chunk at position!"_tr());
                return;
            }
            bool isSuccess = chunk::ChunkManager::getInstance().loadChunk(levelChunk);
            if (isSuccess) {
                output.success("Chunk restore success!"_tr());
            } else {
                output.error("Failed to restore chunk!"_tr());
            }
        }>();
}

} // namespace tls::command
