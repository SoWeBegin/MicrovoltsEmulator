#ifndef CDB_ITEMSINFOS_STRUCT_H
#define CDB_ITEMSINFOS_STRUCT_H

#include "visit_struct/visit_struct.hpp"

// This is for CMV, equivalent to ItemInfo + WeaponInfo on Surge (for CMV they both were placed in a single file)
namespace Common
{
    namespace ConstantDatabase
    {
#pragma pack(push, 1) 
        struct CdbItemsInfoCMV
        {
            int ii_id;
            char ii_name[50];
            char ii_name_option[50];
            char ii_name_time[50];
            int ii_type;
            int ii_type_inven;
            bool ii_inven_usable;
            int ii_type_pcbang;
            int ii_package_result;
            bool ii_dress;
            bool ii_hide_hair;
            bool ii_hide_face;
            bool ii_hide_back;
            bool ii_class_a;
            bool ii_class_b;
            bool ii_class_c;
            bool ii_class_d;
            bool ii_class_e;
            int ii_grade;
            int ii_stocks;
            bool ii_usable;
            bool ii_upgradable;
            bool ii_consumable;
            int ii_weaponinfo;
            int ii_durable_value;
            int ii_durable_factor;
            int ii_durable_repair_type;
            bool ii_limited_mod;
            int ii_limited_time;
            int ii_buy_coupon;
            int ii_buy_cash;
            int ii_buy_point;
            int ii_sell_point;
            int ii_bonus_point;
            int ii_bonus_pack;
            int ii_dioramainfo;
            int ii_dummyinfo;
            int ii_icon;
            int ii_iconsmall;
            char ii_meshfilename[50];
            char ii_nodename[50];
            char ii_color_ambient[50];
            char ii_color_diffuse[50];
            char ii_color_specular[50];
            char ii_color_emittance[50];
            int ii_sfx;
            int ef_effect_1;
            int ef_target_1;
            int ef_effect_2;
            int ef_target_2;
            int ef_effect_3;
            int ef_target_3;
            char ii_desc[200];
        };

#pragma pack(pop)
    }
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbItemsInfoCMV, ii_id, ii_name, ii_name_option, ii_name_time, ii_type, ii_type_inven, ii_inven_usable, ii_type_pcbang, ii_package_result, ii_dress, ii_hide_hair, ii_hide_face, ii_hide_back, ii_class_a, ii_class_b,
    ii_class_c, ii_class_d, ii_class_e, ii_grade, ii_stocks, ii_usable,
    ii_upgradable, ii_consumable, ii_weaponinfo, ii_durable_value, ii_durable_factor, ii_durable_repair_type, ii_limited_mod, ii_limited_time, ii_buy_coupon, ii_buy_cash, ii_buy_point,
    ii_sell_point, ii_bonus_point, ii_bonus_pack, ii_dioramainfo, ii_dummyinfo, ii_icon, ii_iconsmall, ii_meshfilename, ii_nodename, ii_color_ambient, ii_color_diffuse, ii_color_specular, ii_color_emittance,
    ii_sfx, ef_effect_1, ef_target_1, ef_effect_2, ef_target_2, ef_effect_3, ef_target_3, ii_desc
);


#endif 