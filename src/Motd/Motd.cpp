#include "Motd.h"
#include "Entry/Entry.h"
#include "Utils/Utils.h"
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

    fs::path path = mSelf.getDataDir() / "motd.json";
    auto     val  = Utils::readJsonFromFile(path);
    if (!val.has_value()) {
        logger.error("Failed to read motd.json!");
        return;
    }

    motd_list.clear();
    for (auto& s : val.value()) {
        motd_list.push_back(s);
    }
}

void saveMotd() {
    auto& mSelf  = tls::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();

    fs::path path = mSelf.getDataDir() / "motd.json";

    json data = json::array();
    for (auto& i : motd_list) {
        data.push_back(i);
    }

    bool ok = Utils::writeJsonToFile(path, data);
    if (!ok) {
        logger.error("Failed to write to motd.json!");
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
    motdScheduler.add<RepeatTask>(ll::chrono::ticks(tls::Config::cfg.function.motdShowTime * 20), []() {
        if (!Config::cfg.function.enableMotd) {
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