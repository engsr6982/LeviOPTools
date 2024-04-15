#include <ll/api/Logger.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/RegisterHelper.h>
#include <ll/api/service/ServerInfo.h>
#include <ll/api/service/Service.h>
#include <memory>

// my files
#include "Command/Command.h"
#include "Entry.h"
#include "File/Config.h"
#include "Form/Form.h"
#include "PluginInfo.h"


namespace tls {

static std::unique_ptr<entry> instance;

entry& entry::getInstance() { return *instance; }

bool entry::load() {
    auto& logger = getSelf().getLogger();

    ll::i18n::load(mSelf.getLangDir());
    using ll::i18n_literals::operator""_tr;

    tls::config::loadConfig();

    logger.info("Autor: {}"_tr(PLUGIN_AUTHOR));
    logger.info("Version: {}.{}.{} for Levilamina and BDS Protocol {}"_tr(
        PLUGIN_VERSION_MAJOR,
        PLUGIN_VERSION_MINOR,
        PLUGIN_VERSION_REVISION,
        PLUGIN_TARGET_BDS_PROTOCOL_VERSION
    ));

    if (std::filesystem::exists("./plugins/PPOUI/debug")) {
        logger.consoleLevel = 5;
        logger.warn("Printing debugging information is enabled"_tr());
    }
    if (ll::getServerProtocolVersion() != PLUGIN_TARGET_BDS_PROTOCOL_VERSION) {
        logger.warn("The bedrock server protocol version does not match, which can lead to unexpected errors. "_tr());
        logger.warn("Current protocol version {}  Adaptation protocol version {}"_tr(
            ll::getServerProtocolVersion(),
            PLUGIN_TARGET_BDS_PROTOCOL_VERSION
        ));
    }
    return true;
}

bool entry::enable() {
    getSelf().getLogger().info("Enabling...");
    // Code for enabling the plugin goes here.
    tls::command::regCommand();
    tls::form::initMapping();
    return true;
}

bool entry::disable() {
    getSelf().getLogger().info("Disabling...");
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace tls

LL_REGISTER_PLUGIN(tls::entry, tls::instance);
