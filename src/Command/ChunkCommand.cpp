#include "Chunk/BindData.h"
#include "Chunk/ChunkManager.h"
#include "Command.h"
#include "Entry/PluginInfo.h"
#include "Permission/Permission.h"
#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include "Utils/Utils.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/common/bedrock/typeid_t.h"
#include "mc/enums/Mirror.h"
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

struct ArgChunkSelsctCopy {
    int                  id;
    CommandPositionFloat pos;
};

struct ArgCopyConfirm {
    int  id;
    int  dimentionId    = -1;
    bool ignoreEntities = false;
};

struct SaveStructure {
    int    id;
    string fileName;
    bool   ignoreEntities = false;
    bool   ignoreBlocks   = false;
};

struct PlaceStructure {
    int                  id;
    CommandPositionFloat pos;
    bool                 ignoreEntities = false;
    int                  dimentionId    = -1;
};

struct TransFormMirror {
    int    id;
    Mirror direction;
};

struct TransFormRotate {
    int      id;
    Rotation angle;
};

struct OnlyID {
    int id = -1;
};


string TransFormArgToString(Mirror direction) {
    switch (direction) {
    case Mirror::X:
        return "X";
    case Mirror::Z:
        return "Z";
    case Mirror::XZ:
        return "XZ";
    case Mirror::None:
        return "None";
    default:
        return "Unknown";
    }
}
string TransFormArgToString(Rotation angle) {
    switch (angle) {
    case Rotation::None:
        return "None";
    case Rotation::Rotate90:
        return "90";
    case Rotation::Rotate180:
        return "180";
    case Rotation::Rotate270:
        return "270";
    default:
        return "Unknown";
    }
}

#define CHECK_ChunkOperation_Permission(output, origin)                                                                \
    {                                                                                                                  \
        if (origin.getOriginType() == CommandOriginType::Player) {                                                     \
            auto* player = static_cast<Player*>(origin.getEntity());                                                   \
            if (!player) return output.error("[Chunk] Failed to get player entity!"_tr());                             \
            auto core = pmc::PermissionManager::getInstance().getPermissionCore(PLUGIN_NAME);                          \
            if (core) {                                                                                                \
                if (core->checkUserPermission(player->getUuid().asString().c_str(), perms::ChunkOpertion) == false)    \
                    return output.error("[Chunk] You don't have permission to use this command!"_tr());                \
            } else return output.error("[Chunk] Failed to get permission core!"_tr());                                 \
        }                                                                                                              \
    }

void registerChunkCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        Config::cfg.command.tools.commandName,
        Config::cfg.command.tools.commandDescription
    );

    // tools chunk debug
    cmd.overload().text("chunk").text("debug").execute([&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        CHECK_ChunkOperation_Permission(output, origin);
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
    });

    // tools chunk backup [Vec3] <dimentionId>
    cmd.overload<ArgPosAndDimid>()
        .text("chunk")
        .text("backup")
        .optional("pos")
        .required("dimentionId")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ArgPosAndDimid const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

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
        });

    // tools chunk restore [Vec3] <dimentionId>
    cmd.overload<ArgPosAndDimid>()
        .text("chunk")
        .text("restore")
        .optional("pos")
        .required("dimentionId")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ArgPosAndDimid const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

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
        });


    // 选区相关命令

    // tools chunk select <pos1> <pos2> <dimentionId>
    cmd.overload<ArgSelectPos>()
        .text("chunk")
        .text("select")
        .required("pos1")
        .required("pos2")
        .required("dimentionId")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ArgSelectPos const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            Vec3        pos1 = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos1);
            Vec3        pos2 = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos2);
            BoundingBox box(pos1, pos2);
            chunk::ChunkManager::checkAndFixLittelEndianCooridnates(box);

            int id = chunk::BindData::getInstance().createBindData(box, param.dimentionId, true);
            if (id == -1) {
                output.error("[Chunk] Failed to create bind data!"_tr());
                return;
            }
            output.success("[Chunk] Your id for this operation {}"_tr(id));
        });

    // tools chunk select backup <id> <fileName>
    cmd.overload<ArgSelectBackup>()
        .text("chunk")
        .text("select")
        .text("backup")
        .required("id")
        .required("fileName")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ArgSelectBackup const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

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
        });

    // tools chunk select restore <id> <fileName>
    cmd.overload<ArgFileName>()
        .text("chunk")
        .text("select")
        .text("restore")
        .required("fileName")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ArgFileName const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            bool isSuccess = chunk::ChunkManager::getInstance().loadCustomData(param.fileName);
            if (isSuccess) {
                output.success("[Chunk] Recovery successful!"_tr());
            } else {
                output.error("[Chunk] Restore failed, please check and retry!"_tr());
            }
        });

    // tools chunk select copy <id> <vec3>
    cmd.overload<ArgChunkSelsctCopy>().text("chunk").text("select").text("copy").required("id").required("pos").execute(
        [&](CommandOrigin const& origin, CommandOutput& output, ArgChunkSelsctCopy const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            Vec3 pos = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos);
            if (chunk::BindData::getInstance().hasBindData(param.id)) {
                auto& bindData            = chunk::BindData::getInstance().getBindData(param.id);
                bindData.canCopyStructure = true;
                bindData.copyTargetPos    = pos;
                output.success(
                    "[Chunk] Copy mode has been turned on.You can enter commands to mirror and rotate, then enter a command to confirm placement, or enter a command to cancel."_tr(
                    )
                );
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        }
    );

    // tools chunk select copy confirm <id> [ignoreEntities] [dimentionId]
    cmd.overload<ArgCopyConfirm>()
        .text("chunk")
        .text("select")
        .text("copy")
        .text("confirm")
        .required("id")
        .optional("ignoreEntities")
        .optional("dimentionId")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ArgCopyConfirm const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            if (chunk::BindData::getInstance().hasBindData(param.id)) {
                auto& bindData = chunk::BindData::getInstance().getBindData(param.id);
                if (bindData.canCopyStructure) {
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
        });


    // McStructure

    // tools chunk structure load <fileName>
    cmd.overload<ArgFileName>()
        .text("chunk")
        .text("structure")
        .text("load")
        .required("fileName")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ArgFileName const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            auto structure = chunk::ChunkManager::getInstance().loadStructure(param.fileName);
            if (structure) {
                int id = chunk::BindData::getInstance().createBindData(std::move(structure), true);
                if (id == -1) {
                    output.error("[Chunk] Failed to create binding data!"_tr());
                    return;
                }
                output.success("[Chunk] Load file {} successfully, operation id: {}"_tr(param.fileName, id));
            } else {
                output.error("[Chunk] Failed to load structure from file!"_tr());
            }
        });

    // tools chunk structure save <id> <fileName> [ignoreBlocks] [ignoreEntities]
    cmd.overload<SaveStructure>()
        .text("chunk")
        .text("structure")
        .text("save")
        .required("id")
        .required("fileName")
        .optional("ignoreBlocks")
        .optional("ignoreEntities")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, SaveStructure const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            auto& bdInstance = chunk::BindData::getInstance();
            if (bdInstance.hasBindData(param.id)) {
                auto& bindData = bdInstance.getBindData(param.id);
                if (bindData.canSaveStructure) {
                    auto structure = chunk::ChunkManager::getStructureAt(
                        bindData.box,
                        bindData.dimentionId,
                        param.ignoreBlocks,
                        param.ignoreEntities
                    );
                    bool isSuccess =
                        chunk::ChunkManager::getInstance().saveStructure(param.fileName, std::move(structure));
                    if (isSuccess) {
                        bdInstance.removeBindData(param.id);
                        output.success(
                            "[Chunk] Save structure to file {} successful, operation id destroyed!"_tr(param.fileName)
                        );
                    } else {
                        output.error("[Chunk] Failed to save structure!"_tr());
                    }
                } else {
                    output.error(
                        "[Chunk] The data bound to the current operation id is not allowed to save the structure, please check whether the current operation id has been constituency"_tr(
                        )
                    );
                }
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        });

    // tools chunk structure place <id> <pos> <dimentionId> [ignoreEntities]
    cmd.overload<PlaceStructure>()
        .text("chunk")
        .text("structure")
        .text("place")
        .required("id")
        .required("pos")
        .required("dimentionId")
        .optional("ignoreEntities")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, PlaceStructure const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            auto& bdInstance = chunk::BindData::getInstance();
            if (bdInstance.hasBindData(param.id)) {
                auto& bindData = bdInstance.getBindData(param.id);
                if (bindData.canPlaceStructure) {
                    if (bindData.structure == nullptr) {
                        output.error(
                            "[Chunk] The structure data is not available, please load the structure file first!"_tr()
                        );
                        return;
                    }
                    chunk::ChunkManager::placeStructure(
                        param.dimentionId,
                        std::move(bindData.structure),
                        origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos),
                        bindData.mirror,
                        bindData.rotation,
                        false,
                        param.ignoreEntities
                    );
                    bdInstance.removeBindData(param.id);
                    output.success("[Chunk] Place structure successful, operation id destroyed!"_tr());
                } else {
                    output.error(
                        "[Chunk] The data bound to the current operation id is not allowed to place the structure, please check whether the current operation id has been constituency"_tr(
                        )
                    );
                }
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        });

    // StructureTransform

    // tools chunk transform mirror <id> <direction>
    cmd.overload<TransFormMirror>()
        .text("chunk")
        .text("transform")
        .text("mirror")
        .required("id")
        .required("direction")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, TransFormMirror const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            auto& bdInstance = chunk::BindData::getInstance();
            if (bdInstance.hasBindData(param.id)) {
                auto& bindData  = bdInstance.getBindData(param.id);
                bindData.mirror = param.direction;
                output.success("[Chunk] Mirror successful!"_tr());
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        });

    // tools chunk transform rotate <id> <angle>
    cmd.overload<TransFormRotate>()
        .text("chunk")
        .text("transform")
        .text("rotate")
        .required("id")
        .required("angle")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, TransFormRotate const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            auto& bdInstance = chunk::BindData::getInstance();
            if (bdInstance.hasBindData(param.id)) {
                auto& bindData    = bdInstance.getBindData(param.id);
                bindData.rotation = param.angle;
                output.success("[Chunk] Rotate successful!"_tr());
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        });

    // tools chunk cancel <id>
    cmd.overload<OnlyID>().text("chunk").text("cancel").required("id").execute(
        [&](CommandOrigin const& origin, CommandOutput& output, OnlyID const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            auto& bdInstance = chunk::BindData::getInstance();
            if (bdInstance.hasBindData(param.id)) {
                bdInstance.removeBindData(param.id);
                output.success("[Chunk] Operation id {} canceled!"_tr(param.id));
            } else {
                output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
            }
        }
    );

    // tools chunk operateid [id]
    cmd.overload<OnlyID>()
        .text("chunk")
        .text("operateid")
        .optional("id")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, OnlyID const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            CHECK_ChunkOperation_Permission(output, origin);

            auto& bdInstance = chunk::BindData::getInstance();
            if (param.id == -1) {
                auto   allId = bdInstance.getAllBindDataOperatorId();
                string str   = tls::Utils::join(allId, ", ");
                output.success("[Chunk] All operation ids: {}"_tr(str));
            } else {
                if (bdInstance.hasBindData(param.id)) {
                    auto& bindData = bdInstance.getBindData(param.id);
                    output.success("[Chunk] Serach operation id: {}"_tr(param.id));
                    output.success("[Chunk] canSaveStructure: {}"_tr(bindData.canSaveStructure));
                    output.success("[Chunk] canPlaceStructure: {}"_tr(bindData.canPlaceStructure));
                    output.success("[Chunk] canCopyStructure: {}"_tr(bindData.canCopyStructure));
                    output.success("[Chunk] LittelEndianCoordinates: {}"_tr(bindData.box.min.toString()));
                    output.success("[Chunk] BigEndianCoordinates: {}"_tr(bindData.box.min.toString()));
                    output.success("[Chunk] CopyTargetPos: {}"_tr(bindData.copyTargetPos.toString()));
                    output.success("[Chunk] dimentionId: {}"_tr(bindData.dimentionId));
                    output.success("[Chunk] Mirror: {}"_tr(TransFormArgToString(bindData.mirror)));
                    output.success("[Chunk] Rotation: {}"_tr(TransFormArgToString(bindData.rotation)));
                    output.success("[Chunk] hasStructure: {}"_tr(bindData.structure != nullptr));
                } else {
                    output.error("[Chunk] Invalid action id '{}', no access to bound data"_tr(param.id));
                }
            }
        });
}

} // namespace tls::command