#include "Entry.h"
#include "PluginInfo.h"

#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/service/ServerInfo.h>
#include <ll/api/service/Service.h>
#include <memory>


// my files
#include "Command/Command.h"
#include "Config/Config.h"

namespace entry {

entry::entry() = default;

entry& entry::getInstance() {
    static entry instance;
    return instance;
}

ll::plugin::NativePlugin& entry::getSelf() const { return *mSelf; }

bool entry::load(ll::plugin::NativePlugin& self) {
    mSelf        = std::addressof(self);
    auto& logger = getSelf().getLogger();
    // Code for loading the plugin goes here.

    logger.info("Autor: {}", PLUGIN_AUTHOR);
    logger.info(
        "Version: {}.{}.{} for Levilamina and BDS Protocol {}",
        PLUGIN_VERSION_MAJOR,
        PLUGIN_VERSION_MINOR,
        PLUGIN_VERSION_REVISION,
        PLUGIN_TARGET_BDS_PROTOCOL_VERSION
    );
    if (std::filesystem::exists("./plugins/PPOUI/debug")) {
        logger.consoleLevel = 5;
        logger.warn("Printing debugging information is enabled");
    }
    if (ll::getServerProtocolVersion() != PLUGIN_TARGET_BDS_PROTOCOL_VERSION) {
        logger.warn("The bedrock server protocol version does not match, which can lead to unexpected errors. ");
        logger.warn(
            "Current protocol version {}  Adaptation protocol version {}",
            ll::getServerProtocolVersion(),
            PLUGIN_TARGET_BDS_PROTOCOL_VERSION
        );
    }

    tools::config::loadConfig();

    return true;
}

bool entry::enable() {
    getSelf().getLogger().info("enabling...");

    // Code for enabling the plugin goes here.
    tools::command::regCommand();

    return true;
}

bool entry::disable() {
    getSelf().getLogger().info("disabling...");

    // Code for disabling the plugin goes here.

    return true;
}

extern "C" {
_declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) { return entry::getInstance().load(self); }

_declspec(dllexport) bool ll_plugin_enable(ll::plugin::NativePlugin&) { return entry::getInstance().enable(); }

_declspec(dllexport) bool ll_plugin_disable(ll::plugin::NativePlugin&) { return entry::getInstance().disable(); }

/// @warning Unloading the plugin may cause a crash if the plugin has not released all of its
/// resources. If you are unsure, keep this function commented out.
// _declspec(dllexport) bool ll_plugin_unload(ll::plugin::NativePlugin&) {
//     return entry::getInstance().unload();
// }
}

} // namespace entry
