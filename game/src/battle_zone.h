#ifndef __INC_METIN_II_BATTLE_ZONE_H__
#define __INC_METIN_II_BATTLE_ZONE_H__

#include <boost/unordered_map.hpp>

#include "../../common/stl.h"
#include "../../common/length.h"
#include "../../common/tables.h"
#include "packet.h"
#include <lua.h>

enum
{
	BATTLE_ZONE_MAP_INDEX = 357,	// Map index
	MIN_LEVEL_FOR_ENTER = 50,
	TIME_BETWEEN_KILLS = 300, 		// In seconds
};

enum
{
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 3,
	THURSDAY = 4,
	FRIDAY = 5,
	SATURDAY = 6,
	SUNDAY = 0,
};

class CBattleZone : public singleton<CBattleZone>
{
/*
* Standard functions
*/
	public:
		CBattleZone ();
		virtual ~CBattleZone ();

		bool Initialize();
		void Destroy();
		
		bool IsBattleZoneMapIndex(int iMapIndex) const { return iMapIndex == BATTLE_ZONE_MAP_INDEX; };
		
		void PlayerKill(LPCHARACTER pChar, LPCHARACTER pKiller);
		void RewardKiller(LPCHARACTER pKiller, LPCHARACTER pChar);
		void RewardVictim(LPCHARACTER pChar);
		
		void Connect(LPCHARACTER pChar);
		void RestartAtRandomPos(LPCHARACTER pChar);
		PIXEL_POSITION GetRandomPosition();
		
		void RemovePlayerFromVector(DWORD dwPlayerId);
		void TargetOnLeave(LPCHARACTER pChar, bool isDelete);
		bool IsInLeaveProcess(LPCHARACTER pkKiller, LPCHARACTER pChar);
		
	private:
		std::vector<DWORD> m_vec_char;

		typedef std::map<DWORD, TBattleKillVictim> TKillMap;
		TKillMap m_playersKills;
		
/*
* Ranking functions
*/		
	public:
		void CreateRankings(bool isTotal);
		int	GetRankingPosition(LPCHARACTER pChar);
		void RegisterBattleRanking(LPCHARACTER pChar, int iTempPoints);
		
	private:
		std::vector<TBattleRankingMember> ranking_vec;
		
/*
* Open time functions
*/		
	public:
		int GetTimeDiff(bool isClose);
		int GetOpenTimeList(lua_State* L);
		
		void Update();
		
/*
* Enter functions
*/
	public:
		void SetEnterStatus(bool bFlag) { bEnterStatus = bFlag; };
		bool GetEnterStatus() const { return bEnterStatus; };
		
		void SetEventStatus(bool bFlag) { bEventStatus = bFlag; };
		bool GetEventStatus() const { return bEventStatus; };
		
		void SetEventInfo(BYTE bMonth, BYTE bDay) { bEventMonth = bMonth; bEventDay = bDay; };
		bool GetEventMonth() const { return bEventMonth; };
		bool GetEventDay() const { return bEventDay; };
		
		void OpenEnter(bool isEvent = false);
		void CloseEnter(bool isGm = false);
		
		void RequestEnter(LPCHARACTER pChar);
		void RequestExit(LPCHARACTER pChar);
		
	private:
		bool bEnterStatus, bEventStatus;
		BYTE bEventMonth, bEventDay;
};

#endif

