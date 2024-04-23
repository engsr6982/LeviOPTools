#include "File/Config.h"
#include "Motd/Motd.h"
#include "include_all.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/SimpleForm.h"


namespace tls::form {

enum MoveDirection {
    MoveUp   = 1,
    MoveDown = 0,
};
void moveVectorItem(MoveDirection direction, int index) {
    if (direction == MoveUp) {
        // 把下标为index的元素向上移动一位
        // 若index为0，则不移动
        if (index > 0) {
            std::swap(motd::motd_list[index], motd::motd_list[index - 1]);
        }
    } else if (direction == MoveDown) {
        // 把下标为index的元素向下移动一位
        // 若index为motd::motd_list.size()-1，则不移动
        if (index < motd::motd_list.size() - 1) {
            std::swap(motd::motd_list[index], motd::motd_list[index + 1]);
        }
    }
}

void showMotd(Player& player, int index);

void EditMotd(Player& player, int index) {

    CustomForm fm;
    fm.setTitle("LeviOPTools Motd Management Edit"_tr());

    fm.appendInput("value", "Input new motd content"_tr(), "string", motd::motd_list[index]);

    fm.sendTo(player, [index](Player& pl, CustomFormResult const& dt, FormCancelReason) {
        if (!dt) return sendMsg(pl, "Canceled"_tr());

        string value = std::get<string>(dt->at("value"));

        motd::motd_list[index] = value;

        showMotd(pl, index);
    });
}

void showMotd(Player& player, int index) {
    SimpleForm fm;
    fm.setTitle("LeviOPTools Motd Management"_tr());

    fm.setContent("Motd: {}"_tr(motd::motd_list[index]));

    fm.appendButton("Edit"_tr(), [index](Player& pl) { EditMotd(pl, index); });

    fm.appendButton("Move Up"_tr(), [index](Player&) { moveVectorItem(MoveUp, index); });

    fm.appendButton("Move Down"_tr(), [index](Player&) { moveVectorItem(MoveDown, index); });

    fm.appendButton("Delete"_tr(), [index](Player&) {
        motd::motd_list.erase(motd::motd_list.begin() + index);
        motd::saveMotd();
    });

    fm.appendButton("Return"_tr(), [](Player& pl) { motdManagement(pl); });

    fm.sendTo(player);
}


void motdManagement(Player& player) {
    AutoCheckPermission(player, perms::MotdManagement);

    SimpleForm fm;
    fm.setTitle("LeviOPTools Motd Management"_tr());

    fm.appendButton("Create New Line"_tr(), [](Player&) {
        motd::motd_list.push_back("New Line"_tr());
        motd::saveMotd();
    });

    for (int i = 0; i < motd::motd_list.size(); ++i) {
        fm.appendButton(motd::motd_list[i], [i](Player& pl) { showMotd(pl, i); });
    }

    fm.sendTo(player);
}

} // namespace tls::form