#ifndef CDB_SINGLETON_H
#define CDB_SINGLETON_H

#include "Cdb.h"
#include <string>
#include "Structures/CdbItemInfo.h"
#include "Structures/CdbItemsInfo.h"
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
            inline static std::unordered_map<std::uint32_t, std::uint32_t> m_itemByType; // [itemId][Type]

        public:
            static Cdb<T> getInstance()
            {
                return m_cdb;
            }

            static std::uint32_t getItemType(std::uint32_t itemId)
            {
                return m_itemByType[itemId];
            }


            static void initializeItemTypes(const std::string& filePath, const std::string& weaponFileName, const std::string& itemFileName)
                requires std::same_as<T, Common::ConstantDatabase::CdbItemsInfoCMV>
            {
                /*
                Cdb<CdbWeaponInfo> cdbWeapon{};
                cdbWeapon.parse(filePath, weaponFileName);
                auto entries = cdbWeapon.getEntries();
                for (const auto& [id, structType] : entries)
                {
                    m_itemByType[structType.ii_id] = structType.ii_type;
                }*/

                Cdb<Common::ConstantDatabase::CdbItemsInfoCMV> cdbItem{};
                cdbItem.parse(filePath, itemFileName);
                auto entries = cdbItem.getEntries();
                for (const auto& [id, structType] : entries)
                {
                    m_itemByType[structType.ii_id] = structType.ii_type;
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
