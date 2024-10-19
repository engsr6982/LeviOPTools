#include "include_all.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/service/Bedrock.h"
#include "mc/network/packet/SetTimePacket.h"

namespace tls::form {

int toTime(string selected) {
    if (selected == "day"_tr()) return 1000;
    else if (selected == "noon"_tr()) return 6000;
    else if (selected == "sunset"_tr()) return 12000;
    else if (selected == "night"_tr()) return 13000;
    else if (selected == "midnight"_tr()) return 18000;
    else if (selected == "sunrise"_tr()) return 23000;
    else return ll::service::getLevel()->getTime();
}


void changeTime(Player& player) {
    AutoCheckPermission(player, perms::ChangeTime);

    CustomForm fm;
    fm.setTitle("LeviOPTools - Change Time"_tr());

    std::vector<string> timeList =
        {"none"_tr(), "day"_tr(), "noon"_tr(), "sunset"_tr(), "night"_tr(), "midnight"_tr(), "sunrise"_tr()};

    // quick change
    fm.appendStepSlider(
        "time",
        "| none | day | noon | sunset | night | midnight | sunrise |\nSelected"_tr(),
        timeList,
        0
    );

    // advanced
    int  currentTime = -1;
    auto level       = ll::service::getLevel();
    if (level) currentTime = level->getTime();

    fm.appendToggle("advanced", "Open Advanced Change"_tr(), false);
    fm.appendInput(
        "custom",
        "Input Custom Time (-2,147,483,648~2,147,483,647)"_tr(),
        "int",
        std::to_string(currentTime)
    );

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return Utils::sendMsg(player, "Canceled"_tr());
        DebugFormCallBack(dt);

        int time     = 0;
        int advanced = std::get<uint64_t>(dt->at("advanced"));
        int custom   = Utils::string2Int(std::get<string>(dt->at("custom")));

        if (advanced == 0) time = toTime(std::get<string>(dt->at("time")));  // quick change
        else if (custom < 2147483648 && custom > -2147483648) time = custom; // advanced change
        else return Utils::sendMsg(pl, "Invalid input"_tr());                // invalid input

        SetTimePacket pkg; // build packet to update clients
        pkg.mTime = time;
        ll::service::getLevel()->setTime(time); // update server time
        ll::service::getLevel()->forEachPlayer([&](Player& p) {
            pkg.sendTo(p); // broadcast packet to all players
            return true;
        });
        Utils::sendMsg(pl, "Operation completed"_tr());
    });
}

} // namespace tls::form
