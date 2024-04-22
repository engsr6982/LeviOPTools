#include "File/Config.h"
#include "include_all.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/storage/GameRule.h"
#include "mc/world/level/storage/GameRuleId.h"
#include "mc/world/level/storage/GameRules.h"
#include <filesystem>
#include <unordered_map>


namespace tls::form {

string getTr(string rule) {
    static std::unordered_map<string, string> trList;
    auto&                                     mSelf  = tls::entry::getInstance().getSelf();
    auto&                                     logger = mSelf.getLogger();
    try {
        if (trList.empty()) {
            // load translations
            std::filesystem::path path = mSelf.getPluginDir() / tls::config::cfg.gameRuleTranslatFilePath; // json file
            if (std::filesystem::exists(path)) {
                auto json = nlohmann::json::parse(std::ifstream(path));
                for (auto& [key, value] : json.items()) {
                    try {
                        trList[key] = value.get<string>();
                        // clang-format off
                        #ifdef DEBUG
                            logger.info("GameRule translation: {} -> {}"_tr(key, value.get<string>()));
                        #endif
                        // clang-format on
                    } catch (...) {
                        logger.error("GameRule translation parse error: {}"_tr(key));
                    }
                }
            } else {
                logger.warn("GameRule translation file not found: {}"_tr(path.string()));
                return rule;
            }
        }

        if (trList.find(rule) != trList.end()) {
            return trList[rule];
        }
        logger.warn("Untranslated game rules: {}"_tr(rule));
        return rule;
    } catch (const nlohmann::json::parse_error& e) {
        logger.error("GameRule translation file parse error: {}"_tr(e.what()));
        return rule;
    } catch (...) {
        logger.error("Unknown error in GameRule translation!"_tr());
        return rule;
    }
}

void changeGameRule(Player& player) {
    AutoCheckPermission(player, perms::ChangeGameRule);
    auto& logger = tls::entry::getInstance().getSelf().getLogger();
    try {
        CustomForm fm;
        fm.setTitle("LeviOPTools - Change Game Rule"_tr());

        // Get the level
        auto level = ll::service::getLevel();
        if (!level) return sendMsg(player, "Failed to get level Ptr"_tr());

        // Get the game rules
        auto rules = level->getGameRules();
        auto list  = rules.getRules();

        // Sort the list by bool rules first
        std::sort(list.begin(), list.end(), [](const GameRule& a, const GameRule& b) {
            return a.getType() == GameRule::Type::Bool && b.getType() != GameRule::Type::Bool;
        });

        // build the form
        for (auto& rule : list) {
            try {
                if (rule.getType() == GameRule::Type::Bool) {
                    fm.appendToggle( // bool => toggle
                        rule.mName,
                        getTr(rule.mName),
                        rule.getBool()
                    );
                } else if (rule.getType() == GameRule::Type::Int) {
                    fm.appendInput( // int => input
                        rule.mName,
                        getTr(rule.mName),
                        "int",
                        std::to_string(rule.getInt())
                    );
                } else if (rule.getType() == GameRule::Type::Float) {
                    fm.appendInput( // float => input
                        rule.mName,
                        getTr(rule.mName),
                        "float",
                        std::to_string(rule.getFloat())
                    );
                } else { // unknown type => ignore
                    logger.warn("Unparsable game rules: {}"_tr(rule.getName()));
                }
            } catch (...) {
                logger.error("Failed to build game rule form"_tr());
            }
        }

        fm.sendTo(player, [&](Player& pl, CustomFormResult const& dt, FormCancelReason) {
            try {
                if (!dt) return sendMsg(pl, "form cancelled");
                DebugFormCallBack(dt);
            } catch (...) {
                logger.error("Failed to handle Game Rule Form Callback"_tr());
            }
        });
    } catch (...) {
        logger.error("Failed to Build and Send Game Rule Form"_tr());
    }
}

} // namespace tls::form