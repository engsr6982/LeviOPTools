#include "Command.h"

namespace tls::command {

using string = std::string;
using ll::i18n_literals::operator""_tr;
using ll::command::CommandRegistrar;

void registerGamemodeCommand() {
#define Gm_String_Full_CallBack                                                                                        \
    [&](CommandOrigin const& origin, CommandOutput& output, GameMode_String_Full const& param) {                       \
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);                                 \
        if (!checkPlayerPermission(origin, output, tls::perms::ChangeGameMode)) {                                      \
            output.error("You don't have permission to use this command!"_tr());                                       \
            return;                                                                                                    \
        }                                                                                                              \
        if (param.player.results(origin).empty()) {                                                                    \
            auto& player = *static_cast<Player*>(origin.getEntity());                                                  \
            player.setPlayerGameType(param.gameType);                                                                  \
            output.success("try set game mode: {}"_tr(param.gameType));                                                \
        } else {                                                                                                       \
            auto player = param.player.results(origin).data;                                                           \
            for (Player * target : *player) {                                                                          \
                if (target) {                                                                                          \
                    target->setPlayerGameType(param.gameType);                                                         \
                    output.success("try set game mode: {} to player: {}"_tr(param.gameType, target->getRealName()));   \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }

#define Gm_String_Brief_CallBack                                                                                       \
    [&](CommandOrigin const& origin, CommandOutput& output, GameMode_String_Brief const& param) {                      \
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);                                 \
        if (!checkPlayerPermission(origin, output, tls::perms::ChangeGameMode)) {                                      \
            output.error("You don't have permission to use this command!"_tr());                                       \
            return;                                                                                                    \
        }                                                                                                              \
        if (param.player.results(origin).empty()) {                                                                    \
            auto& player = *static_cast<Player*>(origin.getEntity());                                                  \
            player.setPlayerGameType((GameType)param.gameType);                                                        \
            output.success("try set game mode: {}"_tr(param.gameType));                                                \
        } else {                                                                                                       \
            auto player = param.player.results(origin).data;                                                           \
            for (Player * target : *player) {                                                                          \
                if (target) {                                                                                          \
                    target->setPlayerGameType((GameType)param.gameType);                                               \
                    output.success("try set game mode: {} to player: {}"_tr(param.gameType, target->getRealName()));   \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }

#define Gm_Int_CallBack                                                                                                \
    [&](CommandOrigin const& origin, CommandOutput& output, GameMode_Int const& param) {                               \
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);                                 \
        if (!checkPlayerPermission(origin, output, tls::perms::ChangeGameMode)) {                                      \
            output.error("You don't have permission to use this command!"_tr());                                       \
            return;                                                                                                    \
        }                                                                                                              \
        if (param.gameType == -1 || param.gameType == 0 || param.gameType == 1 || param.gameType == 2                  \
            || param.gameType == 5 || param.gameType == 6) {                                                           \
            if (param.player.results(origin).empty()) {                                                                \
                auto& player = *static_cast<Player*>(origin.getEntity());                                              \
                player.setPlayerGameType((GameType)param.gameType);                                                    \
                output.success("try set game mode: {}"_tr(param.gameType));                                            \
            } else {                                                                                                   \
                auto player = param.player.results(origin).data;                                                       \
                for (Player * target : *player) {                                                                      \
                    if (target) {                                                                                      \
                        target->setPlayerGameType((GameType)param.gameType);                                           \
                        output.success("try set game mode: {} to player: {}"_tr(param.gameType, target->getRealName()) \
                        );                                                                                             \
                    }                                                                                                  \
                }                                                                                                      \
            }                                                                                                          \
        } else {                                                                                                       \
            output.error("Invalid game mode!"_tr());                                                                   \
        }                                                                                                              \
    }
    auto& tools = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config::cfg.command.tools.commandName,
        config::cfg.command.tools.commandDescription
    );
    // tools gm <int> [player]
    tools.overload<GameMode_Int>().text("gm").required("gameType").optional("player").execute<Gm_Int_CallBack>();

    // tools gm <s|c|a|d> [player]
    tools.overload<GameMode_String_Brief>()
        .text("gm")
        .required("gameType")
        .optional("player")
        .execute<Gm_String_Brief_CallBack>();

    // tools gm <GameType> [player]
    tools.overload<GameMode_String_Full>()
        .text("gm")
        .required("gameType")
        .optional("player")
        .execute<Gm_String_Full_CallBack>();

    if (config::cfg.command.gm.enable) {
        auto& gm = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
            config::cfg.command.gm.commandName,
            config::cfg.command.gm.commandDescription
        );
        // gm <int> [player]
        gm.overload<GameMode_Int>().required("gameType").optional("player").execute<Gm_Int_CallBack>();

        // gm <s|c|a|d> [player]
        gm.overload<GameMode_String_Brief>()
            .required("gameType")
            .optional("player")
            .execute<Gm_String_Brief_CallBack>();

        // gm <GameType> [player]
        gm.overload<GameMode_String_Full>().required("gameType").optional("player").execute<Gm_String_Full_CallBack>();
    }
}

} // namespace tls::command