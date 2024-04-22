#pragma once
#include <PermissionCore/Group.h>
#include <PermissionCore/PermissionCore.h>
#include <PermissionCore/PermissionManager.h>
#include <PermissionCore/PermissionRegister.h>


namespace tls::perms {

enum Permission {
    Unknown                  = 0,  // 未知
    indexForm                = 1,  // 首页
    KickPlayer               = 2,  // 踢出玩家
    KillPlayer               = 3,  // 杀死玩家
    ChangeWeather            = 4,  // 更改天气
    ChangeTime               = 5,  // 更改时间
    BroadCastMessage         = 6,  // 广播消息
    MotdManagement           = 7,  // MOTD管理
    SetServerMaxPlayer       = 8,  // 设置服务器最大玩家数
    Teleport                 = 9,  // 传送
    CleanDropItem            = 10, // 清理掉落物品
    ChangeGameMode           = 11, // 更改游戏模式
    ChangeGameRule           = 12, // 更改世界规则
    GetBlockOrItem           = 13, // 获取方块或物品
    Terminal                 = 14, // 终端
    SendMessageToPlayer      = 15, // 发送消息给玩家
    CrashPlayerClient        = 16, // 崩溃客户端
    UsePlayerIdentitySay     = 17, // 使用玩家身份说话
    UsePlayerIdentityExecute = 18, // 使用玩家身份执行命令
    BanGui                   = 19, // 封禁GUI
    PlayerInfo               = 20, // 玩家信息
    CommandBlacklist         = 21, // 命令黑名单
    PotionGUI                = 22, // 药水GUI
    ChunkOpertion            = 23, // 区块操作
};

void initPermission();

} // namespace tls::perms