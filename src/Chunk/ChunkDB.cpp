#include "ChunkDB.h"
#include "Entry/Entry.h"
#include "ll/api/data/KeyValueDB.h"
#include <nlohmann/json.hpp>

namespace tls::chunk {

using string = std::string;
using json   = nlohmann::json;

std::unique_ptr<ll::data::KeyValueDB> ChunkDB::dbInstance = nullptr;

ChunkDB& ChunkDB::getInstance() {
    static ChunkDB instance;
    return instance;
}

std::unique_ptr<ll::data::KeyValueDB>& ChunkDB::getDbInstance() { return dbInstance; }

bool ChunkDB::init() {
    auto path  = tls::entry::getInstance().getSelf().getDataDir() / "ChunkDB";
    dbInstance = std::make_unique<ll::data::KeyValueDB>(path);

    dbInstance->set("chunk_data", json::object().dump());
    dbInstance->set("custom_data", json::object().dump());
    return true;
}


string keyTypeToString(KeyType type) {
    switch (type) {
    case KeyType::ChunkData:
        return "chunk_data";
    case KeyType::CustomData:
        return "custom_data";
    default:
        return "";
    }
}


bool ChunkDB::insert(DBChunkData data) {
    auto chunkData = dbInstance->get(keyTypeToString(data.type));
    // parse json
    json j      = json::parse(*chunkData);
    json j2     = data.toJSON();
    j[data.key] = j2;
    dbInstance->set(keyTypeToString(data.type), j.dump());
    return true;
}

bool ChunkDB::insert(LevelChunk* chunk, KeyType type) {
    DBChunkData data;
    data.type = type;
    data.key  = chunk->getPosition().toString();
    data.min  = chunk->getMin();
    data.max  = chunk->getMax();
    return insert(data);
}

std::optional<DBChunkData> ChunkDB::get(string name, KeyType type) {
    auto chunkData = dbInstance->get(keyTypeToString(type));
    // parse json
    json j = json::parse(*chunkData);
    if (j.contains(name)) {
        return DBChunkData::fromJSON(j[name]);
    }
    return std::nullopt;
}

std::optional<DBChunkData> ChunkDB::get(LevelChunk* chunk, KeyType type) {
    return get(chunk->getPosition().toString(), type);
}

std::optional<DBChunkData> ChunkDB::get(ChunkPos pos, KeyType type) { return get(pos.toString(), type); }

bool ChunkDB::remove(string name, KeyType type) {
    auto chunkData = dbInstance->get(keyTypeToString(type));
    // parse json
    json j = json::parse(*chunkData);
    if (j.contains(name)) {
        j.erase(name);
        dbInstance->set(keyTypeToString(type), j.dump());
        return true;
    }
    return false;
}


} // namespace tls::chunk