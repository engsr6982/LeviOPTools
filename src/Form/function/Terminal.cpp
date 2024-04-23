#include "File/Config.h"
#include "include_all.h"
#include "ll/api/form/CustomForm.h"
#include <vector>

namespace tls::form {

static std::vector<string> logs;

void formatOutput(tls::api::RunCmdExOutput& output) {
    output.input = "> " + output.input;
    if (output.success) output.output = "§a§l●§r " + output.output + "§r";
    else output.output = "§4§l●§r §c" + output.output + "§r";
}

void addLog(tls::api::RunCmdExOutput& output) {
    // 插入日志到末尾
    // 检查是否超过最大长度，超过去除最早的日志，直到符合长度要求
    if (logs.size() >= tls::config::cfg.function.terminalCacheSize) logs.erase(logs.begin());

    // 格式化输出
    formatOutput(output);

    string log = output.input + "\n" + output.output;
    logs.push_back(log);
}

void terminal(Player& player) {
    AutoCheckPermission(player, perms::Terminal);

    CustomForm fm;
    fm.setTitle("LeviOPTools - Terminal");

    fm.appendLabel(join(logs, "\n"));

    fm.appendInput("input", "Terminal >"_tr(), "string");

    fm.sendTo(player, [](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return sendMsg(pl, "Canceled"_tr());
        DebugFormCallBack(dt);

        string input = std::get<string>(dt->at("input"));

        if (input.empty() || input == "") return sendMsg(pl, "Empty input"_tr());

        // execute command
        auto output = tls::api::runCmdEx(input);
        addLog(output);
        terminal(pl);
    });
}

} // namespace tls::form