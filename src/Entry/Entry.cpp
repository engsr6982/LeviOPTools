#include <ll/api/Logger.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/ServerInfo.h>
#include <ll/api/service/Service.h>
#include <memory>

// my files
#include "Chunk/ChunkManager.h"
#include "Command/Command.h"
#include "Config/Config.h"
#include "Entry.h"
#include "Motd/Motd.h"
#include "Permission/Permission.h"
#include "PluginInfo.h"



namespace tls {

static std::unique_ptr<entry> instance;

entry& entry::getInstance() { return *instance; }

bool entry::load() {
    auto& logger = getSelf().getLogger();

    // load i18n
    ll::i18n::load(mSelf.getLangDir());
    using ll::i18n_literals::operator""_tr;

    // load
    tls::Config::load();
    tls::chunk::ChunkManager::initAllFolders();
    logger.consoleLevel = tls::Config::cfg.loggerLevel;

    // print plugin info
    logger.info("Autor: {}"_tr(PLUGIN_AUTHOR));
    return true;
}

bool entry::enable() {
    getSelf().getLogger().info("Enabling...");

    tls::command::registerCommand();
    tls::command::registerChunkCommand();
    tls::command::registerGamemodeCommand();
    tls::command::registerTeleportCommand();
    tls::perms::initPermission();
    tls::motd::initMotd();

    return true;
}

bool entry::disable() {
    getSelf().getLogger().info("Disabling...");

    return true;
}

} // namespace tls

LL_REGISTER_MOD(tls::entry, tls::instance);
