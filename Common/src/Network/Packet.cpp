#include <algorithm>
#include <iostream>
#include <cstring>

#include "../../include/Network/Packet.h"
#include "../../include/Enums/MiscellaneousEnums.h"
#include "../../include/Utils/RandomGeneration.h"
#include "../../include/Utils/Parser.h"

namespace Common
{
    namespace Network
    {
        Packet::Packet()
        {
            m_header.setBogus(0);
            m_command.setBogus(0);
        }

        void Packet::setCommand(std::uint16_t order, std::uint8_t mission, std::uint8_t extra, std::uint8_t option)
        {
            m_command = Common::Protocol::CommandHeader{ mission, order, extra, option };
        }

        void Packet::setTcpHeader(std::uint32_t sessionId, std::uint32_t crypt) 
        {
            m_header.initialize(sessionId, 0/* crypt*/, sizeof(m_header) + sizeof(m_command) + m_data.size());
        }

        void Packet::setSession(std::uint16_t session)
        {
            m_header.setSessionId(session);
        }

        void Packet::setMission(std::uint8_t mission)
        {
            m_command.setMission(mission);
        }

        void Packet::setOrder(std::uint16_t order)
        {
            m_command.setOrder(order);
        }

        void Packet::setSize(std::uint32_t size)
        {
            m_header.setSize(size);
        }

        void Packet::setExtra(std::uint8_t extra)
        {
            m_command.setExtra(extra);
        }

        void Packet::setOption(std::uint8_t option)
        {
            m_command.setOption(option);
        }

        void Packet::setData(std::uint8_t* data, uint16_t size)
        {
            m_data.assign(data, data + size);
            m_header.setSize(sizeof(m_header) + sizeof(m_command) + size);
        }

        const std::uint8_t* const Packet::getData() const
        {
            return m_data.data();
        }

        std::uint16_t Packet::getSession() const
        {
            return m_header.getSessionId();
        }

        std::uint8_t Packet::getMission() const
        {
            return m_command.getMission();
        }

        std::uint16_t Packet::getOrder() const
        {
            return m_command.getOrder();
        }

        std::uint8_t Packet::getExtra() const
        {
            return m_command.getExtra();
        }

        std::uint8_t Packet::getOption() const
        {
            return m_command.getOption();
        }

        std::uint32_t Packet::getDataSize() const
        {
            return static_cast<std::uint32_t>(m_data.size());
        }

        std::uint32_t Packet::getFullSize() const
        {
            return static_cast<std::uint32_t>(sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader) + m_data.size());
        }

        void Packet::processIncomingPacket(std::uint8_t* data, std::uint16_t size, std::optional<std::uint32_t> crypt_key)
        {
            constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader);
            constexpr std::size_t commandSize = sizeof(Common::Protocol::CommandHeader);

            if (size < headerSize) return;

            Common::Cryptography::Crypt crypt;
            if (crypt_key.has_value())
            {
                crypt.KeySetup(0);
                crypt.RC5Decrypt32(data, &m_header, headerSize);
            }
            else
            {
                std::memcpy(&m_header, data, headerSize);
            }
            
            const std::uint16_t messageSize = static_cast<std::uint16_t>(m_header.getSize()) - headerSize;
            if (messageSize <= 0) return;

            std::vector<std::uint8_t> decryptedBytes(data + headerSize, data + headerSize + messageSize);

            switch (static_cast<Common::Enums::EncryptionType>(m_header.getCrypt()))
            {
            case Common::Enums::EncryptionType::NO_ENCRYPTION:
                break;

            case Common::Enums::EncryptionType::DEFAULT_ENCRYPTION:
                crypt.KeySetup(0);
                crypt.RC5Decrypt64(decryptedBytes.data(), decryptedBytes.data(), messageSize);
                break;

            case Common::Enums::EncryptionType::DEFAULT_LARGE_ENCRYPTION:
                crypt.KeySetup(0);
                crypt.RC6Decrypt128(decryptedBytes.data(), decryptedBytes.data(), messageSize);
                break;

            case Common::Enums::EncryptionType::USER_ENCRYPTION:
                crypt.KeySetup(crypt_key.value_or(0));
                crypt.RC5Decrypt64(decryptedBytes.data(), decryptedBytes.data(), messageSize);
                break;

            case Common::Enums::EncryptionType::USER_LARGE_ENCRYPTION:
                crypt.KeySetup(crypt_key.value_or(0));
                crypt.RC6Decrypt128(decryptedBytes.data(), decryptedBytes.data(), messageSize);
                break;

            default:
                // Invalid packet
                return;
            }

            std::memcpy(&m_command, decryptedBytes.data(), headerSize);

            if (messageSize > commandSize)
            {
                setData(decryptedBytes.data() + commandSize, messageSize - commandSize);
            }
            else
            {
                setData(nullptr, 0);
            }

            
            //do printing here so I can filter based on orders
            if (m_command.getOrder() == 71 || m_command.getOrder() == 72)// || m_command.getOrder() == 281)
                return;

            std::cout << "\n\033[1;32m[ client -> server ]\033[1;36m";
            std::cout << "\033[1;35mSize:" << size << "] \n";
            Parser::printTcpHeader(m_header);

            std::cout <<"\033[1;37m";
            for (std::size_t i = 0; i < m_header.getSize(); ++i)
            {
                printf("%02X ", static_cast<std::uint8_t>(data[i]));
            }
            std::cout << '\n';
            std::cout << "\033[1;36mCommand Header:\n\033[1;35m";
            std::cout << "[Mission:" << m_command.getMission() << "] \n";
            std::cout << "[Order:" << m_command.getOrder() << "] \n";
            std::cout << "[Extra:" << m_command.getExtra() << "] \n";
            std::cout << "[Option:" << m_command.getOption() << "] \n";
            std::cout << "[Padding:" << m_command.getBogus() << "] \n";
            std::cout << "\033[1;36mDecrypted Packet:\n\033[1;37m";
            for (std::size_t i = 0; i < m_header.getSize(); ++i)
            {
                printf("%02X ", static_cast<std::uint8_t>(decryptedBytes[i]));
            }
            
        }
  
        std::vector<std::uint8_t> Packet::generateOutgoingPacket(std::optional<std::uint32_t> crypt_key) const
        {
            Common::Cryptography::Crypt crypt;
           
            const std::size_t headerSize = sizeof(Common::Protocol::TcpHeader);
            const std::size_t commandSize = sizeof(Common::Protocol::CommandHeader);
            const std::size_t partialSize = commandSize + m_data.size();
            const std::size_t completeSize = headerSize + commandSize + m_data.size();

            std::vector<std::uint8_t> completeData(completeSize, 0);
            std::memcpy(completeData.data(), &m_header, headerSize);

            if (crypt_key != std::nullopt)
            {
                crypt.KeySetup(0);
                crypt.RC5Encrypt32(completeData.data(), completeData.data(), headerSize);
            }

            std::memcpy(completeData.data() + headerSize, &m_command, commandSize);
            std::memcpy(completeData.data() + headerSize + commandSize, m_data.data(), m_data.size());

            switch (static_cast<Common::Enums::EncryptionType>(m_header.getCrypt()))
            {
            case static_cast<uint32_t>(Common::Enums::EncryptionType::NO_ENCRYPTION):
                break;

            case static_cast<uint32_t>(Common::Enums::EncryptionType::DEFAULT_ENCRYPTION):
                crypt.KeySetup(0);
                crypt.RC5Encrypt64(completeData.data() + headerSize, completeData.data() + headerSize, partialSize);
                break;

            case static_cast<uint32_t>(Common::Enums::EncryptionType::DEFAULT_LARGE_ENCRYPTION):
                crypt.KeySetup(0);
                crypt.RC6Encrypt128(completeData.data() + headerSize, completeData.data() + headerSize, partialSize);
                break;

            case static_cast<uint32_t>(Common::Enums::EncryptionType::USER_ENCRYPTION):
                crypt.KeySetup(crypt_key.value_or(0));
                crypt.RC5Encrypt64(completeData.data() + headerSize, completeData.data() + headerSize, partialSize);
                break;

            case static_cast<uint32_t>(Common::Enums::EncryptionType::USER_LARGE_ENCRYPTION):
                crypt.KeySetup(crypt_key.value_or(0));
                crypt.RC6Encrypt128(completeData.data() + headerSize, completeData.data() + headerSize, partialSize);
                break;

            default:
                // Invalid packet
                break;
            }

            return completeData;
        }

    } // End namespace Network
}