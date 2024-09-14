#ifndef CAPSULE_SPIN_HANDLER_H
#define CAPSULE_SPIN_HANDLER_H

#include "../Network/MainSession.h"
#include "../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../Structures/Capsule/CapsuleSpin.h"
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
			SPIN_SUCCESS = 1,
			FAIL = 4,
			INVENTORY_FULL = 7,
			NOT_ENOUGH_CURRENCY = 14,
		};

		enum CapsuleSpinMission
		{
			LUCKY_SPIN = 0,
			NORMAL_SPIN = 1
		};

		inline std::pair<std::uint32_t, std::uint32_t> itemSelectionAlgorithm(const Main::ConstantDatabase::CdbUtil& cdbUtil, std::uint32_t gi_infoid,
			std::uint32_t gi_price, std::uint32_t gi_type) 
		{
			static std::array<double, 3> averageSpinCostByCurrency = cdbUtil.getAverageSpinCostByCurrency();
			auto items = cdbUtil.getAllEntriesWhereId(gi_infoid);

			constexpr const std::uint32_t maxProbability = 200'000;
			constexpr const double priceFactor = 0.2; 

			std::vector<std::pair<std::pair<std::uint32_t, std::uint32_t>, double>> itemProbabilities;
			for (const auto& item : items) 
			{
				double probability = static_cast<double>(item.gi_prob) / maxProbability * 100.0;
				if (gi_price > averageSpinCostByCurrency[gi_type] && item.gi_type == 1) // Increase chance for rare
				{
					probability *= (1.0 + priceFactor); 
				}
				else if (gi_price < averageSpinCostByCurrency[gi_type] && item.gi_type == 1) // decrease chance for rare
				{
					probability *= (1.0 - priceFactor); 
				}
				itemProbabilities.emplace_back(std::pair{ item.gi_itemid, item.gi_type }, probability);
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

		inline void removeCurrencyByCapsuleType(Main::Network::Session& session, const Main::Structures::AccountInfo& accountInfo, CapsuleCurrencyType capsuleCurrencyType,
			std::uint32_t toRemove)
		{
			if (capsuleCurrencyType == CapsuleCurrencyType::COINS)
			{
				//session.setAccountCoins(accountInfo.coins - toRemove);
			}
			else if (capsuleCurrencyType == CapsuleCurrencyType::ROCKTOTENS)
			{
				session.setAccountRockTotens(accountInfo.rockTotens - toRemove);
			}
			else
			{
				session.setAccountMicroPoints(accountInfo.microPoints - toRemove);
			}
		}

		inline void handleCapsuleSpin(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			struct ReceivedRequest
			{
				std::uint32_t capsuleId{};
				std::uint32_t currencySpent{};
				std::uint64_t unknown{}; 
			} receivedRequest;
			std::memcpy(&receivedRequest, request.getData(), sizeof(receivedRequest));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setMission(CapsuleSpinMission::NORMAL_SPIN);
			response.setExtra(CapsuleSpinExtra::SPIN_SUCCESS); 
			response.setOption(1); // number of items per spin (e.g 2 if we want to add addiitonal coupon)

			Main::ConstantDatabase::CdbUtil cdbUtil;
			const auto& capsuleInfo = cdbUtil.getCapsuleInfoById(receivedRequest.capsuleId);
			if (capsuleInfo == std::nullopt)
			{	
				response.setExtra(CapsuleSpinExtra::FAIL);
				response.setMission(1);
				session.asyncWrite(response);
				return;
			}
		
			const auto& accountInfo = session.getAccountInfo();

			if (capsuleInfo->gi_type == CapsuleCurrencyType::COINS && /*accountInfo.coins < capsuleInfo->gi_price * request.getOption()*/
				capsuleInfo->gi_type == CapsuleCurrencyType::ROCKTOTENS && accountInfo.rockTotens < capsuleInfo->gi_price * request.getOption()
				|| capsuleInfo->gi_type == CapsuleCurrencyType::MICROPOINTS && accountInfo.microPoints < capsuleInfo->gi_price * request.getOption())
			{
				response.setExtra(CapsuleSpinExtra::NOT_ENOUGH_CURRENCY);
				response.setOption(capsuleInfo->gi_type);
				session.asyncWrite(response);
				return;
			}

			if (!session.hasEnoughInventorySpace(request.getOption()))
			{
				response.setExtra(CapsuleSpinExtra::INVENTORY_FULL);
				session.asyncWrite(response);
				return;
			}

			constexpr const std::uint32_t maxLuckySpin = 1000;
			std::size_t i = 0;

			while (i < request.getOption())
			{
				Main::Structures::CapsuleSpin capsuleSpin;
				if (session.getLuckyPoints() > maxLuckySpin)
				{
					session.setLuckyPoints(0);
					response.setMission(CapsuleSpinMission::LUCKY_SPIN);
				}
				else
				{
					response.setMission(CapsuleSpinMission::NORMAL_SPIN);
				}
				capsuleSpin.itemSerialInfo.itemNumber = session.getLatestItemNumber() + 1;
				session.setLatestItemNumber(capsuleSpin.itemSerialInfo.itemNumber);
				const std::pair<std::uint32_t, std::uint32_t> wonItemIdAndType = itemSelectionAlgorithm(cdbUtil, capsuleInfo->gi_infoid, capsuleInfo->gi_price, capsuleInfo->gi_type);
				capsuleSpin.winItemId = wonItemIdAndType.first;
				cdbUtil.setItemId(capsuleSpin.winItemId);
				if (cdbUtil.getItemDurability() == std::nullopt)
				{
					response.setExtra(CapsuleSpinExtra::FAIL);
					response.setMission(1);
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
				removeCurrencyByCapsuleType(session, accountInfo, static_cast<CapsuleCurrencyType>(capsuleInfo->gi_type), capsuleInfo->gi_price);

				if (response.getMission() != 0)
				{
					session.addLuckyPoints(capsuleInfo->gi_luckypoint);
					++i;
				}
			}
		}
	}
}

#endif
