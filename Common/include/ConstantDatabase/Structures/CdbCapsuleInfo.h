#ifndef CGD_CAPSULE_INFO_H
#define CGD_CAPSULE_INFO_H

#include "visit_struct/visit_struct.hpp"

namespace Common
{
	namespace ConstantDatabase
	{
#pragma pack(push, 1)
		struct CdbCapsuleInfo
		{
			std::uint32_t gi_id;
			std::uint32_t gi_category;
			std::uint32_t gi_type; // capsule currency: coin(0), RT(1), MP(2)
			std::uint32_t gi_itemid;
			std::uint32_t gi_limited_grade;
			std::uint32_t gi_pay_coin;
			std::uint32_t gi_pay_cash;
			std::uint32_t gi_pay_point;
			std::uint32_t gi_bonus_lucky;
			std::uint32_t gi_accrue_lucky;
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbCapsuleInfo, gi_id, gi_category, gi_type, gi_itemid, gi_limited_grade, gi_pay_coin, gi_pay_cash, gi_pay_point, 
	gi_bonus_lucky, gi_accrue_lucky);

#endif