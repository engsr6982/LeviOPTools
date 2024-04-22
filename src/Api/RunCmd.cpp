#include "Api.h"


namespace tls::api {

bool runCmd(Player& player, const std::string& cmd) {
    try {
        CommandContext context =
            CommandContext(cmd, std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(player)));
        ll::service::getMinecraft()->getCommands().executeCommand(context);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace tls::api