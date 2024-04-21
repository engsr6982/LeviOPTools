#include "Command.h"

namespace tls::command {

using ll::i18n_literals::operator""_tr;
using string = std::string;
using ll::command::CommandRegistrar;

void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.tools.commandName,
        config::cfg.command.tools.commandDescription
    );

    // tools
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        if (!checkPlayerPermission(origin, output, tls::perms::indexForm)) {
            output.error("You don't have permission to use this command!"_tr());
            return;
        }
        Actor* entity = origin.getEntity();
        auto&  player = *static_cast<Player*>(entity); // entity* => Player&
        tls::form::index(player);
    }>();

    // tools reload
    cmd.overload().text("reload").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::DedicatedServer);
        tls::config::loadConfig();
        output.success("Config reloaded!"_tr());
    }>();

    // tools kill <Player>
    cmd.overload<Arg_Player>()
        .text("kill")
        .required("player")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Player const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::KillPlayer)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            auto item = param.player.results(origin).data;
            for (Player* target : *item) {
                if (target) {
                    target->kill();
                    player.sendMessage("try kill player: {}"_tr(target->getRealName()));
                }
            }
        }>();

    // tools kick <Player> [Msg]
    cmd.overload<Args_Player_Msg>()
        .text("kick")
        .required("player")
        .optional("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Args_Player_Msg const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::KickPlayer)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            auto item = param.player.results(origin).data;
            for (Player* target : *item) {
                if (target) {
                    target->disconnect(param.message.empty() ? "server disconnect" : param.message);
                    player.sendMessage("try kick player: {}"_tr(target->getRealName()));
                }
            }
        }>();

    // tools crash <Player>
    cmd.overload<Arg_Player>()
        .text("crash")
        .required("player")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Player const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::CrashPlayerClient)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            auto item = param.player.results(origin).data;
            for (Player* target : *item) {
                if (target) {
                    string           name = target->getRealName();
                    LevelChunkPacket pkt  = LevelChunkPacket();
                    pkt.mCacheEnabled     = true;
                    target->sendNetworkPacket(pkt);
                    player.sendMessage("try crash player: {}"_tr(name));
                }
            }
        }>();

    // tools talkas <Player> <msg>
    cmd.overload<Args_Player_Msg>()
        .text("talkas")
        .required("player")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Args_Player_Msg const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::UsePlayerIdentitySay)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            auto item = param.player.results(origin).data;
            for (Player* target : *item) {
                if (target) {
                    TextPacket pkt = TextPacket::createChat(
                        target->getName(),
                        param.message.empty() ? "" : param.message,
                        target->getXuid(),
                        ""
                    );
                    if (ll::service::getLevel().has_value()) {
                        ll::service::getLevel()->forEachPlayer([&pkt](Player& player) {
                            player.sendNetworkPacket(pkt); // send to all player
                            return true;
                        });
                    }
                    player.sendMessage("try talkas player: {}"_tr(target->getRealName()));
                }
            }
        }>();

    // tools broadcast <msg>
    cmd.overload<Arg_Msg>()
        .text("broadcast")
        .required("message")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, Arg_Msg const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::BroadCastMessage)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            Actor* entity = origin.getEntity();
            auto&  player = *static_cast<Player*>(entity);
            // processing
            TextPacket pkt = TextPacket::createChat("Server", param.message.empty() ? "" : param.message, "", "");
            if (ll::service::getLevel().has_value()) {
                ll::service::getLevel()->forEachPlayer([&pkt](Player& player) {
                    player.sendNetworkPacket(pkt); // send to all player
                    return true;
                });
            }
            player.sendMessage("try broadcast message: {}"_tr(param.message));
        }>();

    // tools setmaxplayers <int>
    cmd.overload<SetServerMaxPlayers>()
        .text("setmaxplayers")
        .required("maxPlayers")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, SetServerMaxPlayers const& param) {
            CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
            if (!checkPlayerPermission(origin, output, tls::perms::SetServerMaxPlayer)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            // processing
            int back = ll::service::getServerNetworkHandler()->setMaxNumPlayers(param.maxPlayers);
            ll::service::getServerNetworkHandler()->updateServerAnnouncement();
            output.success("Max players set to {}, previous value is {}"_tr(param.maxPlayers, back));
        }>();
}

} // namespace tls::command
