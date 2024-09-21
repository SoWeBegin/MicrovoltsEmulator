#ifndef BRILLIANT_BOX_H
#define BRILLIANT_BOX_H

#include <cstdint>
#include <unordered_map>
#include "../MainEnums.h"
#include "BoxBase.h"
#include <random>

namespace Main
{
	namespace Box
	{
		class BrilliantBox : public IBox
		{
		private:
			std::unordered_map<std::uint32_t, Main::Enums::BoxItemsCommon> commonItemProb
			{
				{2000, Main::Enums::BoxItemsCommon::MP_100},      // 20% 
				{1500, Main::Enums::BoxItemsCommon::MP_500},      // 15% 
				{1200, Main::Enums::BoxItemsCommon::MP_1000},     // 12% 
				{1000, Main::Enums::BoxItemsCommon::MP_2500},     //... etc.
				{800,  Main::Enums::BoxItemsCommon::MP_3000},
				{700,  Main::Enums::BoxItemsCommon::MP_4000},
				{600,  Main::Enums::BoxItemsCommon::MP_5000},
				{500,  Main::Enums::BoxItemsCommon::MP_7000},
				{450,  Main::Enums::BoxItemsCommon::MP_10000},
				{410,  Main::Enums::BoxItemsCommon::MP_20000},
				{100,    Main::Enums::BoxItemsCommon::MP_50000},
				{20,    Main::Enums::BoxItemsCommon::MP_100000},
				{10,    Main::Enums::BoxItemsCommon::MP_500000},
			};

		public:
		    explicit BrilliantBox(std::uint32_t rareItemId)
			{
				constexpr std::uint32_t rareItemProbability = 400;
				commonItemProb[rareItemProbability] = static_cast<Main::Enums::BoxItemsCommon>(rareItemId);
			}

			// Returns the random item that has been won after opening the box
			std::uint32_t get() override
			{
				std::vector<std::pair<std::uint32_t, Main::Enums::BoxItemsCommon>> cumulativeItems;
				std::uint32_t cumulativeSum = 0;

				for (const auto& [probability, itemID] : commonItemProb)
				{
					cumulativeSum += probability;
					cumulativeItems.emplace_back(cumulativeSum, itemID);
				}

				static std::random_device rd;
				static std::mt19937 gen(rd());
				static std::uniform_int_distribution<> dis(0, cumulativeSum - 1);
				std::uint32_t randomValue = dis(gen);

				auto it = std::lower_bound(
					cumulativeItems.begin(), cumulativeItems.end(),
					std::make_pair(randomValue, Main::Enums::BoxItemsCommon(0)),
					[](const auto& lhs, const auto& rhs)
					{
						return lhs.first < rhs.first;
					});

				return static_cast<std::uint32_t>(it->second);
			}
		};
	}
}
#endif