#include "Command.h"

namespace tls::command {

using string = std::string;
using ll::i18n_literals::operator""_tr;
using ll::command::CommandRegistrar;


void registerChunkCommand() {
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

} // namespace tls::command
