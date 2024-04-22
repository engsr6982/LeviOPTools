#include "File/Config.h"
#include "include_all.h"
#include "ll/api/service/Bedrock.h"
#include "mc/network/packet/GameRulesChangedPacket.h"
#include "mc/world/level/storage/GameRule.h"
#include "mc/world/level/storage/GameRuleId.h"
#include "mc/world/level/storage/GameRules.h"
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>


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

string joinErrorParameters(std::vector<string> mErrorParameters) {
    if (mErrorParameters.empty()) return "";

    return std::accumulate(
        std::next(mErrorParameters.begin()),
        mErrorParameters.end(),
        mErrorParameters[0],
        [](const string& a, const string& b) -> string { return a + "\n" + b; }
    );
}

bool isFloat(const std::string& str) {
    float value    = 0.0f;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

    // 检查转换是否成功，并且整个字符串都被解析
    return ec == std::errc() && ptr == str.data() + str.size();
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
                logger.error("Build form fails with error location for loop"_tr());
            }
        }

        fm.sendTo(player, [](Player& pl, CustomFormResult const& dt, FormCancelReason) {
            if (!dt) return sendMsg(pl, "form cancelled");
            auto& logger = tls::entry::getInstance().getSelf().getLogger();
            try {
                DebugFormCallBack(dt);

                auto level = ll::service::getLevel();
                if (!level) return sendMsg(pl, "Failed to get level Ptr"_tr());

                auto rules = level->getGameRules();

                for (auto [key, value] : *dt) {
                    try {
                        auto id = rules.nameToGameRuleIndex(key);
                        if (id.value == (int)GameRules::GameRulesIndex::InvalidGameRule || rules.hasRule(id) == false) {
                            logger.warn("Invalid game rule: {}"_tr(key));
                            continue;
                        }

                        bool*                      pValueValidated = nullptr;
                        bool*                      pValueChanged   = nullptr;
                        GameRule::ValidationError* errorOutput     = nullptr;

                        if (std::holds_alternative<uint64_t>(value)) {
                            // bool
                            int bl = std::get<uint64_t>(value);
                            rules.setRule(
                                id,
                                bl == 0 ? false : true,
                                false,
                                pValueValidated,
                                pValueChanged,
                                errorOutput
                            );
                        } else if (std::holds_alternative<string>(value)) {
                            // int or float
                            string str = std::get<string>(value);
                            if (isFloat(str)) {
                                float fl = std::stof(str);
                                rules.setRule(id, fl, false, pValueValidated, pValueChanged, errorOutput);
                            } else {
                                int in = std::stoi(str);
                                rules.setRule(id, in, false, pValueValidated, pValueChanged, errorOutput);
                            }
                        } else {
                            logger.warn("Invalid value type for game rule: {}"_tr(key));
                        }
                    } catch (...) {
                        logger.error("Error setting game rule: {}"_tr(key));
                    }
                }

                auto packet = rules.createAllGameRulesPacket();
                ll::service::getLevel()->forEachPlayer([&](Player& p) {
                    packet->sendTo(p);
                    return true;
                });
                sendMsg(pl, "Operation completed!"_tr());
            } catch (...) {
                logger.error("Error executing form callback"_tr());
            }
        });
    } catch (...) {
        logger.error(
            "Unknown error in function changeGameRule, ignore this error if the function is working properly"_tr()
        );
    }
}

} // namespace tls::form