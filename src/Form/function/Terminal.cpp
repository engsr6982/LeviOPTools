#include "File/Config.h"
#include "Utils/Mc.h"
#include "include_all.h"
#include "ll/api/form/CustomForm.h"
#include <cstddef>
#include <vector>

namespace tls::form {

static std::vector<string> mLogs;

/**
 * @brief 格式化并追加到日志中
 * @param result 命令执行结果
 * @param cmd 命令
 */
void FormatAndAppendToLogs(std::pair<bool, std::string> const& result, std::string const& cmd) {
    if (mLogs.size() >= (size_t)Config::cfg.function.terminalCacheSize) mLogs.erase(mLogs.begin());

    // format
    string formatedInput = "> " + cmd;

    string formatedOutput;
    if (result.first) formatedOutput = "§a§l●§r " + result.second + "§r";
    else formatedOutput = "§4§l●§r §c" + result.second + "§r";

    mLogs.push_back(formatedInput + "\n" + formatedOutput);
}


void terminal(Player& player) {
    AutoCheckPermission(player, perms::Terminal);

    CustomForm fm;
    fm.setTitle("LeviOPTools - Terminal");

    fm.appendLabel(Utils::join(mLogs, "\n"));

    fm.appendInput("input", "Terminal >"_tr(), "string");

    fm.sendTo(player, [](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return Utils::sendMsg(pl, "Canceled"_tr());
        DebugFormCallBack(dt);

        string input = std::get<string>(dt->at("input"));

        if (input.empty() || input == "") return Utils::sendMsg(pl, "Empty input"_tr());

        // execute command
        auto output = mc::executeCommandEx(input);
        FormatAndAppendToLogs(output, input);
        terminal(pl);
    });
}

} // namespace tls::form