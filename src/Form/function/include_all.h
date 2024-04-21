#include "Api/Api.h"
#include "Entry/Entry.h"
#include "Entry/PluginInfo.h"
#include "Permission/Permission.h"
#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/PlayerInfo.h"
#include "ll/api/service/ServerInfo.h"
#include "mc/certificates/WebToken.h"
#include "mc/dataloadhelper/DataLoadHelper.h"
#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
#include "mc/entity/EntityIdTraits.h"
#include "mc/entity/gamerefs_entity/EntityRegistry.h"
#include "mc/entity/utilities/ActorDataIDs.h"
#include "mc/enums/BossBarColor.h"
#include "mc/enums/MinecraftPacketIds.h"
#include "mc/enums/ScorePacketType.h"
#include "mc/enums/TextPacketType.h"
#include "mc/enums/d_b_helpers/Category.h"
#include "mc/nbt/ListTag.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/NetworkIdentifier.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/BossEventPacket.h"
#include "mc/network/packet/LevelChunkPacket.h"
#include "mc/network/packet/ModalFormRequestPacket.h"
#include "mc/network/packet/RemoveObjectivePacket.h"
#include "mc/network/packet/ScorePacketInfo.h"
#include "mc/network/packet/SetDisplayObjectivePacket.h"
#include "mc/network/packet/SetScorePacket.h"
#include "mc/network/packet/SetTitlePacket.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/network/packet/ToastRequestPacket.h"
#include "mc/network/packet/TransferPacket.h"
#include "mc/network/packet/UpdateAbilitiesPacket.h"
#include "mc/network/packet/UpdateAdventureSettingsPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/world/ActorUniqueID.h"
#include "mc/world/Container.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/player/EnderChestContainer.h"
#include "mc/world/actor/player/PlayerScoreSetFunction.h"
#include "mc/world/actor/player/PlayerUISlot.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/events/BossEventUpdateType.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/LayeredAbilities.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/storage/DBStorage.h"
#include "mc/world/phys/HitResult.h"
#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/Scoreboard.h"
#include "mc/world/scores/ScoreboardId.h"
#include <algorithm>
#include <ll/api/form/CustomForm.h>
#include <ll/api/form/ModalForm.h>
#include <ll/api/form/SimpleForm.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/utils/HashUtils.h>
#include <mc/entity/EntityContext.h>
#include <mc/entity/utilities/ActorMobilityUtils.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/SimpleContainer.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/SynchedActorData.h>
#include <mc/world/actor/SynchedActorDataEntityWrapper.h>
#include <mc/world/actor/components/SynchedActorDataAccess.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/attribute/Attribute.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Command.h>
#include <mc/world/level/IConstBlockSource.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/scores/Objective.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace tls::form {

using namespace ll::form;
using string = std::string;
using ll::i18n_literals::operator""_tr;

void kickPlayer(Player& player);


// Helper functions

inline void sendMsg(Player& player, const std::string& msg) {
    player.sendMessage("§6[§a" + string(PLUGIN_NAME) + "§6]§b " + msg);
}

#define AutoCheckPermission(player, permission)                                                                        \
    {                                                                                                                  \
        if (perm::PermissionManager::getInstance()                                                                     \
                .getPermissionCore(PLUGIN_NAME)                                                                        \
                ->checkUserPermission(player.getUuid().asString().c_str(), permission)                                 \
            == false)                                                                                                  \
            return sendMsg(player, "No permissions, this function requires permission '{}'"_tr((int)permission));      \
    }

} // namespace tls::form