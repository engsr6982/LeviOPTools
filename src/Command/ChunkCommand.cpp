#include "Command.h"

namespace tls::command {

using string = std::string;
using ll::i18n_literals::operator""_tr;

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

            Dimension const& dimension   = player->getDimension();
            BlockSource&     blockSource = dimension.getBlockSourceFromMainChunkSource();
            BlockPos         blockPos{playerVec3.x, playerVec3.y, playerVec3.z};
            LevelChunk*      levelChunk = blockSource.getChunkAt(blockPos);
            ChunkPos const&  chunkPos   = levelChunk->getPosition();

            output.success("[Chunk] Current Chunk info:"_tr());
            output.success("[Chunk] ================================="_tr());
            output.success("[Chunk] DimensionId : {}"_tr(dimension.getDimensionId().id));
            output.success("[Chunk] ChunkPos    : {}"_tr(chunkPos.toString()));
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
}

} // namespace tls::command
