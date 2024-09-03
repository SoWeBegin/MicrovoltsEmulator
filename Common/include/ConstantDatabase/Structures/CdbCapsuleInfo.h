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
			char gi_name[64];
			std::uint32_t gi_type; // capsule currency: coin(0), RT(1), MP(2)
			std::uint32_t gi_statetype;
			std::uint32_t gi_infoid;
			std::uint32_t gi_limited_grade;
			std::uint32_t gi_price;
			std::uint32_t gi_luckypoint;
			std::uint32_t gi_listicon;
			std::uint32_t gi_titleicon;
			char gi_desc[255];
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbCapsuleInfo, gi_id, gi_name, gi_type, gi_statetype, gi_infoid, gi_limited_grade, gi_price, gi_luckypoint, gi_listicon, gi_titleicon, gi_desc);

#endif