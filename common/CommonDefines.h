#ifndef __INC_METIN2_COMMON_DEFINES_H__
#define __INC_METIN2_COMMON_DEFINES_H__

#define ENABLE_PLAYER_PER_ACCOUNT5
#define ENABLE_PORT_SECURITY

enum eCommonDefines {
	MAP_ALLOW_LIMIT = 60, // 32 default
};

#define ENABLE_WOLFMAN_CHARACTER
#ifdef ENABLE_WOLFMAN_CHARACTER
#define USE_MOB_BLEEDING_AS_POISON
#define USE_MOB_CLAW_AS_DAGGER
// #define USE_ITEM_BLEEDING_AS_POISON
// #define USE_ITEM_CLAW_AS_DAGGER
#define USE_WOLFMAN_STONES
#define USE_WOLFMAN_BOOKS
#endif

#define ENABLE_MOUNT_COSTUME_SYSTEM

// #define ENABLE_MAGIC_REDUCTION_SYSTEM
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
// #define USE_MAGIC_REDUCTION_STONES
#endif

/* OFFLINE SHOPS */
#define OFFLINE_SHOP // Offline shops system
#define GIFT_SYSTEM // gift system enable
#define FULL_YANG // Enable support for yang type long long
#define SHOP_TIME_REFRESH 1*60 // time for cycle checking older shops
#define SHOP_BLOCK_GAME99 //Blocking create shops on channel 99
//#define SHOP_DISTANCE // Show shops in pos distance like WoM2
#define SHOP_AUTO_CLOSE //Enable auto closing shop after sell last item
//#define SHOP_ONLY_ALLOWED_INDEX //Enable limiting for other map index which is not defined in player.shop_limit
//#define SHOP_HIDE_NAME // Enable hidding shop npc names like "Player's shop"
#define SHOP_GM_PRIVILEGES GM_IMPLEMENTOR //Minimum GM privileges to using Shop GM Panel
#define __SEND_TARGET_INFO__
#define __SASH_SYSTEM__
#define SHOP_SEARCH
#define SHOP_SEARCH_PRICE_MIN //Price in gui is a minimum eq. price >= item_price if is commented price in gui is a maximum
#define ENABLE_DICE_SYSTEM
#define __CHANGELOOK_SYSTEM__
#define __VIEW_TARGET_PLAYER_HP__
#define __VIEW_TARGET_DECIMAL_HP__
#define ENABLE_CUBE_RENEWAL
#define ENABLE_AGGREGATE_MONSTER_EFFECT
#define ENABLE_OFFLINE_SHOP_HIDE
#define ITEM_REMAINING
#define ENABLE_GEM_SYSTEM
#define ENABLE_NEWSTUFF
#define ENABLE_FIX_TINCTURA_WITHOUT_RELOG
#define ENABLE_FIX_ELIXIR_TIMER
#define __EXPANDED_BOOK_NAME__
#define __WEAPON_COSTUME_SYSTEM__
#define __COSTUME_ATTR_SYSTEM__
#define NEW_PET_SYSTEM
#define __COSTUME_RING_SYSTEM__
#define ELEMENT_TARGET
#define __VERSION_162__
#ifdef __VERSION_162__
	#define HEALING_SKILL_VNUM 265
#endif
#define __DUNGEON_FOR_GUILD__
#ifdef __DUNGEON_FOR_GUILD__
	#define __MELEY_LAIR_DUNGEON__
	#ifdef __MELEY_LAIR_DUNGEON__
		#define __DESTROY_INFINITE_STATUES_GM__
		#define __LASER_EFFECT_ON_75HP__
		#define __LASER_EFFECT_ON_50HP__
	#endif
#endif
#define ENABLE_POLY_SHOP
#define ENABLE_SUPPORT_SYSTEM
#define __TITLE_SYSTEM__
#define ENABLE_PVP_ADVANCED
#define EQUIP_ENABLE_VIEW_SASH

#ifdef ENABLE_PVP_ADVANCED
	#define __NEWPET_SYSTEM__
	#define BLOCK_CHANGEITEM	"pvp.BlockChangeItem"
	#define BLOCK_BUFF			"pvp.BlockBuff"
	#define BLOCK_POTION		"pvp.BlockPotion"
	#define BLOCK_RIDE			"pvp.BlockRide"
	#define BLOCK_PET			"pvp.BlockPet"
	#define BLOCK_POLY			"pvp.BlockPoly"
	#define BLOCK_PARTY			"pvp.BlockParty"
	#define BLOCK_EXCHANGE_		"pvp.BlockExchange"
	#define BLOCK_EQUIPMENT_	"pvp.BLOCK_VIEW_EQUIPMENT"
	#define BET_WINNER			"pvp.BetMoney"
	#define CHECK_IS_FIGHT		"pvp.IsFight"
#endif
#define ENABLE_FEATURES_REFINE_SYSTEM
#define ENABLE_WHISPER_ADMIN_SYSTEM
#define ENABLE_EXTENDED_ITEMNAME_ON_GROUND
#define __WJ_SHOW_MOB_INFO__
// #define ENABLE_SHOW_LIDER_AND_GENERAL_GUILD
#define WJ_ENABLE_TRADABLE_ICON
#define ENABLE_BIOLOG_SYSTEM
#define ENABLE_BIOLOG_SYSTEM_TIME
#define ENABLE_ADMIN_BAN_MANAGER
#ifndef ENABLE_ANNOUNCEMENT_REFINE_SUCCES
	#define ENABLE_ANNOUNCEMENT_REFINE_SUCCES
	#define ENABLE_ANNOUNCEMENT_REFINE_SUCCES_MIN_LEVEL 9 // Put what +level you want to announcement in chat when u upgrade something
#endif
#define ENABLE_PET_SLOT
#define NEW_PET_SUMMON
#define ENABLE_CHANGE_CHANNEL
// #define ENABLE_FEATURES_OXEVENT
// #define ENABLE_SHOW_QUIZ_NUMBER_OXEVENT
#define ENABLE_GLOBAL_CHAT
#define ENABLE_AFFECT_POLYMORPH_REMOVE
#define ENABLE_SHOW_CHEST_DROP
#define DISABLE_SEQUENCE_SYSTEM
#define ENABLE_SPECIAL_STORAGE
#define __ENABLE_BATTLE_FIELD__
#define ENABLE_ATTENDANCE_EVENT
#define ENABLE_CHANGELOOK_EXTEND
#define ENABLE_FISH_EVENT
#define __7AND8TH_SKILLS__
#endif
