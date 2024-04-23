#include "Motd.h"
#include "ll/api/service/ServerInfo.h"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace tls::motd {

using json = nlohmann::json;

ll::schedule::ServerTimeScheduler motdScheduler;

std::vector<string> motd_list;

void loadMotd() {
    auto& mSelf  = tls::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();
    try {
        std::filesystem::path path = mSelf.getDataDir() / "motd.json";
        if (!std::filesystem::exists(path)) {
            logger.warn("motd file not found!");
            // check dir
            if (!std::filesystem::exists(mSelf.getDataDir())) {
                std::filesystem::create_directories(mSelf.getDataDir());
            }
            // create empty file, value []
            std::ofstream ofs(path);
            ofs << json::array().dump();
            ofs.close();
            return;
        }
        std::ifstream ifs(path);
        json          j = json::parse(ifs);
        ifs.close();
        motd_list.clear();
        for (auto& s : j) {
            motd_list.push_back(s);
        }
    } catch (...) {
        logger.error("load motd failed!");
    }
}

void saveMotd() {
    auto& mSelf  = tls::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();
    try {
        std::filesystem::path path = mSelf.getDataDir() / "motd.json";
        // 检查目录
        if (!std::filesystem::exists(mSelf.getDataDir())) {
            std::filesystem::create_directories(mSelf.getDataDir());
        }
        std::ofstream ofs(path);
        // 检查文件流是否成功打开
        if (!ofs) {
            logger.error("Failed to open motd.json for writing!");
            return;
        }
        json j = json::array();
        for (auto& s : motd_list) {
            j.push_back(s);
        }
        ofs << j.dump();
        // 检查是否成功写入
        if (!ofs.good()) {
            logger.error("Failed to write to motd.json!");
        }
        ofs.close();
    } catch (...) {
        logger.error("save motd failed!");
    }
}


bool nextMotd() {
    static int index = 0;
    if (motd_list.empty()) return false;
    if (index == motd_list.size() - 1) {
        index = 0;
    }
    return ll::setServerMotd(motd_list[index++]);
}

void initMotd() {
    loadMotd();
    motdScheduler.clear();
    motdScheduler.add<RepeatTask>(tls::config::cfg.function.motdShowTime * 20_tick, [&]() {
        if (tls::config::cfg.function.enableMotd == false) {
            motdScheduler.clear();
            return;
        }
        if (motd_list.empty()) return;
        nextMotd();
    });
}

} // namespace tls::motd