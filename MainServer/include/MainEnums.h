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

		enum TradeSystemExtra
		{
			TRADE_SUCCESS = 1,
			TRADE_CANCELLED = 1,
			TRADE_CONFIRMED_NOTIFY_OTHER_PLAYER = 1,
			TARGET_NOT_ENOUGH_INVENTORY_SPACE = 7,
			TRADE_NOT_ENOUGH_MONEY = 14,
			TRADE_DISABLED_FOR_MAINTENANCE = 15, // currently unused by the server
			TRADE_COOLDOWN = 21, // currently unused by the server; "you can trade again in X hours"] where option is X
			TRADE_DECLINED = 31,
			ITEMS_CANNOT_BE_CHANGED_AFTER_LOCK = 44, // currently unused by the server (the client prevents changing items after lock already)
			BOTH_PLAYERS_MUST_CONFIRM_TRADE_BEFORE_FINALIZATION = 45,
			CANNOT_TRADE_NOW_OR_PLAYER_OFFLINE = 47,
			PLAYERS_NOT_FRIENDS = 73,
			LEVEL_TOO_LOW = 74,
			MAX_NUM_OF_ITEMS_AT_ONCE_REACHED = 75,
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
			GRADE_TESTER = 4,
			GRADE_GM = 7
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
			// TRADE??
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