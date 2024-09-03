#ifndef CGD_CAPSULE_PACKAGE_INFO_H
#define CGD_CAPSULE_PACKAGE_INFO_H

#include "visit_struct/visit_struct.hpp"


namespace Common
{
	namespace ConstantDatabase
	{
#pragma pack(push, 1)
		struct CdbCapsulePackageInfo
		{
			int gi_id;
			int gi_infoid; // common with CdbCapsuleInfo.gi_infoid
			int gi_type;   // type 1 => rare unlimited, type 0 => not unlimited
			int gi_luckytype;
			int gi_group;
			int gi_prob;   // probability to win item
			int gi_itemid;
			int gi_noticeid;
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbCapsulePackageInfo, gi_id, gi_infoid, gi_type, gi_luckytype, gi_group, gi_prob, gi_itemid, gi_noticeid);

#endif