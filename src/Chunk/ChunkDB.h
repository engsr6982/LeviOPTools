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
#include "rapidjson/reader.h"
#include <mc/nbt/CompoundTag.h>

// library
#include "nlohmann/json.hpp"

namespace tls::chunk {

using string = std::string;
using json   = nlohmann::json;

/*

{
    "chunk_data": {         // 区块备份
        [key: ChunkPos]: {  // 区块坐标
            min: {          // 区块对角线坐标
                x: int,
                y: int,
                z: int
            },
            max: {
                x: int,
                y: int,
                z: int
            }
        }
    },
    "custom_data": {        // 自定义备份区域
        [key: string]: {    // 文件名
            min: {
                x: int,
                y: int,
                z: int
            },
            max: {
                x: int,
                y: int,
                z: int
            }
        }
    }
}

*/


enum class KeyType : int { ChunkData = 1, CustomData = 2 };
struct DBChunkData {
    string  key;
    KeyType type;
    Vec3    min;
    Vec3    max;

    static inline DBChunkData fromJSON(const string& jsonStr) {
        json        j = json::parse(jsonStr);
        DBChunkData data;
        data.key = j["key"];
        // data.type = static_cast<KeyType>(j["type"]);
        data.min = Vec3(j["min"]["x"].get<int>(), j["min"]["y"].get<int>(), j["min"]["z"].get<int>());
        data.max = Vec3(j["max"]["x"].get<int>(), j["max"]["y"].get<int>(), j["max"]["z"].get<int>());
        return data;
    }

    inline json toJSON() const {
        json j;
        j["key"] = key;
        // j["type"]     = static_cast<int>(type);
        j["min"]      = json::object();
        j["min"]["x"] = min.x;
        j["min"]["y"] = min.y;
        j["min"]["z"] = min.z;
        j["max"]      = json::object();
        j["max"]["x"] = max.x;
        j["max"]["y"] = max.y;
        j["max"]["z"] = max.z;
        return j;
    }

    inline string toString(int indent = -1) const { return toJSON().dump(indent); }
};

class ChunkDB {
private:
    ChunkDB() {}
    ChunkDB(const ChunkDB&)            = delete;
    ChunkDB& operator=(const ChunkDB&) = delete;

    static std::unique_ptr<ll::data::KeyValueDB> dbInstance;

public:
    static ChunkDB&                               getInstance();
    static std::unique_ptr<ll::data::KeyValueDB>& getDbInstance();

    bool init();

    bool insert(LevelChunk* chunk, KeyType type);
    bool insert(DBChunkData data);

    std::optional<DBChunkData> get(string name, KeyType type);
    std::optional<DBChunkData> get(LevelChunk* chunk, KeyType type);
    std::optional<DBChunkData> get(ChunkPos pos, KeyType type);

    bool remove(string name, KeyType type);
};

} // namespace tls::chunk
