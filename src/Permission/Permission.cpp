#include "Permission.h"
#include "Entry/Entry.h"
#include "Entry/PluginInfo.h"
#include <PermissionCore/Group.h>
#include <PermissionCore/PermissionCore.h>
#include <PermissionCore/PermissionManager.h>
#include <PermissionCore/PermissionRegister.h>
#include <ll/api/Logger.h>
#include <ll/api/i18n/I18n.h>
#include <stdexcept>


namespace tls::perms {

using ll::i18n_literals::operator""_tr;

void initPermission() {
    pmc::AutoRegisterCoreToManager(PLUGIN_NAME);

    if (pmc::PermissionManager::getInstance().hasRegisterPermissionCore(PLUGIN_NAME) == false) {
        std::runtime_error("Plugin fatal error: PermissionCore register failed."_tr());
    }

    auto& reg = pmc::PermissionRegister::getInstance();

    reg.registerPermission(PLUGIN_NAME, Permission::Unknown, "unknown"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::indexForm, "index_form"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::KickPlayer, "kick_player"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::KillPlayer, "kill_player"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::ChangeWeather, "change_weather"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::ChangeTime, "change_time"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::BroadCastMessage, "broad_cast_message"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::MotdManagement, "motd_management"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::SetServerMaxPlayer, "set_server_max_player"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::Teleport, "teleport"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::CleanDropItem, "clean_drop_item"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::ChangeGameMode, "change_game_mode"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::ChangeGameRule, "change_game_rule"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::GetBlockOrItem, "get_block_or_item"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::Terminal, "terminal"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::SendMessageToPlayer, "send_message_to_player"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::CrashPlayerClient, "crash_player_client"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::UsePlayerIdentitySay, "use_player_identity_say"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::UsePlayerIdentityExecute, "use_player_identity_execute"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::BanGui, "ban_gui"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::PlayerInfo, "player_info"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::CommandBlacklist, "command_blacklist"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::PotionGUI, "potion_gui"_tr());
    reg.registerPermission(PLUGIN_NAME, Permission::ChunkOpertion, "chunk_opertion"_tr());

    tls::entry::getInstance().getSelf().getLogger().info(
        "Register permission success. Total permission count: {}"_tr(reg.getAllPermission(PLUGIN_NAME).size())
    );
}

} // namespace tls::perms