#ifndef CDB_UPGRADE_INFO_H
#define CDB_UPGRADE_INFO_H

#include "visit_struct/visit_struct.hpp"

namespace Common
{
	namespace ConstantDatabase
	{
#pragma pack(push, 1)
		struct CdbUpgradeInfo
		{
			int ui_id;
			int ui_type;
			int ui_itemid;
			int ui_parentid;
			int ui_group;
			int ui_prob;
			int ui_added_prob;
			int ui_hold_prob;
			int ui_buy_cash;
			int ui_buy_point; // total MP needed to upgrade to the given new upgraded itemID
			int ui_use_exp;   // total battery needed to upgrade to the given new upgraded itemID
			int ui_restore_cash;
			int ui_restore_point;
			char ui_desc[200];
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbUpgradeInfo, ui_id, ui_type, ui_itemid, ui_parentid, ui_group, ui_prob, ui_added_prob, ui_hold_prob, ui_buy_cash, ui_buy_point, ui_use_exp, ui_restore_cash, ui_restore_point, ui_desc);

#endif