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
			int pi_id;
			int pi_type;   // type 0 = Don't use (use only for lucky spins), type 1 = non-rare, type 2 = unlimited
			int pi_itemid; // useless
			int pi_group;
			int pi_prob;   // probability to win item
			int pi_valueA; // actual won itemId
			int pi_valueB;
			int pi_valueC;
		};
#pragma pack(pop)
	}
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbCapsulePackageInfo, pi_id, pi_type, pi_itemid, pi_group, pi_prob, pi_valueA, pi_valueB, pi_valueC);

#endif