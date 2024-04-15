#include "plugin/MyPlugin.h"

#include <memory>

#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"
// my files
#include "Command/Command.h"
#include "file/Config.h"
#include "form/Global.h"
#include "Entry.h"
#include "PluginInfo.h"

namespace my_plugin {

static std::unique_ptr<MyPlugin> instance;

MyPlugin& MyPlugin::getInstance() { return *instance; }

bool MyPlugin::load() {
    mSelf        = std::addressof(self);
    auto& logger = getSelf().getLogger();

    ll::i18n::load(mSelf->getLangDir());
    using ll::i18n_literals::operator""_tr;

    tools::config::loadConfig();

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

bool MyPlugin::enable() {
    getSelf().getLogger().info("Enabling...");
    // Code for enabling the plugin goes here.
    tools::command::regCommand();
    tools::form::initMapping();
    return true;
}

bool MyPlugin::disable() {
    getSelf().getLogger().info("Disabling...");
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace my_plugin

LL_REGISTER_PLUGIN(my_plugin::MyPlugin, my_plugin::instance);
