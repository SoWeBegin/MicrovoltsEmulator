#ifndef COMMON_PACKET_H
#define COMMON_PACKET_H

#include <stdlib.h>
#include <vector>

#include "../Protocol/TcpHeader.h"
#include "../Protocol/CommandHeader.h"
#include "../Cryptography/Crypt.h"
#include <optional>

namespace Common
{
    namespace Network
    {
        class Packet
        {
        private:
            Common::Protocol::TcpHeader m_header{};
            Common::Protocol::CommandHeader m_command{};
            std::vector<uint8_t> m_data{};

        public:
            explicit Packet();

            void setTcpHeader(std::uint32_t sessionId, std::uint32_t crypt);
            void setCommand(std::uint16_t order, std::uint8_t mission, std::uint8_t extra, std::uint8_t option);
            void setData(std::uint8_t* data, std::uint16_t size);
            void setSession(std::uint16_t session);
            void setMission(std::uint8_t mission);
            void setOrder(std::uint16_t order);
            void setExtra(std::uint8_t extra);
            void setOption(std::uint8_t option);
            void setSize(std::uint32_t size);

            const std::uint8_t* const getData() const;
            std::uint32_t getDataSize() const;
            std::uint32_t getFullSize() const;
            std::uint16_t getSession() const;
            std::uint8_t getMission() const;
            std::uint16_t getOrder() const;
            std::uint8_t getExtra() const;
            std::uint8_t getOption() const;

            std::vector<std::uint8_t> generateOutgoingPacket(std::optional<std::uint32_t> crypt_key = std::nullopt) const;
            void processIncomingPacket(std::uint8_t* data, std::uint16_t size, std::optional<std::uint32_t> = std::nullopt);
        };

    } // end namespace Network
}

#endif