#include "Api.h"
#include "mc/server/commands/CommandOutput.h"

namespace tls::api {

RunCmdExOutput runCmdEx(const string& cmd) {
    RunCmdExOutput result;
    result.input = cmd;

    auto origin =
        ServerCommandOrigin("Server", ll::service::getLevel()->asServer(), CommandPermissionLevel::Internal, 0);
    auto command = ll::service::getMinecraft()->getCommands().compileCommand(
        HashedString(cmd),
        origin,
        (CurrentCmdVersion)CommandVersion::CurrentVersion,
        [](std::string const& err) {}
    );
    CommandOutput output(CommandOutputType::AllOutput);
    string        outputStr;
    try {
        if (command) {
            command->run(origin, output);
            for (auto msg : output.getMessages()) {
                outputStr = outputStr.append(I18n::get(msg.getMessageId(), msg.getParams())).append("\n");
            }
            if (output.getMessages().size()) {
                outputStr.pop_back();
            }
            result.success = output.getSuccessCount() ? true : false;
            result.output  = outputStr;
            return result;
        }
        result.success = false;
        return result;
    } catch (...) {
        result.success = false;
        return result;
        std::runtime_error("Fail in RunCmdEx!");
    }
}

} // namespace tls::Api