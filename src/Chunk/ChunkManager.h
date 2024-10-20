#include "ll/api/data/KeyValueDB.h"
#include "ll/api/service/Bedrock.h"
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
#include <filesystem>
#include <mc/nbt/CompoundTag.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


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
    bool saveChunk(LevelChunk* chunk);
    bool loadChunk(LevelChunk* chunk);
    bool findChunkFile(const ChunkPos& pos, int dimensionId = 0);


    // save and load custom chunk data
    bool saveCustomData(string const& fileName, BoundingBox const& box, int dimensionId);
    bool loadCustomData(string const& fileName);
    bool findCustomDataFile(string const& fileName);


    // structure
    bool saveStructure(string const& fileName, std::unique_ptr<StructureTemplate> structure);
    std::unique_ptr<StructureTemplate> loadStructure(string const& fileName);
    bool                               findStructureFile(string const& fileName);


    // core save and load functions
    enum class FloderType : int { ChunkBackup = 1, CustomBackup = 2, Structure = 3 };
    std::filesystem::path getFilePath(string fileName, FloderType type, int dimensionId = -1);
    static void           initAllFolders();

    bool saveFile(
        string                       fileName,
        std::unique_ptr<CompoundTag> tag,
        FloderType                   type,
        int                          dimensionId = -1 // dimensionId = -1 for global
    );
    std::unique_ptr<CompoundTag> loadFile(string fileName, FloderType type, int dimensionId = -1);
    bool                         findFile(string fileName, FloderType type, int dimensionId = -1);


    // static
    static LevelChunk*  getChunkAt(const Vec3& pos, const Dimension& dimension);
    static const Block& getBlockAt(LevelChunk* chunk, const Vec3& pos);

    static std::unique_ptr<StructureTemplate> convertTagToStructure(const CompoundTag& tag);
    static std::unique_ptr<CompoundTag>       convertStructureToTag(const StructureTemplate& structure);
    static std::unique_ptr<CompoundTag>       convertBinaryNbtToTag(const string& binaryNbt);
    static std::unique_ptr<StructureTemplate>
    convertLevelChunkToStructure(LevelChunk* chunk, bool ignoreBlocks = false, bool ignoreEntities = false);


    static std::unique_ptr<StructureTemplate>
    getStructureAt(BoundingBox const& box, int dimensionId, bool ignoreBlocks = false, bool ignoreEntities = false);

    static void checkAndFixMinMax(BoundingBox& box);

    static void placeStructure(
        int                                dimensionId,
        std::unique_ptr<StructureTemplate> structure,
        BlockPos const&                    minCorner,
        Mirror                             mirror         = Mirror::None,
        Rotation                           rotation       = Rotation::None,
        bool                               ignoreBlocks   = false,
        bool                               ignoreEntities = false
    );
};

} // namespace tls::chunk
