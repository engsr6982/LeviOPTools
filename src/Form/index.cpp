#include "Form/index.h"
#include "entry/Entry.h"
#include "entry/PluginInfo.h"
#include "file/file.h"

#include <fstream>
#include <functional>
#include <ll/api/form/CustomForm.h>
#include <ll/api/form/ModalForm.h>
#include <ll/api/form/SimpleForm.h>
#include <ll/api/i18n/I18n.h>
#include <mc/world/actor/player/Player.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <sys/stat.h>
#include <vector>


namespace tools::form {
using ll::i18n_literals::operator""_tr;
using string     = std::string;
using SimpleForm = ll::form::SimpleForm;
using ModalForm  = ll::form::ModalForm;
using CustomForm = ll::form::CustomForm;
using json       = nlohmann::json;

struct Buttons {
    string title;
    string imageType;
    string imageUrl;
    string callbackType;
    string callbackRun;
};
struct FormStruct {
    string               title;
    string               content;
    std::vector<Buttons> buttons;
};

void from_json(const json& j, Buttons& b) {
    j.at("title").get_to(b.title);
    j.at("imageType").get_to(b.imageType);
    j.at("imageUrl").get_to(b.imageUrl);
    j.at("callbackType").get_to(b.callbackType);
    j.at("callbackRun").get_to(b.callbackRun);
}
void from_json(const json& j, FormStruct& f) {
    j.at("title").get_to(f.title);
    j.at("content").get_to(f.content);
    j.at("buttons").get_to(f.buttons);
}

void index(Player& player) {
    auto& mSelf    = entry::entry::getInstance().getSelf();
    auto& logger   = mSelf.getLogger();
    auto  filePath = mSelf.getPluginDir() / "form" / "index.json";
    auto  jsonData = tools::file::loadJsonFile(filePath);
    if (jsonData) {
        FormStruct formStruct = jsonData->get<FormStruct>();

        SimpleForm fm;
        fm.setTitle(formStruct.title.empty() ? "" : formStruct.title);
        fm.setContent(formStruct.content.empty() ? "" : formStruct.content);

        for (const auto& button : formStruct.buttons) {

            std::function<void(Player&)> buttonCallback = [&, button](Player& player) {
                // 这里可以访问button变量
                player.sendMessage("点击了按钮: " + button.title); // 举例
            };

            if (button.imageType.empty() || button.imageUrl.empty()) {
                fm.appendButton(button.title, buttonCallback);
            } else if (button.imageType == "url" || button.imageType == "path") {
                fm.appendButton(button.title, button.imageUrl, button.imageType, buttonCallback);
            } else {
                logger.error(
                    "Build button failed, imageType value is invalid, allowed value \"url\" or \"path\", current value {}"_tr(
                        button.imageType
                    )
                );
            }
        }

        fm.sendTo(player);
    } else {
        player.sendMessage("Plugin error, unable to read or find file \"{}\""_tr(filePath));
        logger.error("The plugin error is due to the error in the console"_tr());
    }
}


} // namespace tools::form