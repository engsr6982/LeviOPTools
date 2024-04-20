#include "ChunkManager.h"
#include "Entry/Entry.h"
#include "ll/api/i18n/i18n.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/Service.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/IntTag.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/LevelListener.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/level/levelgen/structure/StructureTemplate.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <tuple>


namespace tls::chunk {

using ll::i18n_literals::operator""_tr;

ChunkManager& ChunkManager::getInstance() {
    static ChunkManager instance;
    return instance;
}


// save and load chunk
bool ChunkManager::saveChunk(LevelChunk* chunk) {
    if (chunk->isFullyLoaded() == false) return false;
    auto tag      = ChunkManager::convertLevelChunkToStructure(chunk, false, true)->save();
    auto fileName = chunk->getPosition().toString() + ".chunk";
    return saveFile(fileName, std::move(tag), FloderType::ChunkBackup, chunk->getDimension().getDimensionId().id);
}

bool ChunkManager::loadChunk(LevelChunk* chunk) {
    if (chunk->isFullyLoaded() == false) return false;
    auto chunkPos = chunk->getPosition();
    if (findChunkFile(chunkPos, chunk->getDimension().getDimensionId().id)) {
        auto tag = loadFile(
            chunkPos.toString() + ".chunk",
            FloderType::ChunkBackup,
            chunk->getDimension().getDimensionId().id
        );
        if (tag) {
            auto structure = ChunkManager::convertTagToStructure(*tag);
            if (structure) {
                ChunkManager::placeStructure(
                    chunk->getDimension().getDimensionId().id,
                    std::move(structure),
                    chunk->getMin(),
                    Mirror::None,
                    Rotation::None,
                    false,
                    true
                );
                return true;
            }
        }
    }
    return false;
}

bool ChunkManager::findChunkFile(const ChunkPos& pos, int dimensionId) {
    return findFile(pos.toString() + ".chunk", FloderType::ChunkBackup, dimensionId);
}


// save and load custom chunk data
bool ChunkManager::saveCustomData(string fileName, BoundingBox box, int dimensionId) {
    fileName                         += ".custom";
    auto structure                    = getStructureAt(box, dimensionId, false, true);
    auto tag                          = structure->save();
    tag->at("levioptools_min_x")      = box.min.x;
    tag->at("levioptools_min_y")      = box.min.y;
    tag->at("levioptools_min_z")      = box.min.z;
    tag->at("levioptools_dimension")  = dimensionId;
    return saveFile(fileName, std::move(tag), FloderType::CustomBackup, dimensionId);
}

bool ChunkManager::loadCustomData(string fileName) {
    fileName += ".custom";
    if (findCustomDataFile(fileName)) {
        auto tag = loadFile(fileName, FloderType::CustomBackup);
        if (tag) {
            auto& minX        = tag->at("levioptools_min_x").get<IntTag>().data;
            auto& minY        = tag->at("levioptools_min_y").get<IntTag>().data;
            auto& minZ        = tag->at("levioptools_min_z").get<IntTag>().data;
            auto& dimensionId = tag->at("levioptools_dimension").get<IntTag>().data;

            ChunkManager::placeStructure(
                dimensionId,
                ChunkManager::convertTagToStructure(*tag),
                BlockPos{minX, minY, minZ},
                Mirror::None,
                Rotation::None,
                false,
                true
            );
            return true;
        }
    }
    return false;
}

bool ChunkManager::findCustomDataFile(string fileName) { return findFile(fileName, FloderType ::CustomBackup); }


// structure
bool ChunkManager::saveStructure(string fileName, std::unique_ptr<StructureTemplate> structure) {
    fileName += ".mcstructure";
    auto tag  = structure->save();
    return saveFile(fileName, std::move(tag), FloderType::Structure, -1);
}

std::unique_ptr<StructureTemplate> ChunkManager::loadStructure(string fileName) {
    fileName += ".mcstructure";
    if (findStructureFile(fileName)) {
        auto tag = loadFile(fileName, FloderType::Structure);
        if (tag) {
            return ChunkManager::convertTagToStructure(*tag);
        }
    }
    return nullptr;
}

bool ChunkManager::findStructureFile(string fileName) { return findFile(fileName, FloderType::Structure); }


// core save and load functions
void ChunkManager::initAllFolders() {
    auto pluginDir       = tls::entry::getInstance().getSelf().getPluginDir();
    auto globalChunkDir  = pluginDir / "Chunk";
    auto ChunBackupDir   = globalChunkDir / "ChunkBackup";
    auto CustomBackupDir = globalChunkDir / "CustomBackup";
    auto StructureDir    = globalChunkDir / "Structure";
    if (!std::filesystem::exists(globalChunkDir)) {
        std::filesystem::create_directories(globalChunkDir);
    }
    if (!std::filesystem::exists(ChunBackupDir)) {
        std::filesystem::create_directories(ChunBackupDir);
    }
    if (!std::filesystem::exists(CustomBackupDir)) {
        std::filesystem::create_directories(CustomBackupDir);
    }
    if (!std::filesystem::exists(StructureDir)) {
        std::filesystem::create_directories(StructureDir);
    }
}

std::filesystem::path ChunkManager::getFilePath(string fileName, ChunkManager::FloderType type, int dimensionId) {
    string typeStr         = "";
    bool   createDimFolder = false;
    switch (type) {
    case tls::chunk::ChunkManager::FloderType::ChunkBackup:
        createDimFolder = true;
        typeStr         = "ChunkBackup";
        break;
    case tls::chunk::ChunkManager::FloderType::CustomBackup:
        typeStr = "CustomBackup";
        break;
    case tls::chunk::ChunkManager::FloderType::Structure:
        typeStr = "Structure";
        break;
    default:
        std::runtime_error("Invalid folder type");
    }
    auto globalPath = tls::entry::getInstance().getSelf().getPluginDir() / "Chunk" / (typeStr);
    if (!createDimFolder) {
        return globalPath / (fileName);
    } else {
        auto dimPath = globalPath / (std::to_string(dimensionId));
        if (!std::filesystem::exists(dimPath)) {
            std::filesystem::create_directories(dimPath);
        }
        return dimPath / (fileName);
    }
}

bool ChunkManager::saveFile(
    string                             fileName,
    std::unique_ptr<class CompoundTag> tag,
    ChunkManager::FloderType           type,
    int                                dimensionId
) {
    try {
        if (tag == nullptr) {
            return false;
        }
        auto          binaryData = tag->toBinaryNbt();
        auto          filePath   = getFilePath(fileName, type, dimensionId);
        std::ofstream ofs(filePath, std::ios::binary);
        ofs.write(binaryData.data(), binaryData.size());
        ofs.close();
        return true;
    } catch (...) {
        tls::entry::getInstance().getSelf().getLogger().fatal("Failed to save file {}", fileName);
        return false;
    }
}

std::unique_ptr<class CompoundTag>
ChunkManager::loadFile(string fileName, ChunkManager::FloderType type, int dimensionId) {
    try {
        if (findFile(fileName, type, dimensionId)) {
            auto          filePath = getFilePath(fileName, type, dimensionId);
            std::ifstream ifs(filePath, std::ios::binary | std::ios::ate);
            if (!ifs.is_open()) {
                return nullptr;
            }
            auto end = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            auto size = std::size_t(end - ifs.tellg());

            if (size == 0) { // 文件为空
                return nullptr;
            }

            std::string binaryData(size, '\0');
            ifs.read(&binaryData[0], size);
            ifs.close();

            return ChunkManager::convertBinaryNbtToTag(binaryData);
        }
        return nullptr;
    } catch (...) {
        tls::entry::getInstance().getSelf().getLogger().fatal("Failed to load file {}", fileName);
        return nullptr;
    }
}

bool ChunkManager::findFile(string fileName, ChunkManager::FloderType type, int dimensionId) {
    auto filePath = getFilePath(fileName, type, dimensionId);
    return std::filesystem::exists(filePath);
}


// chunk tools functions
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

std::unique_ptr<StructureTemplate> ChunkManager::convertTagToStructure(const class CompoundTag& tag) {
    return StructureTemplate::create("", tag);
}

std::unique_ptr<class CompoundTag> ChunkManager::convertStructureToTag(const StructureTemplate& structure) {
    return structure.save();
}

std::unique_ptr<StructureTemplate>
ChunkManager::convertLevelChunkToStructure(LevelChunk* chunk, bool ignoreBlocks, bool ignoreEntities) {
    return StructureTemplate::create(
        chunk->getPosition().toString(),
        chunk->getDimension().getBlockSourceFromMainChunkSource(),
        BoundingBox(chunk->getMin(), chunk->getMax()),
        ignoreBlocks,
        ignoreEntities
    );
}

std::unique_ptr<class CompoundTag> ChunkManager::convertBinaryNbtToTag(const string& binaryNbt) {
    return CompoundTag::fromBinaryNbt(binaryNbt)->clone();
}

std::unique_ptr<StructureTemplate>
ChunkManager::getStructureAt(BoundingBox box, int dimensionId, bool ignoreBlocks, bool ignoreEntities) {
    try {
        return StructureTemplate::create(
            "",
            ll::service::getLevel()->getDimension(dimensionId)->getBlockSourceFromMainChunkSource(),
            box,
            ignoreBlocks,
            ignoreEntities
        );
        // catch std::length_error, vector too long
    } catch (const std::length_error& e) {
        tls::entry::getInstance().getSelf().getLogger().error(
            "Selection error, point 1: {} | point 2: {}, original error: {}"_tr(
                box.min.toString(),
                box.max.toString(),
                e.what()
            )
        );
        return nullptr;
    }
}

void ChunkManager::checkAndFixLittelEndianCooridnates(BoundingBox& box) {
    // Checking small end coordinates and large end coordinates
    // Y-axis is given to box.max.y if it is big, or box.min.y if it is small.
    if (box.min.y > box.max.y) {
        std::swap(box.min.y, box.max.y);
    }
    // Checking the x and z axis
    if (box.min.x > box.max.x) {
        std::swap(box.min.x, box.max.x);
    }
    if (box.min.z > box.max.z) {
        std::swap(box.min.z, box.max.z);
    }
    tls::entry::getInstance().getSelf().getLogger().debug(
        "try fix little endian coordinates, min: {}, max: {}"_tr(box.min.toString(), box.max.toString())
    );
}

// structure place in world
void ChunkManager::placeStructure(
    int                                dimensionId,
    std::unique_ptr<StructureTemplate> structure,
    BlockPos                           minCorner,
    Mirror                             mirror,
    Rotation                           rotation,
    bool                               ignoreBlocks,
    bool                               ignoreEntities
) {
    auto& blockSource = ll::service::getLevel()->getDimension(dimensionId)->getBlockSourceFromMainChunkSource();
    structure->placeInWorld(blockSource, minCorner, mirror, rotation, ignoreBlocks, ignoreEntities);
}

} // namespace tls::chunk