#ifndef CGD_SET_ITEM_INFO_H
#define CGD_SET_ITEM_INFO_H

#include "visit_struct/visit_struct.hpp"


namespace Common
{
	namespace ConstantDatabase
	{
#pragma pack(push,1)
		struct SetItemInfo
		{
			std::uint32_t si_id;
			std::uint32_t si_hair;
			std::uint32_t si_face;
			std::uint32_t si_top;
			std::uint32_t si_under;
			std::uint32_t si_pants;
			std::uint32_t si_arms;
			std::uint32_t si_boots;
			std::uint32_t si_acce_A;
			std::uint32_t si_acce_B;
			std::uint32_t si_acce_C;
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::SetItemInfo, si_id, si_hair, si_face, si_top, si_under, si_pants, si_arms, si_boots, si_acce_A, si_acce_B, si_acce_C);


#endif