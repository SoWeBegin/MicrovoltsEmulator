

#include <string>
#include <charconv>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/SpawnItem.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNumericalCommand.h"
#include "../../../../include/Structures/Item/SpawnedItem.h"


namespace Main
{
	namespace Command
	{
			SpawnItem::SpawnItem(const Common::Enums::PlayerGrade requiredGrade)
				: AbstractOneArgNumericalCommand{ requiredGrade }
			{
			}

			void SpawnItem::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response) 
			{
				if (!parseCommand(providedCommand))
				{
					this->m_confirmationMessage += "error: parsing error";
					sendConfirmation(response, session);
					return;
				}
				response.setOrder(66);
				response.setExtra(51);
				response.setOption(2);
				Main::Structures::SpawnedItem spawnedItem{};
				Main::ConstantDatabase::CdbUtil cdbUtil(m_value);
				if (cdbUtil.getItemDurability() == std::nullopt)
				{
					this->m_confirmationMessage += "item not found.";
					sendConfirmation(response, session);
					return;
				}
				this->m_confirmationMessage += "success";
				spawnedItem.itemId = m_value;
				spawnedItem.serialInfo.itemNumber = session.getLatestItemNumber() + 1;
				
				auto opt = cdbUtil.getItemDuration();
				if (opt == std::nullopt)
				{
					this->m_confirmationMessage += "error: failed to retrieve item duration from cdb files!";
				}
				else
				{
					if (*opt <= 2)
					{
						spawnedItem.expirationDate = static_cast<__time32_t>(*opt);
					}
					else
					{
						spawnedItem.expirationDate = static_cast<time_t>(std::time(0)) + *opt;
					}
					session.addItem(spawnedItem);
					session.setLatestItemNumber(spawnedItem.serialInfo.itemNumber);
					response.setData(reinterpret_cast<std::uint8_t*>(&spawnedItem), sizeof(spawnedItem));
					session.asyncWrite(response);
				}
				sendConfirmation(response, session);
			}


			void SpawnItem::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
			{
				this->m_confirmationMessage += "/getitem itemid: gets an item through the specified itemID.";
				sendConfirmation(response, session);
			}

		};
	}
