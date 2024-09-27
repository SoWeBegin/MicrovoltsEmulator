#ifndef CDB_UTILITY_H
#define CDB_UTILITY_H

#include <cstdint>
#include <optional>
#include <array>
#include <functional>

#include "../include/MainEnums.h"
#include "ConstantDatabase/CdbSingleton.h"
#include "ConstantDatabase/Structures/CdbItemInfo.h"
#include "ConstantDatabase/Structures/CdbWeaponsInfo.h"
#include "ConstantDatabase/Structures/CdbUpgradeInfo.h"
#include "ConstantDatabase/Structures/CdbCapsuleInfo.h"
#include "ConstantDatabase/Structures/CdbCapsulePackageInfo.h"
#include "ConstantDatabase/Structures/CdbCapsuleDisplay.h"
#include "ConstantDatabase/Structures/CdbMapInfo.h"
#include "Structures/Capsule/CapsuleList.h"
#include <ConstantDatabase/Structures/CdbRewardInfo.h>
#include <ConstantDatabase/Structures/CdbCapsuleDisplay.h>
#include <ConstantDatabase/Structures/CdbGradeInfo.h>

namespace Main
{
	namespace ConstantDatabase
	{
		class CdbUtil
		{
		private:
			std::uint32_t itemId{};
			std::uint32_t m_maxCapsuleItems{};

			using cdbItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbItemInfo>;
			using cdbWeapons = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbWeaponInfo>;
			using cdbUpgrades = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbUpgradeInfo>;
			using cdbCapsuleInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsuleInfo>;
			using cdbCapsulePackageInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsulePackageInfo>;
			using cdbCapsuleDisplay = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsuleDisplay>;
			using cdbMapInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbMapInfo>;
			using cdbRewardInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbRewardInfo>;
			using ItemTypePricePair = std::pair<Main::Enums::ItemCurrencyType, std::uint32_t>;
			using cdbLevelInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbGradeInfo>;

		public:
			explicit CdbUtil(std::uint32_t itemId)
				: itemId{ itemId }
			{
			}

			CdbUtil() = default;

			static std::uint32_t getItemType(std::uint32_t itemId)
			{
				return cdbItems::getItemType(itemId);
			}

			void setItemId(std::uint32_t itemid)
			{
				itemId = itemid;
			}

			std::optional<std::uint32_t> getMapDeathHeight() const
			{
				const auto mapInfoMap = cdbMapInfo::getInstance().getEntry(itemId);
				if (mapInfoMap == std::nullopt) return std::nullopt;
				return mapInfoMap.value().mi_death_height;
			}

			std::optional<ItemTypePricePair> getItemPrice() const
			{
				const auto itemInfoMapPrice = getItemPriceInternal(cdbItems::getInstance());
				if (itemInfoMapPrice == std::nullopt)
				{
					return getItemPriceInternal(cdbWeapons::getInstance());
				}
				return itemInfoMapPrice;
			}

			std::optional<std::uint16_t> getItemDurability() const
			{
				const auto itemDurability = getDurabilityInternal(cdbItems::getInstance());
				if (itemDurability == std::nullopt)
				{
					return getDurabilityInternal(cdbWeapons::getInstance());
				}
				return itemDurability;
			}

			std::optional<std::uint16_t> getItemRefundPrice() const
			{
				const auto refundValue = getRefundValue(cdbItems::getInstance());
				if (refundValue == std::nullopt)
				{
					return getRefundValue(cdbWeapons::getInstance());
				}
				return refundValue;
			}

			std::optional<Common::ConstantDatabase::CdbRewardInfo> getRewardInfoForMode(std::uint32_t mode) const
			{
				auto entry = cdbRewardInfo::getInstance().getEntry("ri_mod", mode);
				if (entry == std::nullopt)
				{
					return std::nullopt;
				}
				return entry;
			}

			std::optional<Common::ConstantDatabase::CdbGradeInfo> getGradeInfoForLevel(std::uint32_t level) const
			{
				auto entry = cdbLevelInfo::getInstance().getEntry("gi_grade", level);
				if (entry == std::nullopt)
				{
					return std::nullopt;
				}
				return entry;
			}

			std::optional<bool> isImmediatelySet() const
			{
				const auto isItemImmediatelySet = isImmediatelySetInternal(cdbItems::getInstance());
				if (isItemImmediatelySet == std::nullopt)
				{
					return isImmediatelySetInternal(cdbWeapons::getInstance());
				}
				return isItemImmediatelySet;
			}

			std::optional<std::uint32_t> getItemDuration() const
			{
				const auto itemInfoMapExpiration = getItemDurationInternal(cdbItems::getInstance());
				if (itemInfoMapExpiration == std::nullopt)
				{
					return getItemDurationInternal(cdbWeapons::getInstance());
				}
				return itemInfoMapExpiration;
			}

			std::optional<std::uint32_t> getItemType() const
			{
				const auto itemType = getItemTypeInternal(cdbItems::getInstance());
				if (itemType == std::nullopt)
				{
					return getItemTypeInternal(cdbWeapons::getInstance());
				}
				return itemType;
			}

			std::optional<std::uint32_t> getBatteryNeededForUpgrade() const
			{
				auto entry = cdbUpgrades::getInstance().getEntry("ui_itemid", itemId);
				if (entry == std::nullopt)
				{
					return std::nullopt;
				}
				return entry->ui_use_exp;
			}

			std::optional<std::uint32_t> getMpNeededForUpgrade() const
			{
				auto entry = cdbUpgrades::getInstance().getEntry("ui_itemid", itemId);
				if (entry == std::nullopt)
				{
					return std::nullopt;
				}
				return entry->ui_buy_point;
			}

			std::optional<bool> hasParentId() const
			{
				auto entry = cdbUpgrades::getInstance().getEntry("ui_itemid", itemId);
				if (entry == std::nullopt)
				{
					return std::nullopt;
				}				
				return entry->ui_parentid;
			}

			std::optional<Common::ConstantDatabase::CdbCapsuleInfo> getCapsuleInfoById(std::uint32_t gi_id) const
			{
				const auto& entries = cdbCapsuleInfos::getInstance().getEntries();
				for (const auto& [entryId, capsuleInfoStruct] : entries)
				{
					if (capsuleInfoStruct.gi_id == gi_id)
					{
						return capsuleInfoStruct;
					}
				}
				return std::nullopt;
			}

			std::array<double, 3> getAverageSpinCostByCurrency() const
			{
				std::array<double, 3> averageCosts{}; // [0]=coin, [1]=rt, [2]=mp
				std::array<double, 3> totalCapsuleTypesByCurrency{};  // [0]=coin, [1]=rt, [2]=mp
				for (std::size_t total = 0; const auto & [id, capsuleInfoStruct] : cdbCapsuleInfos::getInstance().getEntries())
				{
					if (total == m_maxCapsuleItems) break;
					++totalCapsuleTypesByCurrency[capsuleInfoStruct.gi_type];
				}
				averageCosts[0] /= totalCapsuleTypesByCurrency[0];
				averageCosts[1] /= totalCapsuleTypesByCurrency[1];
				averageCosts[2] /= totalCapsuleTypesByCurrency[2];

				return averageCosts;
			}

			auto getAllEntriesWhereIdSeparated(std::uint32_t id) const
				-> std::pair<std::vector<Common::ConstantDatabase::CdbCapsulePackageInfo>, std::vector<Common::ConstantDatabase::CdbCapsulePackageInfo>>
			{
				const auto& entries = cdbCapsulePackageInfos::getInstance().getEntries();
				std::vector<Common::ConstantDatabase::CdbCapsulePackageInfo> rares;
				std::vector<Common::ConstantDatabase::CdbCapsulePackageInfo> notRares;

				for (const auto& [entryId, capsulePackageInfoStruct] : entries)
				{
					if (capsulePackageInfoStruct.pi_id == id && capsulePackageInfoStruct.pi_type == 2)
					{
						rares.push_back(capsulePackageInfoStruct);
					}
					else if (capsulePackageInfoStruct.pi_id == id && capsulePackageInfoStruct.pi_type == 1)
					{
						notRares.push_back(capsulePackageInfoStruct);
					}
				}

				return { rares, notRares };
			}

			auto getAllEntriesWhereId(std::uint32_t itemid) const
				-> std::vector<Common::ConstantDatabase::CdbCapsulePackageInfo>
			{
				const auto& entries = cdbCapsulePackageInfos::getInstance().getEntries();
				std::vector<Common::ConstantDatabase::CdbCapsulePackageInfo> items;

				for (const auto& [entryId, capsulePackageInfoStruct] : entries)
				{
					if (capsulePackageInfoStruct.pi_itemid == itemid)
					{
						items.push_back(capsulePackageInfoStruct);
					}
				}

				return items;
			}

		private:
			template<typename T>
			constexpr std::optional<std::uint32_t> getItemTypeInternal(Common::ConstantDatabase::Cdb<T> cdb) const
			{
				const std::optional<T> entry = cdb.getEntry("ii_id", itemId);
				if (entry == std::nullopt)
				{
					return std::nullopt;
				}
				return entry->ii_type;
			}

			template<typename T>
			std::optional<ItemTypePricePair> getItemPriceInternal(const Common::ConstantDatabase::Cdb<T>& cdb) const
			{

				const std::optional<T> entry = cdb.getEntry("ii_id", itemId);
				if (entry == std::nullopt) return std::nullopt;
				T foundEntry = entry.value();

				std::int32_t coupons = foundEntry.ii_buy_coupon;
				if (coupons == 0)
				{
					std::int32_t rt = foundEntry.ii_buy_cash;
					if (rt == 0)
					{
						std::int32_t mp = foundEntry.ii_buy_point;
						if (mp == 0) return std::nullopt;
						return std::pair{ Main::Enums::ItemCurrencyType::ITEM_MP, mp };
					}
					return std::pair{ Main::Enums::ItemCurrencyType::ITEM_RT, rt };
				}
				return std::pair{ Main::Enums::ItemCurrencyType::ITEM_COUPON, coupons };
			}

			template<typename T>
			std::optional<std::uint32_t> getItemDurationInternal(const Common::ConstantDatabase::Cdb<T>& cdb) const
			{
				const std::optional<T> entry = cdb.getEntry("ii_id", itemId);
				if (entry == std::nullopt) return std::nullopt;
				return entry->ii_limited_time;
			}

			template<typename T>
			std::optional<bool> isImmediatelySetInternal(const Common::ConstantDatabase::Cdb<T>& cdb) const
			{
				const std::optional<T> entry = cdb.getEntry("ii_id", itemId);
				if (entry == std::nullopt) return std::nullopt;
				return entry->ii_immediately_set;
			}

			template<typename T>
			std::optional<std::uint16_t> getDurabilityInternal(const Common::ConstantDatabase::Cdb<T>& cdb) const
			{
				const std::optional<T> entry = cdb.getEntry("ii_id", itemId);
				if (entry == std::nullopt) return std::nullopt;
				return entry->ii_durable_value;
			}

			template<typename T>
			std::optional<std::uint16_t> getRefundValue(const Common::ConstantDatabase::Cdb<T>& cdb) const
			{
				const std::optional<T> entry = cdb.getEntry("ii_id", itemId);
				if (entry == std::nullopt) return std::nullopt;
				return entry->ii_sell_point;
			}
		};
	}
}


#endif
