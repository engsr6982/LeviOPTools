#include "Motd.h"
#include "ll/api/service/ServerInfo.h"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace tls::motd {

using json = nlohmann::json;

void loadMotd() {
    auto& mSelf  = tls::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();
    try {
        std::filesystem::path path = mSelf.getDataDir() / "motd.json";
        if (!std::filesystem::exists(path)) {
            logger.warn("motd file not found!");
            // create empty file, value []
            std::ofstream ofs(path);
            ofs << json::array();
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
        std::ofstream         ofs(path);
        json                  j;
        for (auto& s : motd_list) {
            j.push_back(s);
        }
        ofs << j;
        ofs.close();
    } catch (...) {
        logger.error("save motd failed!");
    }
}


bool nextMotd() {
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