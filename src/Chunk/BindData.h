#pragma once

// stl
#include <chrono>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>
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
#include "mc/world/actor/player/Player.h"
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
#include <memory>


namespace tls::chunk {

using string = std::string;

struct BindDataItem {
    bool canSaveStructure  = false; // 是否可以保存结构
    bool canCopyStructure  = false; // 是否可以复制结构
    bool canPlaceStructure = false; // 是否可以放置结构

    BoundingBox                        box;                          // 选择的区域
    Vec3                               copyTargetPos;                // 复制目标位置
    int                                dimentionId = 0;              // 维度ID
    Mirror                             mirror      = Mirror::None;   // 镜像模式
    Rotation                           rotation    = Rotation::None; // 旋转角度
    std::unique_ptr<StructureTemplate> structure   = nullptr;        // 加载的结构

    ~BindDataItem() {}
    BindDataItem() {}

    BindDataItem(BoundingBox box, int dimId, bool canSaveStructure_)
    : box(box),
      dimentionId(dimId),
      canSaveStructure(canSaveStructure_) {}

    BindDataItem(std::unique_ptr<StructureTemplate> structure, bool canPlaceStructure_)
    : structure(std::move(structure)),
      canPlaceStructure(canPlaceStructure_) {}
};


class BindData {

private:
    BindData()                           = default;
    BindData(const BindData&)            = delete;
    BindData(BindData&&)                 = delete;
    BindData& operator=(const BindData&) = delete;
    BindData& operator=(BindData&&)      = delete;

    std::unordered_map<int, std::unique_ptr<BindDataItem>> bindData; // 玩家绑定数据 key:玩家名 value:绑定数据

    inline int getOperatorId() {
        static int operatorId = 1;
        return operatorId++;
    }

public:
    inline static BindData& getInstance() {
        static BindData instance;
        return instance;
    }

    inline bool hasBindData(const int& operatorId) { return bindData.find(operatorId) != bindData.end(); }

    inline int createBindData(BoundingBox box, int dimId, bool canSaveStructure) {
        int operatorId = getOperatorId();
        if (hasBindData(operatorId)) {
            operatorId = getOperatorId();
        }
        if (hasBindData(operatorId)) {
            return -1;
        }
        bindData[operatorId] = std::make_unique<BindDataItem>(box, dimId, canSaveStructure);
        return operatorId;
    }

    inline int createBindData(std::unique_ptr<StructureTemplate> structure, bool canPlaceStructure) {
        int operatorId = getOperatorId();
        if (hasBindData(operatorId)) {
            operatorId = getOperatorId();
        }
        if (hasBindData(operatorId)) {
            return -1;
        }
        bindData[operatorId] = std::make_unique<BindDataItem>(std::move(structure), canPlaceStructure);
        return operatorId;
    }

    inline BindDataItem& getBindData(const int& operatorId) { return *bindData[operatorId]; }

    inline void removeBindData(const int& operatorId) { bindData.erase(operatorId); }

    inline std::vector<int> getAllBindDataOperatorId() {
        std::vector<int> operatorIds;
        for (auto& [operatorId, bindDataItem] : bindData) {
            operatorIds.push_back(operatorId);
        }
        return operatorIds;
    }
};


} // namespace tls::chunk