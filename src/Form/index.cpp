#include "Form/index.h"
#include "Entry/Entry.h"
#include "Entry/PluginInfo.h"
#include "Form/Mapping.h"
#include "Utils/Mc.h"
#include "Utils/Utils.h"
#include <functional>
#include <ll/api/form/CustomForm.h>
#include <ll/api/form/ModalForm.h>
#include <ll/api/form/SimpleForm.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/utils/HashUtils.h>
#include <mc/world/actor/player/Player.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <sys/stat.h>
#include <vector>


namespace tls::form {
using ll::i18n_literals::operator""_tr;
using string     = std::string;
using SimpleForm = ll::form::SimpleForm;
using ModalForm  = ll::form::ModalForm;
using CustomForm = ll::form::CustomForm;

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

void index(Player& player, std::filesystem::path filePath) {
    auto& mSelf    = entry::entry::getInstance().getSelf();
    auto& logger   = mSelf.getLogger();
    auto  jsonData = Utils::readJsonFromFile(filePath);
    if (jsonData.has_value()) {
        FormStruct formStruct = jsonData->get<FormStruct>();

        SimpleForm fm;
        fm.setTitle(formStruct.title.empty() ? "" : formStruct.title);
        fm.setContent(formStruct.content.empty() ? "" : formStruct.content);

        for (const auto& button : formStruct.buttons) {

            std::function<void(Player&)> buttonCallback = [&, button](Player& player) {
                using namespace ll::hash_utils;
                using namespace ll::hash_literals;
                switch (doHash(button.callbackType)) {
                case "cmd"_h:
                    mc::executeCommand(player, button.callbackRun);
                    break;
                case "form"_h:
                    index(player, mSelf.getModDir() / "form" / button.callbackRun);
                    break;
                case "function"_h:
                    if (mapping.find(button.callbackRun) != mapping.end()) {
                        return mapping[button.callbackRun](player);
                    }
                    logger.error("Unsupported function parameters: {}"_tr(button.callbackRun));
                    tls::Utils::sendMsg(player, "Plugin error, go to the console for the cause of the error"_tr());
                    break;
                default:
                    logger.error("Unsupported callbackType: {}"_tr(button.callbackType));
                    tls::Utils::sendMsg(player, "Plugin error, go to the console for the cause of the error"_tr());
                    break;
                }
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
        tls::Utils::sendMsg(player, "Plugin error, go to the console for the cause of the error"_tr());
        logger.error("Plugin error, unable to read or find file \"{}\""_tr(filePath));
    }
}
void index(Player& player) {
    auto filePath = entry::entry::getInstance().getSelf().getModDir() / "form" / "index.json";
    return index(player, filePath);
}

} // namespace tls::form