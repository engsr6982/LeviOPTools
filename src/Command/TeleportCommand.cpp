#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/server/commands/CommandPositionFloat.h"
#include "mc/server/commands/CommandSelector.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Command.h"


namespace tls::command {

using string = std::string;
using ll::i18n_literals::operator""_tr;

struct TeleportToPlayer {
    CommandSelector<Player> player;
    CommandSelector<Player> target;
};

struct TeleportToPosition {
    CommandSelector<Player> player;
    CommandPositionFloat    pos;
    int                     dimension = -114514;
};

void registerTeleportCommand() {
    auto& tools = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        tls::config::cfg.command.tools.commandName,
        tls::config::cfg.command.tools.commandDescription
    );

    // tools teleport <player> [target]
    tools.overload<TeleportToPlayer>()
        .text("teleport")
        .required("player")
        .optional("target")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, TeleportToPlayer const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            if (!checkPlayerPermission(origin, output, perms::Teleport)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            auto player = param.player.results(origin);
            auto target = param.player.results(origin);

            if (target.empty()) {
                // me => player
                if (player.size() == 1) {
                    auto& me = *static_cast<Player*>(origin.getEntity());
                    me.teleport(
                        player.data->data()[0]->getPosition(),
                        player.data->data()[0]->getDimension().getDimensionId()
                    );
                    output.success("Teleported to {}!"_tr(player.data->data()[0]->getRealName()));
                } else {
                    output.error("You have to choose a target player!"_tr());
                }
            } else {
                // player => target
                if (target.size() == 1) {
                    auto t = target.data->data()[0];
                    for (Player* p : *player.data) {
                        p->teleport(t->getPosition(), t->getDimension().getDimensionId());
                        output.success("Teleported {} to {}!"_tr(p->getRealName(), t->getRealName()));
                    }
                } else {
                    output.error("You have to choose a target player!"_tr());
                }
            }
        });

    // tools teleport <player> <x> <y> <z> [dimension]
    tools.overload<TeleportToPosition>()
        .text("teleport")
        .required("player")
        .required("pos")
        .optional("dimension")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, TeleportToPosition const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            if (!checkPlayerPermission(origin, output, perms::Teleport)) {
                output.error("You don't have permission to use this command!"_tr());
                return;
            }
            auto player = param.player.results(origin);
            auto pos    = origin.getExecutePosition(CommandVersion::CurrentVersion, param.pos);

            if (player.empty()) {
                output.error("You need to choose at least one player!"_tr());
                return;
            }

            for (Player* p : *player.data) {
                p->teleport(
                    pos,
                    param.dimension != -114514 ? (DimensionType)param.dimension : p->getDimension().getDimensionId()
                );
                output.success("Teleported {} to {}!"_tr(p->getRealName(), pos.toString()));
            }
        });
}

} // namespace tls::command