#include "include_all.h"

namespace tls::form {

// int toTimeValue(int selected) {
//     switch (selected) {
//     case 1: // day
//         return 1000;
//     case 2: // noon
//         return 6000;
//     case 3: // sunset
//         return 12000;
//     case 4: // night
//         return 13000;
//     case 5: // midnight
//         return 18000;
//     case 6: // sunrise
//         return 23000;
//     default:
//         return 0;
//     }
// };

void changeTime(Player& player) {

    // std::vector<string> timeList =
    //     {"none"_tr(), "day"_tr(), "noon"_tr(), "sunset"_tr(), "night"_tr(), "midnight"_tr(), "sunrise"_tr()};

    // int  currentTime = -1;
    // auto level       = ll::service::getLevel();
    // if (level) currentTime = level->getTime();

    // fm.appendLabel("Current World Time: {}"_tr(currentTime));
    // fm.appendStepSlider("time", "| day | noon | sunset | night | midnight | sunrise |"_tr(), timeList, 0);

    // int selectedTime = std::get<uint64_t>(dt->at("time"));
    // if (selectedTime != 0) {
    //     int timeValue = toTimeValue(selectedTime);
    //     level->setTime(timeValue);
    //     sendMsg(pl, "Set the world time to '{}'"_tr(timeList[selectedTime]));
    // }
}

} // namespace tls::form
