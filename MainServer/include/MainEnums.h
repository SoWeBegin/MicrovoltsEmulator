#ifndef MAIN_ENUMERATIVES_H
#define MAIN_ENUMERATIVES_H

#include <cstdint>

namespace Main
{
	namespace Enums
	{
		enum RoomSimpleSetting
		{
			SETTING_ITEM,
			SETTING_OPEN,
			SETTING_OBSERVER,
			SETTING_TEAMBALANCE,
			SETTING_MAP,
			SETTING_PLAYERS_PER_TEAM,
			SETTING_TIME,
			SETTING_SPECIFIC
		};

		enum MatchEnd
		{
			MATCH_WON,
			MATCH_LOST,
			MATCH_DRAW
		};

		enum SellItemExtra
		{
			SELL_SUCCESS = 1,
			SELL_ERROR = 21,
		};

		enum MailboxExtra
		{
			MAILBOX_SENT = 0,
			RECEIVER_NOT_FOUND = 4,
			RECEIVER_NO_SPACE_LEFT = 8,
			SENDER_NO_SPACE_LEFT = 17,
			MAILBOX_RECEIVER_BLOCKED_SENDER = 42,
			MAILBOX_RECEIVED = 46,
			MAILBOX_HAS_BEEN_READ = 53
		};

		enum MailboxMission
		{
			MISSION_MAILBOX_RECEIVED = 0,
			MISSION_MAILBOX_SENT = 1,
		};

		enum InitialPacketOption
		{
			CHANNEL1 = 1,
			CHANNEL2,
			CHANNEL3,
			CHANNEL4,
			CHANNEL5,
			CHANNEL6
		};

		enum PlayerGrade
		{
			GRADE_NORMAL = 2,
			GRADE_MOD = 3,
			GRADE_GM = 4
		};

		enum ChatGrade
		{
			CHAT_NORMAL = 0,
			CHAT_MOD,
			CHAT_GM,
			CHAT_TESTER,
		};

		enum ChatExtra
		{
			NORMAL = 0,
			HELP = 1,
			WHISPER = 2,
			COMMAND = 3,
			TEAM = 5,
			CLAN = 7,
			INFO = 10,
			GM = 11,
			TIP = 12,
		};

		enum WhisperExtra
		{
			WHISPER_SENT = 1,
			WHISPER_SENDER_BLOCKED_RECEIVER = 2, // Whisper failed => Change to "Cannot whisper to a blocked player.
			RECEIVER_OFFLINE = 13,
			WHISPER_SELF = 15,
			WHISPER_RECEIVER_BLOCKED_SENDER = 35, // "Recipient has refused your whisper" => Change to "The player you're trying to whisper to has blocked you."
		};

		enum ID_QUIT
		{
			ID_QUIT_DEFAULT_ACCOUNT_BLOCKED = 0,// ID_QUIT_LOCK
			ID_QUIT_DATAERROR = 4,
			ID_QUIT_BUSY = 5,
			ID_QUIT_UNKNOWN_ERROR = 27,         // ID_QUIT_CLOSE
			ID_QUIT_REMOVED_BYMOD = 35,         // ID_QUIT_DENY
			ID_QUIT_MULTIPLE_USERS = 42,        // ID_QUIT_BLOCK
			ID_QUIT_MAINTENANCE = 47            // ID_QUIT_OFFLINE
		};

		enum ItemCurrencyType
		{
			ITEM_MP,
			ITEM_RT,
			ITEM_COUPON,
			ITEM_COIN
		};

		enum BoxItemsCommon 
		{
			MP_100 = 4600001,
			MP_500 = 4600005,
			MP_1000 = 4600010,
			MP_2500 = 4600020,
			MP_3000 = 4600030,
			MP_4000 = 4600040,
			MP_5000 = 4600050,
			MP_7000 = 4600070,
			MP_10000 = 4600100,
			MP_20000 = 4600200,
			MP_50000 = 4600500,
			MP_100000 = 4601000,
			MP_500000 = 4605000,
			COUPON_1 = 4306001,
			COUPON_5 = 4306002,
			COUPON_10 = 4306003,
			WHITE_HAMMER = 3015250,
			WHITE_KATANA = 3015450,
			WHITE_PENCIL = 3015350,
			WHITE_SEAWASP = 3042850,
			OUTLAW = 3062850,
			WHITE_PREDATOR = 3072850,
			GOLDEN_KUKRI = 3200750,
			MENTHA = 3023550,
			ACME_BLASTER = 3037050, // does NOT work (makes client crash) -- replace this one
			GOLD_CORAL = 3034350,
		};

		enum BoxIds
		{
			BRILLIANT_HAMMER = 4530000,
			BRILLIANT_SWORD = 4530001,
			BRILLIANT_PENCIL = 4530002,
			BRILLIANT_SNIPER = 4530003,
			BRILLIANT_BAZOOKA = 4530004,
			BRILLIANT_GRENADE = 4530005,
			BRILLIANT_KUKRI = 4530007,
			BRILLIANT_MENTHA = 4530008,
			BRILLIANT_CORAL = 4530009,
			BRILLIANT_BLASTER = 4530010,
		};

		enum ItemIds
		{
			RECORD_RESET = 4302000,
			KILLDEATH_RESET = 4303000,
			BATTERY_500_RT = 4305005,
			BATTERY_500_MP = 4305009,
			BATTERY_1000_RT = 4305006,
			BATTERY_1000_MP = 4305010,
			BATTERY_EXPANSION = 4305007,
		};

		enum ItemExpirationType
		{
			// // 0=nothing happens, 1=becomes unused(1 day); 2=bomb?!;  3<=expired
			UNLIMITED = 0,
			UNUSED,
			BOMB
		};

		enum ItemFrom
		{
			SHOP = 0,
			GIFT,
			UNKNOWN,
		};

		// Sent to the source who sent the friend request
		enum AddFriendServerExtra
		{
			REQUEST_SENT = 0,
			REQUEST_ACCEPTED = 1, // Friend is registered as your friend ??!!
			TARGET_NOT_FOUND = 6, // Cannot find player
			TARGET_OR_SENDER_FRIEND_LIST_FULL = 7, // ok
			SEND_REQUEST_TO_TARGET = 28, // ok
			REQUEST_RECEIVED_WITH_OPTION = 37, // ????????
			RECEIVER_BLOCKED_SENDER = 42,
			DB_ERROR,
		};

		enum AddFriendServerMission
		{
			SENDER_FRIENDLIST_FULL,
			RECEIVER_FRIENDLIST_FULL
		};

		enum ClientFriendExtra
		{
			FRIEND_REQUEST_SENT = 28,
			INCOMING_FRIEND_REQUEST_ACCEPTED = 30
		};
	}
}
#endif