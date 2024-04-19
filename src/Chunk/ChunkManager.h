// stl
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// ll
#include "ll/api/data/KeyValueDB.h"
#include "ll/api/service/Bedrock.h"

// mc
#include "mc/common/wrapper/optional_ref.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/math/Vec3.h"
#include "mc/nbt/ByteArrayTag.h"
#include "mc/nbt/ByteTag.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/DoubleTag.h"
#include "mc/nbt/EndTag.h"
#include "mc/nbt/FloatTag.h"
#include "mc/nbt/Int64Tag.h"
#include "mc/nbt/IntTag.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/ShortTag.h"
#include "mc/nbt/StringTag.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/level/levelgen/structure/StructureTemplate.h"
#include <mc/nbt/CompoundTag.h>

namespace tls::chunk {

using string = std::string;

class ChunkManager {
private:
    ChunkManager() {}
    ChunkManager(const ChunkManager&)            = delete;
    ChunkManager& operator=(const ChunkManager&) = delete;

public:
    static ChunkManager& getInstance();

    // save and load chunk
    bool                               saveChunk(LevelChunk* chunk);
    std::unique_ptr<StructureTemplate> loadChunk(const ChunkPos& pos);
    bool                               findChunkInFile(const ChunkPos& pos);
    bool                               findChunkInDb(const ChunkPos& pos);
    bool                               canLoadChunk(const ChunkPos& pos);

    // save and load custom chunk data
    bool                               saveCustomData(std::unique_ptr<class CompoundTag> customData);
    std::unique_ptr<StructureTemplate> loadCustomData(string fileName);
    bool                               canLoadCustomData(string fileName);
    bool                               canSaveCustomData(string fileName);

    // structure
    bool                               saveStructure(std::unique_ptr<StructureTemplate> structure);
    std::unique_ptr<StructureTemplate> loadStructure(string fileName);
    bool                               canLoadStructure(string fileName);
    bool                               canSaveStructure(string fileName);

    // core save and load functions
    bool saveFile(std::filesystem::path path, std::unique_ptr<class CompoundTag> nbt);
    bool loadFile(std::filesystem::path path);
    bool findFile(std::filesystem::path path);
    bool canLoadFile(std::filesystem::path path);
    bool canSaveFile(std::filesystem::path path);

    // chunk tools functions
    static LevelChunk*  getChunkAt(const Vec3& pos, const Dimension& dimension);
    static const Block& getBlockAt(LevelChunk* chunk, const Vec3& pos);

    static std::unique_ptr<StructureTemplate> convertTagToStructure(const class CompoundTag& tag);
    static std::unique_ptr<class CompoundTag> convertStructureToTag(const StructureTemplate& structure);
    static std::unique_ptr<StructureTemplate>
    convertLevelChunkToStructure(LevelChunk* chunk, bool ignoreBlocks = false, bool ignoreEntities = false);
};

} // namespace tls::chunk
