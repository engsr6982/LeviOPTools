#include "Config.h"
#include "entry/Entry.h"

#include <filesystem>
#include <ll/api/Config.h>
#include <ll/api/i18n/I18n.h>

namespace tls {

using ll::i18n_literals::operator""_tr;
namespace fs = std::filesystem;

Config Config::cfg; // init Config

bool Config::load() {
    auto& mSelf    = entry::entry::getInstance().getSelf();
    auto  filePath = mSelf.getConfigDir() / CONFIG_FILE_NAME;

    if (!fs::exists(filePath)) {
        save(); // save default config
    }

    if (!ll::config::loadConfig(cfg, filePath)) {
        save(); // 覆写配置文件
    }

    return true;
}

bool Config::save() {
    auto& mSelf          = entry::entry::getInstance().getSelf();
    auto  configFilePath = mSelf.getConfigDir() / CONFIG_FILE_NAME;
    auto& logger         = mSelf.getLogger();

    if (!ll::config::saveConfig(cfg, configFilePath)) {
        logger.error("Cannot save configurations to {}"_tr(configFilePath));
        return false;
    }
    return true;
}

} // namespace tls