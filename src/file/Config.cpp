#include "Config.h"
#include "entry/Entry.h"

#include <filesystem>
#include <ll/api/Config.h>
#include <ll/api/i18n/I18n.h>

namespace tls::config {

Configs cfg; // init Config

#define CONFIG_FILE_NAME "Config.json"
using ll::i18n_literals::operator""_tr;


bool loadConfig() {
    auto& mSelf    = entry::entry::getInstance().getSelf();
    auto  filePath = mSelf.getConfigDir() / CONFIG_FILE_NAME;
    auto& logger   = mSelf.getLogger();
    // init file
    std::filesystem::path p = filePath;
    if (!std::filesystem::exists(p)) {
        logger.info("Saving default configurations");
        writeConfig(cfg);
    }
    // loading
    if (!ll::config::loadConfig(cfg, filePath)) {
        logger.warn("Unable to load configuration from {}"_tr(filePath));
        return false;
    }
    return true;
}

bool writeConfig(Configs newCfg) {
    auto& mSelf          = entry::entry::getInstance().getSelf();
    auto  configFilePath = mSelf.getConfigDir() / CONFIG_FILE_NAME;
    auto& logger         = mSelf.getLogger();

    if (!ll::config::saveConfig(newCfg, configFilePath)) {
        logger.error("Cannot save configurations to {}"_tr(configFilePath));
        return false;
    }
    return true;
}
bool writeConfig() { return writeConfig(cfg); }

} // namespace tls::config