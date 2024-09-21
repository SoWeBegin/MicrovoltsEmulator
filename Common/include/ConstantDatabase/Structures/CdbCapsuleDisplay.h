#ifndef CGD_CAPSULE_DISPLAY_H
#define CGD_CAPSULE_DISPLAY_H

#include "visit_struct/visit_struct.hpp"

namespace Common
{
	namespace ConstantDatabase
	{
#pragma pack(push, 1)
		struct CdbCapsuleDisplay
		{
			std::uint32_t gd_infoid;
			std::uint32_t gd_group;
			char gd_name[255];
			char gd_desc[255];
			std::uint32_t gd_type; // capsule currency: coin(0), RT(1), MP(2)
			std::uint32_t gd_set;
			std::uint32_t gd_hair; 
			std::uint32_t gd_face;
			std::uint32_t gd_upper;
			std::uint32_t gd_under;
			std::uint32_t gd_pants;
			std::uint32_t gd_arms;
			std::uint32_t gd_boots;
			std::uint32_t gd_acce_A;
			std::uint32_t gd_acce_B;
			std::uint32_t gd_acce_C;
			std::uint32_t gd_weapon;
			std::uint32_t gd_diorama;
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbCapsuleDisplay, gd_infoid, gd_group, gd_name, gd_desc, gd_type, gd_set, gd_hair, gd_face, gd_upper, gd_under, gd_pants,
	gd_arms, gd_boots, gd_acce_A, gd_acce_B, gd_acce_C, gd_weapon, gd_diorama);

#endif