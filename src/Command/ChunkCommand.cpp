#include "Chunk/BindData.h"
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

struct ArgSelectPos {
    CommandPositionFloat pos1;
    CommandPositionFloat pos2;
    int                  dimentionId;
};

struct ArgSelectBackup {
    int    id;
    string fileName;
};

struct ArgFileName {
    string fileName;
};

struct ArgIdAndVec3 {
    int                  id;
    CommandPositionFloat pos;
};

struct ArgCopyConfirm {
    int  id;
    int  dimentionId    = -1;
    bool ignoreEntities = false;
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
                output.error("[Chunk] Failed to get player entity!"_tr());
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
            output.error("[Chunk] unknown error!"_tr());
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
                    output.error("[Chunk] Failed to get player entity!"_tr());
                    return;
                }
                pos = player->getPosition();
            } else if (origin.getOriginType() == CommandOriginType::DedicatedServer) {
                if (pos.x == 0 && pos.y == 0 && pos.z == 0) {
                    output.error("[Chunk] You must specify a position!"_tr());
                    return;
                }
            }
            // process dimension id
            auto dimension = ll::service::getLevel()->getDimension(param.dimentionId);
            if (!dimension) {
                output.error("[Chunk] Invalid dimension id!"_tr());
                return;
            }
            auto levelChunk = chunk::ChunkManager::getChunkAt(pos, *dimension);
            if (!levelChunk) {
                output.error("[Chunk] Failed to get chunk at position!"_tr());
                return;
            }
            bool isSuccess = chunk::ChunkManager::getInstance().saveChunk(levelChunk);
            if (isSuccess) {
                output.success("[Chunk] Chunk backup success!"_tr());
            } else {
                output.error("[Chunk] Failed to backup chunk!"_tr());
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
                    output.error("[Chunk] Failed to get player entity!"_tr());
                    return;
                }
                pos = player->getPosition();
            } else if (origin.getOriginType() == CommandOriginType::DedicatedServer) {
                if (pos.x == 0 && pos.y == 0 && pos.z == 0) {
                    output.error("[Chunk] You must specify a position!"_tr());
                    return;
                }
            }
            // process dimension id
            auto dimension = ll::service::getLevel()->getDimension(param.dimentionId);
            if (!dimension) {
                output.error("[Chunk] Invalid dimension id!"_tr());
                return;
            }
            auto levelChunk = chunk::ChunkManager::getChunkAt(pos, *dimension);
            if (!levelChunk) {
                output.error("[Chunk] Failed to get chunk at position!"_tr());
                return;
            }
            bool isSuccess = chunk::ChunkManager::getInstance().loadChunk(levelChunk);
            if (isSuccess) {
                output.success("[Chunk] Chunk restore success!"_tr());
            } else {
                output.error("[Chunk] Failed to restore chunk!"_tr());
            }
        }>();


    // 选区相关命令

    // tools chunk select <pos1> <pos2> <dimentionId>
    cmd.overload<ArgSelectPos>()
        .text("chunk")
        .text("select")
        .required("pos1")
        .required("pos2")
        .required("dimentionId")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ArgSelectPos const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            Vec3        pos1 = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos1);
            Vec3        pos2 = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos2);
            BoundingBox box(pos1, pos2);
            chunk::ChunkManager::checkAndFixLittelEndianCooridnates(box);

            int id = chunk::BindData::getInstance().createBindData(box, param.dimentionId);
            if (id == -1) {
                output.error("[Chunk] Failed to create bind data!"_tr());
                return;
            }
            output.success("[Chunk] Your id for this operation {}"_tr(id));
        }>();

    // tools chunk select backup <id> <fileName>
    cmd.overload<ArgSelectBackup>()
        .text("chunk")
        .text("select")
        .text("backup")
        .required("id")
        .required("fileName")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ArgSelectBackup const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            if (chunk::BindData::getInstance().hasBindData(param.id)) {
                auto& bindData  = chunk::BindData::getInstance().getBindData(param.id);
                bool  isSuccess = chunk::ChunkManager::getInstance()
                                     .saveCustomData(param.fileName, bindData.box, bindData.dimentionId);
                if (isSuccess) {
                    chunk::BindData::getInstance().removeBindData(param.id);
                    output.success("[Chunk] Backup successful, operation id destroyed!"_tr());
                } else {
                    output.error("[Chunk] Backup failed, please check and retry!"_tr());
                }
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        }>();

    // tools chunk select restore <id> <fileName>
    cmd.overload<ArgFileName>()
        .text("chunk")
        .text("select")
        .text("restore")
        .required("fileName")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ArgFileName const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            bool isSuccess = chunk::ChunkManager::getInstance().loadCustomData(param.fileName);
            if (isSuccess) {
                output.success("[Chunk] Recovery successful!"_tr());
            } else {
                output.error("[Chunk] Restore failed, please check and retry!"_tr());
            }
        }>();

    // tools chunk select copy <id> <vec3>
    cmd.overload<ArgIdAndVec3>()
        .text("chunk")
        .text("select")
        .text("copy")
        .required("id")
        .required("pos")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ArgIdAndVec3 const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            Vec3 pos = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos);
            if (chunk::BindData::getInstance().hasBindData(param.id)) {
                auto& bindData         = chunk::BindData::getInstance().getBindData(param.id);
                bindData.isOpenCopy    = true;
                bindData.copyTargetPos = pos;
                output.success(
                    "[Chunk] Copy mode has been turned on.You can enter commands to mirror and rotate, then enter a command to confirm placement, or enter a command to cancel."_tr(
                    )
                );
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        }>();

    // tools chunk select copy confirm <id> [ignoreEntities] [dimentionId]
    cmd.overload<ArgCopyConfirm>()
        .text("chunk")
        .text("select")
        .text("copy")
        .text("confirm")
        .required("id")
        .optional("ignoreEntities")
        .optional("dimentionId")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ArgCopyConfirm const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            if (chunk::BindData::getInstance().hasBindData(param.id)) {
                auto& bindData = chunk::BindData::getInstance().getBindData(param.id);
                if (bindData.isOpenCopy) {
                    auto structure = chunk::ChunkManager::getStructureAt(bindData.box, bindData.dimentionId);
                    chunk::ChunkManager::placeStructure(
                        param.dimentionId == -1 ? bindData.dimentionId : param.dimentionId,
                        std::move(structure),
                        bindData.copyTargetPos,
                        bindData.mirror,
                        bindData.rotation,
                        false,
                        param.ignoreEntities
                    );
                    chunk::BindData::getInstance().removeBindData(param.id);
                    output.success("[Chunk] Copy successful, operation id destroyed!"_tr());
                } else {
                    output.error("[Chunk] Copy mode is not turned on, please enter a command to turn on copy mode."_tr()
                    );
                }
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        }>();
}

} // namespace tls::command
