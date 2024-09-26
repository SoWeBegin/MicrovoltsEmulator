#ifndef CAPSULE_SPIN_HANDLER_H
#define CAPSULE_SPIN_HANDLER_H

#include "../Network/MainSession.h"
#include "../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../Structures/Capsule/CapsuleSpin.h"
#include "../Classes/CapsuleManager.h"
#include <random>

namespace Main
{
	namespace Handlers
	{
		enum CapsuleCurrencyType : std::uint32_t
		{
			COINS = 0,
			ROCKTOTENS = 1,
			MICROPOINTS = 2
		};

		enum CapsuleSpinExtra
		{
			INVENTORY_FULL = 7,
			NOT_ENOUGH_CURRENCY = 14,
		};

		enum CapsuleSpinMission
		{
			LUCKY_SPIN = 0,
			NORMAL_SPIN = 1
		};

		enum CapsuleSpinLuckyRewards
		{
			GoldLuck = 7304,
			SilverLuck = 7305,
			BronzeLuck = 7306,
			SemiLuck = 7307,
			NotSoLuck = 7308,
			ToughLuck = 7309
		};

		inline std::pair<std::uint32_t, std::uint32_t> itemSelectionAlgorithm(const Main::ConstantDatabase::CdbUtil& cdbUtil, std::uint32_t gi_itemid,
			std::uint32_t gi_price, std::uint32_t gi_type) 
		{
			static std::array<double, 3> averageSpinCostByCurrency = cdbUtil.getAverageSpinCostByCurrency();
			auto items = cdbUtil.getAllEntriesWhereId(gi_itemid);

			constexpr const std::uint32_t maxProbability = 200'000;
			constexpr const double priceFactor = 0.2; 

			std::vector<std::pair<std::pair<std::uint32_t, std::uint32_t>, double>> itemProbabilities;
			for (const auto& item : items) 
			{
				if (!item.pi_type) continue;
				double probability = static_cast<double>(item.pi_prob) / maxProbability * 100.0;
				if (gi_price > averageSpinCostByCurrency[gi_type] && item.pi_type == 2) // Increase chance for rare
				{
					probability *= (1.0 + priceFactor); 
				}
				else if (gi_price < averageSpinCostByCurrency[gi_type] && item.pi_type == 2) // decrease chance for rare
				{
					probability *= (1.0 - priceFactor); 
				}
				itemProbabilities.emplace_back(std::pair{ item.pi_valueA, item.pi_type }, probability);
			}

			std::vector<double> probabilities;
			for (const auto& pair : itemProbabilities) 
			{
				probabilities.push_back(pair.second);
			}

			std::discrete_distribution<int> itemDistribution(probabilities.begin(), probabilities.end());
			static std::random_device rd;
			static std::mt19937 gen(rd());
			int randomIndex = itemDistribution(gen);

			return itemProbabilities[randomIndex].first;
		}

		inline Common::ConstantDatabase::CdbCapsulePackageInfo luckySelectionAlgorithm(const ConstantDatabase::CdbUtil& cdbUtil, std::uint32_t gi_itemid,
			std::uint32_t gi_type, Classes::CapsuleManager capsuleManager)
		{
			auto items = cdbUtil.getAllEntriesWhereId(gi_itemid);
		
			constexpr const std::uint32_t maxProbability = 860400;
			std::vector<std::pair<Common::ConstantDatabase::CdbCapsulePackageInfo, double>> itemProbabilities;
			for (const auto& item : items)
			{
				double probability = static_cast<double>(item.pi_prob) / maxProbability * 100.0;
				itemProbabilities.emplace_back(item, probability);
			}
		
			std::vector<double> probabilities;
			for (const auto& pair : itemProbabilities)
			{
				probabilities.push_back(pair.second);
			}

			std::discrete_distribution<int> itemDistribution(probabilities.begin(), probabilities.end());
			static std::random_device rd;
			static std::mt19937 gen(rd());
			int randomIndex = itemDistribution(gen);
			for (int i = 0; i < itemProbabilities.size(); i++)
			{
				if (itemProbabilities[i].first.pi_id == SilverLuck)
				{
					randomIndex = i;
				}
			}
			return itemProbabilities[randomIndex].first;
		}

		inline void removeCurrencyByCapsuleType(Main::Network::Session& session, const Main::Structures::AccountInfo& accountInfo, CapsuleCurrencyType capsuleCurrencyType,
			std::uint32_t toRemove)
		{
			if (capsuleCurrencyType == CapsuleCurrencyType::ROCKTOTENS)
			{
				session.setAccountRockTotens(accountInfo.rockTotens - toRemove);
			}
			else
			{
				session.setAccountMicroPoints(accountInfo.microPoints - toRemove);
			}
		}

		inline void handleItemSpin(const Common::Network::Packet& request, Network::Session& session, Common::Network::Packet& response,
			ConstantDatabase::CdbUtil& cdbUtil, Common::ConstantDatabase::CdbCapsuleInfo capsuleInfo, Structures::AccountInfo accountInfo,
			Classes::CapsuleManager& capsuleManager)
		{
			if (!session.hasEnoughInventorySpace(request.getOption()))
			{
				response.setExtra(CapsuleSpinExtra::INVENTORY_FULL);
				session.asyncWrite(response);
				return;
			}

			Main::Structures::CapsuleSpin capsuleSpin;
			capsuleSpin.itemSerialInfo.itemNumber = session.getLatestItemNumber() + 1;
			session.setLatestItemNumber(capsuleSpin.itemSerialInfo.itemNumber);

			// Unsure how to handle this for CMV, currently only RT capsules are enabled, the rest are prices = 0
			// Here I'm assuming that to enable e.g. the MP capsule, we just need to 
			std::uint32_t price = 0;
			if (capsuleInfo.gi_type == CapsuleCurrencyType::ROCKTOTENS) // rt
			{
				price = capsuleInfo.gi_pay_cash;
			}
			else if (capsuleInfo.gi_type == CapsuleCurrencyType::MICROPOINTS) // mp
			{
				price = capsuleInfo.gi_pay_point;
			}

			const std::pair<std::uint32_t, std::uint32_t> wonItemIdAndType = itemSelectionAlgorithm(cdbUtil, capsuleInfo.gi_itemid, price, capsuleInfo.gi_type);
			capsuleSpin.winItemId = wonItemIdAndType.first;
			cdbUtil.setItemId(capsuleSpin.winItemId);

			if (cdbUtil.getItemDurability() == std::nullopt)
			{
				// Apparently there's no error message in this case in CMV, just resend the packet given by the client...
				session.asyncWrite(response);
				return;
			}
			Main::Structures::Item item;
			item.durability = *cdbUtil.getItemDurability();
			item.expirationDate = *cdbUtil.getItemDuration();
			item.serialInfo = capsuleSpin.itemSerialInfo;
			item.id = capsuleSpin.winItemId;

			response.setData(reinterpret_cast<std::uint8_t*>(&capsuleSpin), sizeof(Main::Structures::CapsuleSpin));
			session.asyncWrite(response);
			session.addItem(item);
			removeCurrencyByCapsuleType(session, accountInfo, static_cast<CapsuleCurrencyType>(capsuleInfo.gi_type), price);
			capsuleManager.addJackpot(400);

			if (response.getMission() != 0)
			{
				session.addLuckyPoints(capsuleInfo.gi_bonus_lucky);
			}
		}

		inline void handleLuckySpin(const Common::Network::Packet& request, Network::Session& session, Common::Network::Packet& response,
			ConstantDatabase::CdbUtil& cdbUtil, Common::ConstantDatabase::CdbCapsuleInfo capsuleInfo, Structures::AccountInfo accountInfo,
			Classes::CapsuleManager& capsuleManager, Network::SessionsManager sessionsManager)
		{
			const auto selectedItem = luckySelectionAlgorithm(cdbUtil, capsuleInfo.gi_itemid, capsuleInfo.gi_type, capsuleManager);

			std::uint32_t mpAmountToReturn;
			std::uint32_t mpAmountToAdd;
			auto const jackpotValue = capsuleManager.getJackpot();
			std::string winMessage;

			switch (selectedItem.pi_id)
			{
				case GoldLuck:
					mpAmountToReturn = jackpotValue;
					mpAmountToAdd = jackpotValue;
					winMessage = std::string("[") + session.getPlayerName() + "] won a Gold Luck Charm and earned " + std::to_string(mpAmountToAdd) + " MP!";
					break;
				case SilverLuck:
					mpAmountToReturn = selectedItem.pi_valueA * 1000; // % of the jackpot we won, 300000 = 30%, pi_valueA = 300
					mpAmountToAdd = jackpotValue * (selectedItem.pi_valueA / 10) / 100; // 30% of the jackpot
					winMessage = std::string("[") + session.getPlayerName() + "] won a Silver Luck Charm and earned " + std::to_string(mpAmountToAdd) + " MP!";
					break;
				case BronzeLuck:
					mpAmountToReturn = jackpotValue; // 10% of this value results in the MP amount we get
					mpAmountToAdd = jackpotValue * (selectedItem.pi_valueA / 10) / 100; // 10% of the jackpot
					winMessage = std::string("[") + session.getPlayerName() + "] won a Copper Luck Charm and earned " + std::to_string(mpAmountToAdd) + " MP!";
					break;
				case SemiLuck:
				case NotSoLuck:
				case ToughLuck:
					mpAmountToReturn = selectedItem.pi_valueB;
					mpAmountToAdd = selectedItem.pi_valueB;
					break;
			}

			Structures::CapsuleSpin capsuleSpin;
			capsuleSpin.winItemId = selectedItem.pi_id;
			capsuleSpin.mpAmount = mpAmountToReturn;

			session.setAccountMicroPoints(accountInfo.microPoints + mpAmountToAdd);
			capsuleManager.subJackpot(mpAmountToAdd);

			response.setExtra(37);
			response.setData(reinterpret_cast<std::uint8_t*>(&capsuleSpin), sizeof(Main::Structures::CapsuleSpin));
			session.asyncWrite(response);
			session.setLuckyPoints(0);
			sessionsManager.broadcastMessageExceptSelf(session.getSessionId(), winMessage);
		}

		inline void handleCapsuleSpin(const Common::Network::Packet& request, Main::Network::Session& session, Classes::CapsuleManager& capsuleManager, Network::SessionsManager sessionsManager)
		{
			auto response = request;
			response.setOption(1); // number of items per spin (e.g 2 if we want to add addiitonal coupon)

			Main::ConstantDatabase::CdbUtil cdbUtil;
			const auto& capsuleInfo = cdbUtil.getCapsuleInfoById(request.getOption());
			if (capsuleInfo == std::nullopt)
			{
				// Apparently there's no error message in this case in CMV, just resend the packet given by the client...
				session.asyncWrite(response);
				return;
			}

			const auto& accountInfo = session.getAccountInfo();

			if (capsuleInfo->gi_type == CapsuleCurrencyType::COINS &&
				capsuleInfo->gi_type == CapsuleCurrencyType::ROCKTOTENS && accountInfo.rockTotens < capsuleInfo->gi_pay_cash * request.getOption()
				|| capsuleInfo->gi_type == CapsuleCurrencyType::MICROPOINTS && accountInfo.microPoints < capsuleInfo->gi_pay_point * request.getOption())
			{
				response.setExtra(CapsuleSpinExtra::NOT_ENOUGH_CURRENCY);
				response.setOption(capsuleInfo->gi_type);
				session.asyncWrite(response);
				return;
			}

			if (session.getLuckyPoints() >= 1000)
			{
				handleLuckySpin(request, session, response, cdbUtil, capsuleInfo.value(), accountInfo, capsuleManager, sessionsManager);
			}
			else
			{
				handleItemSpin(request, session, response, cdbUtil, capsuleInfo.value(), accountInfo, capsuleManager);
			}
		}
	}
}

#endif
