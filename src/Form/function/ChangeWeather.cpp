#include "include_all.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/service/Bedrock.h"

namespace tls::form {

void changeWeather(Player& player) {
    AutoCheckPermission(player, perms::ChangeWeather);

    CustomForm fm;
    fm.setTitle("LeviOPTools - Change Weather"_tr());

    fm.appendLabel(
        "Modify the world's weather, provide quick changes and advanced customization changes, note that advanced changes need to pay attention to the parameter type, otherwise it may lead to a crash"_tr(
        )
    );
    // quick changes
    std::vector<string> weatherList = {"none"_tr(), "clean"_tr(), "rain"_tr(), "thunder"_tr()};
    fm.appendStepSlider("weather", "| none | clean | rain | thunder |"_tr(), weatherList, 0);


    // advanced changes
    fm.appendToggle("advanced", "Open advanced changes"_tr(), false); // advanced: false
    fm.appendInput(
        "rainLevel", // 下雨强度 0.0：无雨，1.0：下雨， 1065353216.0： 大雨
        "Rain level(0.0: no rain, 1.0: heavy rain, 1065353216.0: heavy storm)"_tr(),
        "float",
        "0.0"
    );
    fm.appendInput(
        "rainTime", // 下雨持续时间 单位tick， -1代表随机
        "Rain Time(Unit: tick, -1: random)"_tr(),
        "int",
        "-1"
    );
    fm.appendInput(
        "lightningLevel", // 雷电强度 0.0：无雷，1065353216.0：大雷
        "lightningLevel(0.0: no lightning, 1065353216.0: heavy lightning)"_tr(),
        "float",
        "0.0"
    );
    fm.appendInput(
        "lightningTime", // 雷电持续时间 单位tick， -1代表随机
        "lightningTime(Unit: tick, -1: random)"_tr(),
        "int",
        "-1"
    );

    fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return sendMsg(pl, "Cancelled");

        auto level = ll::service::getLevel();
        if (!level) return sendMsg(pl, "Failed to get level");

        int  selectedWeather = std::get<uint64_t>(dt->at("weather")); // weather: 0: None, 1: Clear, 2: Rain, 3: Thunder
        bool advanced        = std::get<uint64_t>(dt->at("advanced")); // advanced: false

        // change args
        int   random         = 20 * (ll::random_utils::rand(600) + 300);
        float rainLevel      = 0.0; // default clear weather
        int   rainTime       = random;
        float lightningLevel = 0.0;
        int   lightningTime  = random;

        if (advanced) {
            rainLevel      = std::get<double>(dt->at("rainLevel"));
            lightningLevel = std::get<double>(dt->at("lightningLevel"));

            int rTime = std::get<uint64_t>(dt->at("rainTime"));
            int lTime = std::get<uint64_t>(dt->at("lightningTime"));

            rainTime      = rTime == -1 ? random : rTime; // check is random
            lightningTime = lTime == -1 ? random : lTime;
        } else {
            switch (selectedWeather) {
            case 2: // Rain
                rainLevel      = 1.0;
                rainTime       = random;
                lightningLevel = 0.0;
                lightningTime  = random;
                break;
            case 3: // Thunder
                rainLevel      = 1065353216.0;
                rainTime       = random;
                lightningLevel = 1065353216.0;
                lightningTime  = random;
                break;
            default: // clear and none
                break;
            }
        }

        level->updateWeather(rainLevel, rainTime, lightningLevel, lightningTime);
        sendMsg(pl, "Operation completed");
    });
}

} // namespace tls::form