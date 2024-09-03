#include <cstdint>
#ifndef CDB_GRADEINFO_H
#define CDB_GRADEINFO_H

#include "visit_struct/visit_struct.hpp"

namespace Common
{
	namespace ConstantDatabase
	{
#pragma pack(push, 1)
		struct CdbGradeInfo
		{
			std::uint32_t gi_grade;
			char gi_name[16];
			std::uint32_t gi_exp;
			std::uint32_t gi_reward_point;
			std::uint32_t gi_reward_item;
			std::uint32_t gi_icon;
			std::uint32_t gi_pve_ticket;
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbGradeInfo, gi_grade, gi_name, gi_exp, gi_reward_point, gi_reward_item, gi_icon, gi_pve_ticket);

#endif