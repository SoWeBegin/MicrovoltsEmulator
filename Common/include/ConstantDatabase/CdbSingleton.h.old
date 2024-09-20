#ifndef CDB_SINGLETON_H
#define CDB_SINGLETON_H

#include "Cdb.h"
#include <string>
#include "Structures/CdbItemInfo.h"
#include "Structures/CdbWeaponsInfo.h"

namespace Common
{
    namespace ConstantDatabase
    {
        template <typename T>
        class CdbSingleton
        {
        private:
            inline static Cdb<T> m_cdb{};
            inline static std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> m_gambleItems; // [type][itemIds]
            inline static std::unordered_map<std::uint32_t, std::uint32_t> m_itemByType; // [itemId][Type]

        public:
            static Cdb<T> getInstance()
            {
                return m_cdb;
            }

            static const auto& getGambleItems()
            {
                return m_gambleItems;
            }

            static std::uint32_t getItemType(std::uint32_t itemId)
            {
                return m_itemByType[itemId];
            }

            static void initialize(const std::string& filePath, const std::string& fileName)
                requires std::same_as<T, Common::ConstantDatabase::CdbWeaponInfo>
            {
                m_cdb.parse(filePath, fileName);
                auto entries = m_cdb.getEntries();
                for (const auto& [id, structType] : entries)
                {
                    if ((strcmp(structType.ii_name_time, "Unlimited") == 0) && structType.ii_is_trade && structType.ii_upgradable)
                    {
                        m_gambleItems[structType.ii_type].push_back(static_cast<std::uint32_t>(structType.ii_id));
                    }
                }
            }

            static void initializeItemTypes(const std::string& filePath, const std::string& weaponFileName, const std::string& itemFileName)
                requires std::same_as<T, Common::ConstantDatabase::CdbItemInfo>
            {
                Cdb<CdbWeaponInfo> cdbWeapon{};
                cdbWeapon.parse(filePath, weaponFileName);
                auto entries = cdbWeapon.getEntries();
                for (const auto& [id, structType] : entries)
                {
                    if (m_itemByType.contains(structType.ii_id))
                    {
                        std::cout << "Item Already Contained in FirstLoop\n";
                    }
                    m_itemByType[structType.ii_id] = structType.ii_type;
                }

                Cdb<CdbItemInfo> cdbItem{};
                cdbItem.parse(filePath, itemFileName);
                auto entries2 = cdbItem.getEntries();
                for (const auto& [id, structType] : entries2)
                {
                    if (m_itemByType.contains(structType.ii_id))
                    {
                        std::cout << "Item Already Contained in SecondLoop\n";
                    }
                    m_itemByType[structType.ii_id] = structType.ii_type;
                }
            }

            static void initialize(const std::string& filePath, const std::string& fileName)
                requires std::same_as<T, Common::ConstantDatabase::CdbItemInfo>
            {
                m_cdb.parse(filePath, fileName);
                auto entries = m_cdb.getEntries();
                for (const auto& [id, structType] : entries)
                {
                    if ((strcmp(structType.ii_name_time, "Unlimited") == 0) && structType.ii_is_trade)
                    {
                        m_gambleItems[structType.ii_type].push_back(static_cast<std::uint32_t>(structType.ii_id));
                    }
                }
            }

            static void initialize(const std::string& filePath, const std::string& fileName)
                requires not std::same_as<T, Common::ConstantDatabase::CdbItemInfo> and not std::same_as<T, Common::ConstantDatabase::CdbWeaponInfo>
            {
                m_cdb.parse(filePath, fileName);
            }

            CdbSingleton(CdbSingleton const&) = delete;
            void operator=(CdbSingleton const&) = delete;

        private:
            CdbSingleton()
            {
            }
        };
    }
}
#endif
