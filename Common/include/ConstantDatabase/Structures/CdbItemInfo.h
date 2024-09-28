#ifndef CDB_ITEM_INFO_H
#define CDB_ITEM_INFO_H

#include "visit_struct/visit_struct.hpp"

// This is used for MVSurge, refer to "CdbItemsInfo" for CMV
namespace Common
{
    namespace ConstantDatabase
    {
#pragma pack(push, 1) 
        struct CdbItemInfo
        {
            int ii_id;
            char ii_name[50];
            char ii_name_option[100];
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
            bool ii_class_f;
            bool ii_class_g;
            bool ii_class_h;
            bool ii_class_i;
            bool ii_class_j;
            bool ii_class_k;
            bool ii_class_l;
            bool ii_class_m;
            bool ii_class_n;
            bool ii_class_o;
            bool ii_class_p;
            int ii_grade;
            int ii_stocks;
            bool ii_usable;
            bool ii_upgradable;
            bool ii_consumable;
            int ii_weaponinfo;
            int ii_fullseteffectinfo;
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
            bool ii_immediately_set;
            bool ii_is_trade;
            int ii_ei_exp;
            bool ii_evolution_type;
        };

#pragma pack(pop)
    }
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbItemInfo, ii_id, ii_name, ii_name_option, ii_name_time, ii_type, ii_type_inven, ii_inven_usable, ii_type_pcbang, ii_package_result, ii_dress, ii_hide_hair, ii_hide_face, ii_hide_back, ii_class_a, ii_class_b,
    ii_class_c, ii_class_d, ii_class_e, ii_class_f, ii_class_g, ii_class_h, ii_class_i, ii_class_j, ii_class_k, ii_class_l, ii_class_m, ii_class_n, ii_class_o, ii_class_p, ii_grade, ii_stocks, ii_usable,
    ii_upgradable, ii_consumable, ii_weaponinfo, ii_fullseteffectinfo, ii_durable_value, ii_durable_factor, ii_durable_repair_type, ii_limited_mod, ii_limited_time, ii_buy_coupon, ii_buy_cash, ii_buy_point,
    ii_sell_point, ii_bonus_point, ii_bonus_pack, ii_dioramainfo, ii_dummyinfo, ii_icon, ii_iconsmall, ii_meshfilename, ii_nodename, ii_color_ambient, ii_color_diffuse, ii_color_specular, ii_color_emittance,
    ii_sfx, ef_effect_1, ef_target_1, ef_effect_2, ef_target_2, ef_effect_3, ef_target_3, ii_desc, ii_immediately_set, ii_is_trade, ii_ei_exp, ii_evolution_type
);


#endif