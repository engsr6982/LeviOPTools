#include "Config/Config.h"
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
            std::filesystem::path path =
                mSelf.getModDir() / tls::Config::cfg.function.gameRuleTranslatFilePath; // json file
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
                return string(rule);
            }
        }

        if (trList.find(rule) != trList.end()) {
            return trList[rule];
        }
        logger.warn("Untranslated game rules: {}"_tr(rule));
        return string(rule);
    } catch (const nlohmann::json::parse_error& e) {
        logger.error("GameRule translation file parse error: {}"_tr(e.what()));
        return string(rule);
    } catch (...) {
        logger.error("Unknown error in GameRule translation!"_tr());
        return string(rule);
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
        if (level.has_value() == false) return Utils::sendMsg(player, "Failed to get level Ptr"_tr());

        // Get the game rules
        GameRules&                     rules = level->getGameRules();
        const GameRules::GameRuleList& list  = rules.getRules();

        // Sort the list by bool rules first
        // std::sort(list.begin(), list.end(), [](const GameRule& a, const GameRule& b) {
        //     return a.mType == GameRule::Type::Bool && b.mType != GameRule::Type::Bool;
        // });

        // build the form
        for (const GameRule& rule : list) {
            try {
                if (rule.mType == GameRule::Type::Bool) {
                    fm.appendToggle( // bool => toggle
                        string(rule.mName),
                        getTr(rule.mName),
                        rule.getBool()
                    );
                } else if (rule.mType == GameRule::Type::Int) {
                    fm.appendInput( // int => input
                        string(rule.mName),
                        getTr(rule.mName),
                        "int",
                        std::to_string(rule.getInt())
                    );
                } else { // unknown type => ignore
                    logger.warn("Unparsable game rules: {}"_tr(rule.getName()));
                }
            } catch (...) {
                logger.error("Build form fails with error location for loop"_tr());
            }
        }

        fm.sendTo(player, [](Player& pl, CustomFormResult const& dt, FormCancelReason) {
            if (!dt) return Utils::sendMsg(pl, "form cancelled");
            auto& logger = tls::entry::getInstance().getSelf().getLogger();
            try {
                DebugFormCallBack(dt);

                // 由于未知问题，这里无法正常工作，暂时注释掉（错误原因：访问冲突）
                /* for (auto [key, value] : *dt) {
                    try {
                        const GameRuleId cid = ll::service::getLevel()->getGameRules().nameToGameRuleIndex(key);
                        GameRuleId&      id  = const_cast<GameRuleId&>(cid);
                        if (id.value == static_cast<int>(GameRules::GameRulesIndex::InvalidGameRule)
                            || ll::service::getLevel()->getGameRules().hasRule(id) == false) {
                            logger.warn("Invalid game rule: {}"_tr(key));
                            continue;
                        }

                        bool*                      pValueValidated = nullptr;
                        bool*                      pValueChanged   = nullptr;
                        GameRule::ValidationError* errorOutput     = nullptr;

                        if (std::holds_alternative<uint64_t>(value)) {
                            // bool
                            int bl = std::get<uint64_t>(value);
                            ll::service::getLevel()->getGameRules().setRule(
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
                                float fl =  Utils::string2Float(str);
                                ll::service::getLevel()
                                    ->getGameRules()
                                    .setRule(id, fl, false, pValueValidated, pValueChanged, errorOutput);
                            } else {
                                int in =  Utils::string2Int(str);
                                ll::service::getLevel()
                                    ->getGameRules()
                                    .setRule(id, in, false, pValueValidated, pValueChanged, errorOutput);
                            }
                        } else {
                            logger.warn("Invalid value type for game rule: {}"_tr(key));
                        }

                        delete pValueValidated;
                        delete pValueChanged;
                        delete errorOutput;
                    } catch (...) {
                        logger.error("Error setting game rule: {}"_tr(key));
                    }
                } */

                // 备用方案：遍历规则列表，逐个设置
                const auto& list = ll::service::getLevel()->getGameRules().getRules();

                for (auto& rule : list) {
                    try {
                        if (dt->find(rule.mName) != dt->end()) {

                            bool*                      pValidated  = new bool(false);
                            GameRule::ValidationError* errorOutput = new GameRule::ValidationError();
                            errorOutput->mSuccess                  = false;

                            if (std::holds_alternative<uint64_t>(dt->at(rule.mName))) {
                                // bool
                                bool bl = std::get<uint64_t>(dt->at(rule.mName)) == 0 ? false : true;
                                if (rule.getBool() == bl) continue;
                                GameRule& r = const_cast<GameRule&>(rule);
                                r.setBool(bl, pValidated, errorOutput);
                            } else if (std::holds_alternative<string>(dt->at(rule.mName))) {
                                // int
                                string str = std::get<string>(dt->at(rule.mName));
                                int    in  = Utils::string2Int(str);
                                // clang-format off
                                #ifdef DEBUG
                                logger.warn("[Debug] rule: {}, string value: {}"_tr(getTr(rule.getName()), str));
                                logger.warn("[Debug] rule: {}, int value: {}"_tr(getTr(rule.getName()), in));
                                logger.warn("[Debug] rule: {}, is changed: {}"_tr(getTr(rule.getName()), rule.getInt() != in));
                                #endif
                                // clang-format on
                                if (rule.getInt() == in) continue;
                                GameRule& r = const_cast<GameRule&>(rule);
                                r.setInt(in, pValidated, errorOutput);
                            } else {
                                logger.warn("Invalid value type for game rule: {}"_tr(getTr(rule.getName())));
                            }

                            // error processing
                            if (pValidated == 0) {
                                logger.warn("[McAPI] Game rule '{}' validation failed, return value: '{}'"_tr(
                                    getTr(rule.getName()),
                                    bool(pValidated)
                                ));
                            }
                            if (errorOutput->mSuccess == false)
                                logger.warn("[McAPI] Failed to modify game rules '{}', return value: '{}'"_tr(
                                    getTr(rule.getName()),
                                    errorOutput->mSuccess
                                ));
                            if (errorOutput->mErrorDescription.empty() == false)
                                logger.warn("[McAPI] Rules of the game: '{}', Description of the error: '{}'"_tr(
                                    getTr(rule.getName()),
                                    errorOutput->mErrorDescription
                                ));
                            if (errorOutput->mErrorParameters.empty() == false)
                                logger.warn("[McAPI] Game rules: '{}', error parameters: '{}'"_tr(
                                    getTr(rule.getName()),
                                    Utils::join(errorOutput->mErrorParameters)
                                ));

                            // destroy pointers
                            delete pValidated;
                            delete errorOutput;
                        }
                    } catch (...) {
                        logger.error("Error setting game rule: {}"_tr(getTr(rule.getName())));
                    }
                }

                auto packet = ll::service::getLevel()->getGameRules().createAllGameRulesPacket();
                ll::service::getLevel()->forEachPlayer([&](Player& p) {
                    packet->sendTo(p);
                    return true;
                });
                Utils::sendMsg(pl, "Operation completed!"_tr());
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