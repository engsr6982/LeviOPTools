#include "ChunkManager.h"
#include "Entry/Entry.h"
#include "mc/world/level/LevelListener.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/level/levelgen/structure/StructureTemplate.h"
#include <algorithm>


namespace tls::chunk {

ChunkManager& ChunkManager::getInstance() {
    static ChunkManager instance;
    return instance;
}

LevelChunk* ChunkManager::getChunkAt(const Vec3& pos, const Dimension& dimension) {
    BlockSource& blockSource = dimension.getBlockSourceFromMainChunkSource();
    BlockPos     blockPos{pos.x, pos.y, pos.z};
    LevelChunk*  levelChunk = blockSource.getChunkAt(blockPos);
    return levelChunk;
}

const Block& ChunkManager::getBlockAt(LevelChunk* chunk, const Vec3& pos) {
    ChunkBlockPos cbp = ChunkBlockPos(pos, chunk->getDimension().getMinHeight());
    Block const&  bl  = chunk->getBlock(cbp);
    return bl;
}

std::unique_ptr<StructureTemplate>
ChunkManager::getStructureTemplate(LevelChunk* chunk, bool ignoreBlocks, bool ignoreEntities) {
    return StructureTemplate::create(
        chunk->getPosition().toString(),
        chunk->getDimension().getBlockSourceFromMainChunkSource(),
        BoundingBox(chunk->getMin(), chunk->getMax()),
        ignoreBlocks,
        ignoreEntities
    );
}

std::unique_ptr<class CompoundTag>
ChunkManager::getStructureNBT(LevelChunk* chunk, bool ignoreBlocks, bool ignoreEntities) {
    return getStructureTemplate(chunk, ignoreBlocks, ignoreEntities)->save();
}

// save and load nbt to/from file

bool ChunkManager::saveNbtToFile(std::unique_ptr<class CompoundTag> nbt, std::filesystem::path path) {
    try {
        auto          structureBin = nbt->toBinaryNbt();
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(structureBin.data()), structureBin.size());
        ofs.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool ChunkManager::saveNbtToFile(std::unique_ptr<class CompoundTag> nbt, string filename) {
    auto path = tls::entry::getInstance().getSelf().getPluginDir() / "chunks" / (filename + ".mcstructure");
    return saveNbtToFile(std::move(nbt), path);
}

bool ChunkManager::saveNbtToFile(LevelChunk* chunk) {
    auto structureNBT = getStructureNBT(chunk, false, false);
    return saveNbtToFile(std::move(structureNBT), chunk->getPosition().toString());
}

std::unique_ptr<class CompoundTag> ChunkManager::loadNbtFromFile(const std::filesystem::path& path) {
    try {
        std::ifstream ifs(path, std::ios::binary);
        auto          structureBin = std::vector<char>(std::istreambuf_iterator<char>(ifs), {});
        ifs.close();

        auto tag = CompoundTag::fromBinaryNbt(std::string_view((char*)structureBin.data(), structureBin.size()));

        if (tag.has_value()) return tag->clone();
        else return nullptr;
    } catch (...) {
        return nullptr;
    }
}

std::unique_ptr<class CompoundTag> ChunkManager::loadNbtFromFile(const string& filename) {
    auto path = tls::entry::getInstance().getSelf().getPluginDir() / "chunks" / (filename + ".mcstructure");
    return loadNbtFromFile(path);
}

std::unique_ptr<class CompoundTag> ChunkManager::loadNbtFromFile(const ChunkPos& pos) {
    return loadNbtFromFile(pos.toString());
}

} // namespace tls::chunk