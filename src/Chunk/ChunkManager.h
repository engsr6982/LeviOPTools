// stl
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

    LevelChunk* getChunkAt(const Vec3& pos, const Dimension& dimension);

    const Block& getBlockAt(LevelChunk* chunk, const Vec3& pos);

    std::unique_ptr<StructureTemplate>
    getStructureTemplate(LevelChunk* chunk, bool ignoreBlocks = false, bool ignoreEntities = false);

    std::unique_ptr<class CompoundTag>
    getStructureNBT(LevelChunk* chunk, bool ignoreBlocks = false, bool ignoreEntities = false);

    // save and load nbt
    bool saveNbtToFile(LevelChunk* chunk);
    bool saveNbtToFile(std::unique_ptr<class CompoundTag> nbt, string filename);
    bool saveNbtToFile(std::unique_ptr<class CompoundTag> nbt, std::filesystem::path path);

    std::unique_ptr<class CompoundTag> loadNbtFromFile(const ChunkPos& pos);
    std::unique_ptr<class CompoundTag> loadNbtFromFile(const string& filename);
    std::unique_ptr<class CompoundTag> loadNbtFromFile(const std::filesystem::path& path);
};

} // namespace tls::chunk
