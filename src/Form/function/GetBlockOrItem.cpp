#include "include_all.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/SimpleForm.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/item/components/ComponentItem.h"
#include "mc/world/item/registry/ItemStack.h"

/*

file path: DataDir/ItemOrBlock.json

[
    {
        "name": string,
        "imageType": "url" | "path"
        "imageUrl": string,
        "type": string // minecraft item type
    }
]

 */

namespace tls::form {


struct JsonItem {
    std::string name;
    std::string imageType;
    std::string imageUrl;
    std::string type;
};

std::vector<JsonItem> loadJson() {
    std::vector<JsonItem> items;
    auto&                 mSelf  = tls::entry::getInstance().getSelf();
    auto&                 logger = mSelf.getLogger();
    try {
        auto path = mSelf.getDataDir() / "ItemOrBlock.json";
        if (std::filesystem::exists(path) == false) {
            logger.error("ItemOrBlock.json not found");
            return items;
        }
        // try load json
        std::ifstream ifs(path);
        if (ifs.is_open() == false) {
            logger.error("Failed to open ItemOrBlock.json");
            return items;
        }
        nlohmann::json j;
        ifs >> j;
        for (auto& item : j) {
            JsonItem jsonItem;
            jsonItem.name      = item["name"].get<std::string>();
            jsonItem.imageType = item["imageType"].get<std::string>();
            jsonItem.imageUrl  = item["imageUrl"].get<std::string>();
            jsonItem.type      = item["type"].get<std::string>();
            items.push_back(jsonItem);
        }

        ifs.close();
        return items;
    } catch (...) {
        logger.error("Failed to load ItemOrBlock.json");
        return items;
    }
};


void give(Player& player, string itemType, int count = 1, int lightLevel = 0) {
    try {
        // clang-format off
        #ifdef DEBUG
        tls::entry::getInstance().getSelf().getLogger().info("give | itemType: {}, count: {}, lightLevel: {}", itemType, count, lightLevel);
        #endif
        // clang-format on

        // check start with minecraft:
        if (!itemType.starts_with("minecraft:")) itemType = "minecraft:" + itemType;

        ItemStack* it = new ItemStack{itemType, count, lightLevel};

        if (!player.add(*it)) {
            player.drop(*it, false);
        }

        player.refreshInventory();
    } catch (...) {
        tls::entry::getInstance().getSelf().getLogger().error("Failed in give");
    }
}


void inputCount(Player& player, string itemType) {
    try {
        // clang-format off
        #ifdef DEBUG
        tls::entry::getInstance().getSelf().getLogger().info("inputCount | itemType: {}", itemType);
        #endif
        // clang-format on
        CustomForm fm;
        fm.setTitle("LeviOPTools - Input Count"_tr());

        fm.appendLabel("Select Input Count"_tr());

        // 数量
        // fm.appendInput("count", "Enter the required quantity", "integer", "1");
        fm.appendSlider("count", "Slide to select the amount to be given that has been selected: ", 1, 64);

        // 光照等级
        if (itemType == "light_block" || itemType == "minecraft:light_block") {
            fm.appendStepSlider(
                "level",
                "level: (0~15)\nSelected"_tr(),
                std::vector<
                    string>{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"}
            );
        }

        // 目标玩家
        std::vector<string> options;
        int                 currentPlayer = 0;
        ll::service::getLevel()->forEachPlayer([&options, &currentPlayer, &player](Player& pl) {
            options.push_back(pl.getRealName());
            if (pl.getRealName() == player.getRealName()) currentPlayer = options.size() - 1;
            return true;
        });
        fm.appendDropdown("target", "Selection of targets to be given"_tr(), options, currentPlayer);

        fm.sendTo(player, [itemType](Player& pl, CustomFormResult const& dt, FormCancelReason) {
            if (!dt) return Utils::sendMsg(pl, "Canceled"_tr());
            DebugFormCallBack(dt);

            // clang-format off
            #ifdef DEBUG
            tls::entry::getInstance().getSelf().getLogger().info("inputCount.lambda | itemType: {}", itemType);
            #endif
            // clang-format on

            // int count =  Utils::string2Int(std::get<string>(dt->at("count")));
            int count = std::get<double>(dt->at("count"));

            string targetStr = std::get<string>(dt->at("target"));

            Player* target = nullptr;

            if (targetStr == pl.getRealName()) target = &pl;
            else target = ll::service::getLevel()->getPlayer(targetStr);

            if (!target) return Utils::sendMsg(pl, "Invalid target"_tr());

            if (itemType == "light_block" || itemType == "minecraft:light_block") {
                int level = Utils::string2Int(std::get<string>(dt->at("level")));
                give(*target, itemType, count, level);
                return;
            }

            give(*target, itemType, count);
        });
    } catch (...) {
        tls::entry::getInstance().getSelf().getLogger().error("Failed in inputCount");
    }
}


void getBlockOrItem(Player& player) {
    AutoCheckPermission(player, perms::GetBlockOrItem);

    SimpleForm fm;
    fm.setTitle("LeviOPTools - Get Block or Item"_tr());

    fm.setContent("Click button to get block or item"_tr());

    auto items = loadJson();
    if (items.empty()) return Utils::sendMsg(player, "No item or block found"_tr());
    for (auto& it : items) {
        try {
            // clang-format off
            #ifdef DEBUG
            tls::entry::getInstance().getSelf().getLogger().info("Name: {}, Type: {}, ImageType: {}, ImageUrl: {}", it.name, it.type, it.imageType, it.imageUrl);
            #endif
            string itemType = it.type; // copy to local variable to avoid error
            if (it.imageType == "url" || it.imageType == "path") {
                fm.appendButton(it.name, it.imageUrl, it.imageType, [itemType](Player& pl) {
                    inputCount(pl, itemType);
                });
            } else {
                fm.appendButton(it.name, [itemType](Player& pl) { inputCount(pl, itemType); });
            }
        } catch (...) {
            tls::entry::getInstance().getSelf().getLogger().error("Failed to build button for item: {}", it.name);
        }
    }

    fm.sendTo(player);
}

} // namespace tls::form
