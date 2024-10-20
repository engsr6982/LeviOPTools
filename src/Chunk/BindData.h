#pragma once
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/level/levelgen/structure/StructureTemplate.h"
#include <mc/nbt/CompoundTag.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


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

    BindDataItem() {}
    BindDataItem(BoundingBox const& box, int dimId, bool canSaveStructure_)
    : canSaveStructure(canSaveStructure_),
      box(box),
      dimentionId(dimId) {}

    BindDataItem(std::unique_ptr<StructureTemplate> structure, bool canPlaceStructure_)
    : canPlaceStructure(canPlaceStructure_),
      structure(std::move(structure)) {}
};


class BindData {

private:
    BindData()                           = default;
    BindData(const BindData&)            = delete;
    BindData(BindData&&)                 = delete;
    BindData& operator=(const BindData&) = delete;
    BindData& operator=(BindData&&)      = delete;

    std::unordered_map<int, std::unique_ptr<BindDataItem>> bindData; // 玩家绑定数据 key:玩家名 value:绑定数据

    int getOperatorId() {
        static int operatorId = 1;
        return operatorId++;
    }

public:
    static BindData& getInstance() {
        static BindData instance;
        return instance;
    }

    bool hasBindData(int operatorId) { return bindData.find(operatorId) != bindData.end(); }

    int createBindData(BoundingBox const& box, int dimId, bool canSaveStructure) {
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

    int createBindData(std::unique_ptr<StructureTemplate> structure, bool canPlaceStructure) {
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

    BindDataItem& getBindData(int operatorId) { return *bindData[operatorId]; }

    void removeBindData(int operatorId) { bindData.erase(operatorId); }

    std::vector<int> getAllBindDataOperatorId() {
        std::vector<int> operatorIds;
        for (auto& [operatorId, bindDataItem] : bindData) {
            operatorIds.push_back(operatorId);
        }
        return operatorIds;
    }
};


} // namespace tls::chunk