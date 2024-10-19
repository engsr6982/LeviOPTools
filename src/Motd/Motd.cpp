#include "Motd.h"
#include "Entry/Entry.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/ServerInfo.h"
#include <cstddef>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>


namespace tls::motd {

using json   = nlohmann::json;
using string = std::string;
using namespace ll::schedule;
using namespace ll::chrono_literals;
namespace fs = std::filesystem;

ll::schedule::GameTickAsyncScheduler motdScheduler;

std::vector<string> motd_list;

void loadMotd() {
    auto& mSelf  = tls::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();
    try {
        fs::path path = mSelf.getDataDir() / "motd.json";
        if (!fs::exists(path)) {
            logger.warn("motd file not found!");
            // check dir
            if (!fs::exists(mSelf.getDataDir())) {
                fs::create_directories(mSelf.getDataDir());
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
        fs::path path = mSelf.getDataDir() / "motd.json";

        if (!fs::exists(path)) {
            fs::create_directories(mSelf.getDataDir());
        }

        std::ofstream ofs(path);
        if (!ofs) {
            logger.error("Failed to open motd.json for writing!");
            return;
        }

        json j = json::array();
        for (auto& s : motd_list) {
            j.push_back(s);
        }
        ofs << j.dump();

        if (!ofs.good()) {
            logger.error("Failed to write to motd.json!");
        }
        ofs.close();
    } catch (...) {
        logger.error("save motd failed!");
    }
}


bool nextMotd() {
    static size_t index = 0;
    index               = (index + 1) % motd_list.size(); // 循环
    if (index >= motd_list.size()) {
        index = 0; // 防止越界
    }
    return ll::setServerMotd(motd_list[index]);
}

void initMotd() {
    loadMotd();
    motdScheduler.clear();
    motdScheduler.add<RepeatTask>(ll::chrono::ticks(tls::config::cfg.function.motdShowTime * 20), []() {
        if (!config::cfg.function.enableMotd) {
            motdScheduler.clear();
            return;
        }
        if (motd_list.empty()) {
            return;
        }
        nextMotd();
    });
}

} // namespace tls::motd