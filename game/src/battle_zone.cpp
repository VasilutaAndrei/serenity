#include "stdafx.h"
#include "config.h"

#include "../../common/length.h"
#include "../../common/tables.h"
#include "p2p.h"
#include "locale_service.h"
#include "char.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "questmanager.h"
#include "questlua.h"
#include "start_position.h"
#include "char_manager.h"
#include "sectree_manager.h"
#include "regen.h"
#include "log.h"
#include "db.h"
#include "target.h"
#include "party.h"
#include "battle_zone.h"
#include "utils.h"

CBattleZone::CBattleZone()
{
	Initialize();
}

CBattleZone::~CBattleZone()
{
	Destroy();
}

bool CBattleZone::Initialize()
{
	m_vec_char.clear();
	SetEventInfo(12, 32);
	
	return true;
}

void CBattleZone::Destroy()
{
	CloseEnter();
	SetEventInfo(12, 32);
	
	m_vec_char.clear();
}

/*
*	{ 
*		Day name  	 -> eg.: MONDAY, 		
*		Open Hour 	 -> eg.: 13, 		== [0,6] (Sunday = 0)
*		Open Minute  -> eg.: 0,			== [0,59]
*		Close Hour	 -> eg.: 14,		== [0,6] (Sunday = 0)
*		Close Minute -> eg.: 0			== [0,59]
*	},
*
*   All the code written above means : 
*	Monday at hour 13:00 battle zone will open
*	And it will close at 13:00
*/
const TOpenTimeInfo openTimeInfo[7 * 2] = {
	{ MONDAY, 		13, 0, 14, 0 },
	{ MONDAY, 		20, 0, 22, 0 },
	{ TUESDAY, 		13, 0, 14, 0 },
	{ TUESDAY, 		20, 0, 22, 0 },
	{ WEDNESDAY, 	13, 0, 14, 0 },
	{ WEDNESDAY, 	20, 0, 22, 0 },
	{ THURSDAY, 	13, 0, 14, 0 },
	{ THURSDAY, 	20, 0, 22, 0 },
	{ FRIDAY, 		13, 0, 14, 0 },
	{ FRIDAY, 		20, 0, 22, 0 },
	{ SATURDAY, 	13, 0, 14, 0 },
	{ SATURDAY, 	20, 0, 22, 0 },
	{ SUNDAY, 		13, 0, 14, 0 },
	{ SUNDAY, 		20, 0, 22, 0 },
};

struct FExitTarget
{
	LPCHARACTER pTarget;
	bool isDelete;
	
	FExitTarget(LPCHARACTER p, bool bDelete = false)
	{
		pTarget = p;
		isDelete = bDelete;
	}
	
	void operator() (LPENTITY ent)
	{
		if (NULL != ent)
		{
			LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

			if (NULL != pChar)
			{
				if (true == pChar->IsPC())
				{
					if(isDelete)
					{
						CTargetManager::Instance().DeleteTarget(pChar->GetPlayerID(), 10000 + pTarget->GetPlayerID(), "BATTLE_ZONE_TARGET");
					}
					else
					{
						CTargetManager::Instance().CreateTarget(pChar->GetPlayerID(), 10000 + pTarget->GetPlayerID(), "BATTLE_ZONE_TARGET", 2, pTarget->GetVID(), 0, pTarget->GetMapIndex(), "1");
					}
				}
			}
		}
	}
};

void CBattleZone::PlayerKill(LPCHARACTER pChar, LPCHARACTER pkKiller)
{
	if (NULL == pChar || NULL == pkKiller)
		return;
	
	if (false == pChar->IsPC() || false == pkKiller->IsPC()) 
		return;

	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()) || false == IsBattleZoneMapIndex(pkKiller->GetMapIndex()))
		return;
	
	if (pChar->m_pkBattleZoneEvent)
	{
		event_cancel(&pChar->m_pkBattleZoneEvent);
	}
	
	RewardKiller(pkKiller, pChar);
	RewardVictim(pChar);
}

void CBattleZone::RewardKiller(LPCHARACTER pkKiller, LPCHARACTER pChar)
{
	if (NULL == pChar || NULL == pkKiller)
		return;
	
	if (false == pChar->IsPC() || false == pkKiller->IsPC()) 
		return;

	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()) || false == IsBattleZoneMapIndex(pkKiller->GetMapIndex()))
		return;
	
	if (!pChar->GetDesc() || !pkKiller->GetDesc())
		return;
	
	if (pChar->GetDesc()->GetHostName() == pkKiller->GetDesc()->GetHostName()) 
		return;
	
	for ( TKillMap::iterator iter = m_playersKills.begin(); iter != m_playersKills.end(); iter++ )
	{
		if(iter->first == pkKiller->GetPlayerID() && iter->second.dwVictimPid == pChar->GetPlayerID())
		{
			if (get_dword_time() < iter->second.dwLastKillTime + TIME_BETWEEN_KILLS * 1000)
			{
				pkKiller->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You already kill this player in last 5 minutes."));
				return;
			}
			else
			{
				m_playersKills.erase(iter);
			}
		}
	}

	int addPointForKiller = GetEventStatus() == true ? (IsInLeaveProcess(pkKiller, pChar) ? 10 : 2) : (IsInLeaveProcess(pkKiller, pChar) ? 5 : 1);
	pkKiller->PointChange(POINT_TEMP_BATTLE_POINT, addPointForKiller);
	pkKiller->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You currently have %d point(s). Your total score is %d."), addPointForKiller, pkKiller->GetPoint(POINT_TEMP_BATTLE_POINT));

	TBattleKillVictim tempVictim;
	tempVictim.dwVictimPid = pChar->GetPlayerID();
	tempVictim.dwLastKillTime = get_dword_time();
	
	m_playersKills.insert(std::make_pair(pkKiller->GetPlayerID(), tempVictim));
}

void CBattleZone::RewardVictim(LPCHARACTER pChar)
{
	if (NULL == pChar)
		return;
	
	if (false == pChar->IsPC()) 
		return;

	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()))
		return;
	
	int removeHalfPoints = (pChar->GetPoint(POINT_TEMP_BATTLE_POINT) / 2) + (pChar->GetPoint(POINT_TEMP_BATTLE_POINT) % 2);
	pChar->PointChange(POINT_TEMP_BATTLE_POINT, -removeHalfPoints);
	if(pChar->GetPoint(POINT_TEMP_BATTLE_POINT) > 0)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have lost %d points. Remaining points: %d."), removeHalfPoints, pChar->GetPoint(POINT_TEMP_BATTLE_POINT));
	}
	
	pChar->IncreaseBattleDeadCount();
	
	TargetOnLeave(pChar, true);
}

void CBattleZone::TargetOnLeave(LPCHARACTER pChar, bool isDelete)
{
	if (NULL == pChar)
		return;
	
	if (false == pChar->IsPC()) 
		return;
	
	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap( pChar->GetMapIndex() );
	
	if (NULL == pSecMap)
		return;
	
	FExitTarget f(pChar, isDelete);
	pSecMap->for_each( f );
}

bool CBattleZone::IsInLeaveProcess(LPCHARACTER pkKiller, LPCHARACTER pChar)
{
	if (NULL == pChar || NULL == pkKiller)
		return false;
	
	if (false == pChar->IsPC() || false == pkKiller->IsPC()) 
		return false;
	
	TargetInfo * pInfo = CTargetManager::instance().GetTargetInfo(pkKiller->GetPlayerID(), 2, pChar->GetVID());
	
	if (pInfo) {
		if(pInfo->dwQuestIndex == 10000 + pChar->GetPlayerID())
		{
			return true;
		}
	}
	
	return false;
}

void CBattleZone::Connect(LPCHARACTER pChar)
{
	if (NULL == pChar)
		return;
	
	if (false == pChar->IsPC()) 
		return;
	
	if (!pChar->GetDesc())
		return;
	
	if (get_global_time() - pChar->GetQuestFlag("battle_zone.usable_points_last_update") > 86400)
	{
		pChar->SetQuestFlag("battle_zone.usable_points_last_update", get_global_time());
		pChar->SetQuestFlag("battle_zone.usable_points", quest::CQuestManager::instance().GetEventFlag("battle_limit_max_points"));
	}

	if (pChar->GetQuestFlag("battle_zone.first_time_open") == 0)
	{
		pChar->SetQuestFlag("battle_zone.first_time_open", 1);
		pChar->SetQuestFlag("battle_zone.usable_points", quest::CQuestManager::instance().GetEventFlag("battle_limit_max_points"));
	}
	
	//if(pChar->GetGMLevel() > GM_PLAYER && GetEnterStatus() == false)
	//{
	//	OpenEnter();
	//}
	
	ranking_vec.clear();
	
	CreateRankings(true);
	CreateRankings(false);
	
	pChar->SetBattleRank(GetRankingPosition(pChar));

	TPacketGCBattleInfo packet;
	packet.bHeader = HEADER_GC_BATTLE_ZONE_INFO;
	packet.wSize = sizeof(packet) + sizeof(TBattleRankingMember) * ranking_vec.size();
	
	packet.iOpenTime = GetTimeDiff(false);;
	packet.iCloseTime = GetTimeDiff(true);
	
	packet.bIsEnterOpen = GetEnterStatus();
	packet.bIsEvent = GetEventStatus();
	
	pChar->GetDesc()->BufferedPacket(&packet, sizeof(packet));
	pChar->GetDesc()->Packet(&ranking_vec[0], sizeof(TBattleRankingMember) * ranking_vec.size());
	
	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()))
		return;
	
	pChar->SkillLevelPacket();
	
	if(GetEnterStatus() == false)
	{
		m_vec_char.erase(std::remove(m_vec_char.begin(), m_vec_char.end(), pChar->GetPlayerID()), m_vec_char.end()); 
		pChar->WarpSet(EMPIRE_START_X(pChar->GetEmpire()), EMPIRE_START_Y(pChar->GetEmpire()));
		return;
	}

	// Set battle mode so you can attack everyone
	pChar->SetPKMode(PK_MODE_BATTLE);
	
	// If player have a party and party is not null
	if (LPPARTY party = pChar->GetParty())
	{
		// Remove player from party
		party->Quit(pChar->GetPlayerID());
	}
	
	// Find if player id is already in vector
	if (std::find(m_vec_char.begin(), m_vec_char.end(), pChar->GetPlayerID()) != m_vec_char.end())
	{
		// If yes, the remove it
		m_vec_char.erase(std::remove(m_vec_char.begin(), m_vec_char.end(), pChar->GetPlayerID()), m_vec_char.end()); 
		
		// Warp player from battle map
		pChar->WarpSet(EMPIRE_START_X(pChar->GetEmpire()), EMPIRE_START_Y(pChar->GetEmpire()));
		return;
	}

	// If player was not found in vector then insert it
	m_vec_char.push_back(pChar->GetPlayerID());
}

PIXEL_POSITION CBattleZone::GetRandomPosition()
{
	PIXEL_POSITION validPositions[20] = { 
		{ 284, 204, 0 }, { 267, 180, 0 }, { 248, 184, 0 }, { 222, 192, 0 },
		{ 200, 192, 0 }, { 195, 210, 0 }, { 195, 236, 0 }, { 195, 272, 0 },
		{ 214, 283, 0 }, { 188, 300, 0 }, { 178, 326, 0 }, { 190, 346, 0 },
		{ 235, 341, 0 }, { 265, 330, 0 }, { 292, 330, 0 }, { 338, 290, 0 },
		{ 325, 325, 0 }, { 330, 305, 0 }, { 332, 265, 0 }, { 315, 237, 0 }
	};
	
	return validPositions[number(0, (sizeof(validPositions)/sizeof(validPositions[0])) - 1)];
}

void CBattleZone::RestartAtRandomPos(LPCHARACTER pChar)
{
	if (NULL == pChar)
		return;
	
	if (false == pChar->IsPC()) 
		return;
	
	int x = 0, y = 0;
	
	PIXEL_POSITION tempPos = GetRandomPosition();

	x = tempPos.x;
	y = tempPos.y;
	
	PIXEL_POSITION pos;
	if (SECTREE_MANAGER::instance().GetMapBasePosition(pChar->GetX(), pChar->GetY(), pos))
	{
		x += pos.x / 100;
		y += pos.y / 100;
	}
	
	x *= 100;
	y *= 100;

	pChar->Show(pChar->GetMapIndex(), x, y);
	pChar->Stop();
}

void CBattleZone::RemovePlayerFromVector(DWORD dwPlayerId)
{
	if (std::find(m_vec_char.begin(), m_vec_char.end(), dwPlayerId) != m_vec_char.end())
	{
		m_vec_char.erase(std::remove(m_vec_char.begin(), m_vec_char.end(), dwPlayerId), m_vec_char.end()); 
	}
}

int CBattleZone::GetTimeDiff(bool isClose)
{
	time_t iTime;
	time(&iTime);
	tm* pTimeInfo = localtime(&iTime);
	
	int finalValue = 0;
	
	for(DWORD i = 0; i < sizeof(openTimeInfo)/sizeof(openTimeInfo[0]); i+=2)
	{
		if(openTimeInfo[i].bDay == pTimeInfo->tm_wday && openTimeInfo[i + 1].bDay == pTimeInfo->tm_wday)
		{
			int curT = (pTimeInfo->tm_hour * 1000) + pTimeInfo->tm_min;
			int openT = (openTimeInfo[i].bOpenHour * 1000) + openTimeInfo[i].bOpenMinute;
			int closeT = (openTimeInfo[i].bCloseHour * 1000) + openTimeInfo[i].bCloseMinute;
			int openTN = (openTimeInfo[i + 1].bOpenHour * 1000) + openTimeInfo[i + 1].bOpenMinute;
			int closeTN = (openTimeInfo[i + 1].bCloseHour * 1000) + openTimeInfo[i + 1].bCloseMinute;
			int tempHourDiff = 0, tempMinuteDiff = 0;
		  
			if(curT <= openT)
			{
				tempHourDiff = isClose ? openTimeInfo[i].bCloseHour : openTimeInfo[i].bOpenHour;
				tempMinuteDiff = isClose ? openTimeInfo[i].bCloseMinute : openTimeInfo[i].bOpenMinute;
			}
			else if(curT <= closeT)
			{
				tempHourDiff = isClose ? openTimeInfo[i].bCloseHour : openTimeInfo[i].bOpenHour;
				tempMinuteDiff = isClose ? openTimeInfo[i].bCloseMinute : openTimeInfo[i].bOpenMinute;
			}
			else if(curT <= openTN)
			{
				tempHourDiff = isClose ? openTimeInfo[i + 1].bCloseHour : openTimeInfo[i + 1].bOpenHour;
				tempMinuteDiff = isClose ? openTimeInfo[i + 1].bCloseMinute : openTimeInfo[i + 1].bOpenMinute;
			}
			else if(curT <= closeTN)
			{
				tempHourDiff = isClose ? openTimeInfo[i + 1].bCloseHour : openTimeInfo[i + 1].bOpenHour;
				tempMinuteDiff = isClose ? openTimeInfo[i + 1].bCloseMinute : openTimeInfo[i + 1].bOpenMinute;
			}
			else
			{
				finalValue = 0;
				continue;
			}
			
			finalValue = (60 * ((tempHourDiff * 60 + tempMinuteDiff) - (pTimeInfo->tm_hour * 60 + pTimeInfo->tm_min))) + pTimeInfo->tm_sec;
			break;
		}
		else
		{
			finalValue = 0;
			continue;
		}
	}
	
	return finalValue;
}

int CBattleZone::GetOpenTimeList(lua_State* L)
{
	DWORD index = 1;
	lua_newtable(L);
	
	for(DWORD i = 0; i < sizeof(openTimeInfo)/sizeof(openTimeInfo[0]); i++)
	{
		lua_newtable(L);

		lua_pushnumber(L, openTimeInfo[i].bDay);
		lua_rawseti(L, -2, 1);

		lua_pushnumber(L, openTimeInfo[i].bOpenHour);
		lua_rawseti(L, -2, 2);

		lua_pushnumber(L, openTimeInfo[i].bOpenMinute);
		lua_rawseti(L, -2, 3);

		lua_pushnumber(L, openTimeInfo[i].bCloseHour);
		lua_rawseti(L, -2, 4);

		lua_pushnumber(L, openTimeInfo[i].bCloseMinute);
		lua_rawseti(L, -2, 5);

		lua_rawseti(L, -2, index++);
	}
}

static LPEVENT s_pkNoticeEvent;

EVENTINFO(notice_event_data)
{
	int seconds;
	bool isClose;
	notice_event_data(): seconds(0), isClose(0) {}
};

EVENTFUNC(notice_event)
{
	notice_event_data* info = dynamic_cast<notice_event_data*>(event->info);

	if (info == NULL)
	{
		sys_err("<notice_event_data> <Factor> Null pointer");
		return 0;
	}

	int& pSec = info->seconds;

	if(pSec >= 0)
	{
		if (pSec > 50 && pSec % 60 == 0)
		{
			char buf[64];
			if(info->isClose)
			{
				snprintf(buf, sizeof(buf), LC_TEXT("Combat Zone will close in %d minute(s)."), pSec / 60);
			}
			else
			{
				snprintf(buf, sizeof(buf), LC_TEXT("Combat Zone will open in %d minute(s)."), pSec / 60);
			}

			SendNotice(buf);
		}

		--pSec;
		return PASSES_PER_SEC(1);
	}
}

void CBattleZone::OpenEnter(bool isEvent)
{
	if(s_pkNoticeEvent)
	{
		event_cancel(&s_pkNoticeEvent);
	}
	
	SetEnterStatus(true);
	SendNotice(LC_TEXT("Combat Zone is open."));
	
	if(isEvent)
	{
		SetEventStatus(true);
	}
}

void CBattleZone::CloseEnter(bool isGm)
{
	if(s_pkNoticeEvent)
	{
		event_cancel(&s_pkNoticeEvent);
	}
	
	if(GetEventStatus() == true)
	{
		SetEventStatus(false);
	}
			
	SetEnterStatus(false);
	
	if(isGm)
	{
		SendNotice(LC_TEXT("Battle Zone was forced closed by an GM because of technical issues."));
	}
	else
	{
		SendNotice(LC_TEXT("Combat Zone is closed."));
	}
	
	for (DWORD i = 0; i < m_vec_char.size(); ++i)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::instance().FindByPID(m_vec_char[i]);
		if (pkChar != NULL)
		{
			pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
		}
	}
	
	m_vec_char.clear();
}

EVENTINFO(BattleZoneEventInfo)
{
	DynamicCharacterPtr ch;
	int         	left_second;
	bool			isExit;

	BattleZoneEventInfo()
	: ch()
	, left_second( 0 )
	, isExit( 0 )
	{
	}
};

EVENTFUNC(battle_zone_event)
{
	BattleZoneEventInfo * info = dynamic_cast<BattleZoneEventInfo *>( event->info );

	if ( info == NULL )
	{
		sys_err( "battle_zone_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) {
		return 0;
	}

	if (info->left_second <= 0)
	{
		ch->m_pkBattleZoneEvent = NULL;

		if(info->isExit)
		{
			if(ch->GetPoint(POINT_TEMP_BATTLE_POINT) > 0)
			{
				ch->PointChange(POINT_BATTLE_POINT, ch->GetPoint(POINT_TEMP_BATTLE_POINT));
				CBattleZone::instance().RegisterBattleRanking(ch, ch->GetPoint(POINT_TEMP_BATTLE_POINT));
				ch->PointChange(POINT_TEMP_BATTLE_POINT, -ch->GetPoint(POINT_TEMP_BATTLE_POINT));
			}
			
			CBattleZone::instance().RemovePlayerFromVector(ch->GetPlayerID());
			
			ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
		else
		{
			int x = 0, y = 0;
			
			PIXEL_POSITION tempPos = CBattleZone::instance().GetRandomPosition();
		
			x = tempPos.x;
			y = tempPos.y;
			
			PIXEL_POSITION pos;
			if (SECTREE_MANAGER::instance().GetMapBasePositionByMapIndex(BATTLE_ZONE_MAP_INDEX, pos))
			{
				x += pos.x / 100;
				y += pos.y / 100;
			}
			
			x *= 100;
			y *= 100;
		
			ch->WarpSet(x, y);
		}
		
		return 0;
	}
	else
	{
		if (info->left_second <= 15 && info->left_second > 0)
		{
			if(info->isExit)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d second(s) left to enter in the Combat Zone."), info->left_second);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d second(s) left to to join on the Combat Zone."), info->left_second);
			}
		}
		
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

void CBattleZone::RequestEnter(LPCHARACTER pChar)
{
	if (NULL == pChar)
		return;
	
	if (false == pChar->IsPC()) 
		return;
	
	if (pChar->m_pkBattleZoneEvent)
		 return;
	
	if(GetEnterStatus() == false)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Combat Zone is currently not open."));
		return;
	}
	
	if(pChar->GetLevel() < MIN_LEVEL_FOR_ENTER)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need minimum level %d to enter in the Combat Zone."), MIN_LEVEL_FOR_ENTER);
		return;
	}
	
	if(g_bChannel == 99){
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't enter in battle zone from this map."));
		return;		
	}
	
    if (pChar->IsDead())
	{
        pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't enter battle zone while you are dead."));
        return;
	}
	
    if (!pChar->CanWarp())
	{
        pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't do that now. Wait 10 seconds and try again."));
        return;
	}
	
	pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Enter in the Combat Zone. Please wait."));
	
	BattleZoneEventInfo* info = AllocEventInfo<BattleZoneEventInfo>();

	info->left_second = 3;
	info->ch = pChar;
	info->isExit = false;
	
	pChar->m_pkBattleZoneEvent = event_create(battle_zone_event, info, 1);
}

void CBattleZone::RequestExit(LPCHARACTER pChar)
{
	if (NULL == pChar)
		return;
	
	if (false == pChar->IsPC()) 
		return;
	
	if (pChar->m_pkBattleZoneEvent)
		 return;
	
    if (!pChar->CanWarp())
	{
        pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't do that now. Wait 10 seconds and try again."));
        return;
	}
	
	bool bCanExitDead = pChar->GetPoint(POINT_TEMP_BATTLE_POINT) == 0 ? true : false;
	int iExitSeconds = 3;
	
	if(pChar->GetPoint(POINT_TEMP_BATTLE_POINT) < 5)
	{
		if(pChar->GetPoint(POINT_TEMP_BATTLE_POINT) > 0)
			iExitSeconds = 15;
	}
	else
	{
		iExitSeconds = 120;
	}
	
    if (pChar->IsDead() && bCanExitDead == true)
	{
        pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't exit battle zone while you are dead."));
        return;
	}
	
	if(iExitSeconds > 15)
	{
		pChar->AddAffect(AFFECT_MOV_SPEED, POINT_MOV_SPEED, 130, AFF_MOV_SPEED_POTION, iExitSeconds, 0, true);
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You've announced your withdrawal, you have to stay standing for 2 minutes."));
		SendNoticeMap(LC_TEXT("A player has announced their withdrawal. Kill them to earn more points!"), BATTLE_ZONE_MAP_INDEX, true);
		TargetOnLeave(pChar, false);
	}
	else
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Exit from the Combat Zone. Please wait."));
	}
	
	BattleZoneEventInfo* info = AllocEventInfo<BattleZoneEventInfo>();

	info->left_second = iExitSeconds;
	info->ch = pChar;
	info->isExit = true;
	
	pChar->m_pkBattleZoneEvent = event_create(battle_zone_event, info, 1);
}

void CBattleZone::CreateRankings(bool isTotal)
{
	char querySelect[256];
	
	if(isTotal)
	{
		snprintf(querySelect, sizeof(querySelect), 
			"SELECT name, empire, score FROM log.battle_total ORDER BY score DESC LIMIT 10;");
	}
	else
	{
		snprintf(querySelect, sizeof(querySelect), 
			"SELECT name, empire, score FROM log.battle_week WHERE WEEKOFYEAR(last_update) = WEEKOFYEAR(NOW()) ORDER BY score DESC LIMIT 10;");
	}
		
	std::auto_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();
	
	if (resSelect && resSelect->uiNumRows > 0)
	{
		TBattleRankingMember tempMember;
		
		for (uint i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			int	col = 0;
			
			tempMember.bPosition = i;
			tempMember.bType = isTotal ? 1 : 0;
			
			strlcpy(tempMember.szName, row[col++], sizeof(tempMember.szName));
			str_to_number(tempMember.bEmpire, row[col++]);
			str_to_number(tempMember.lScore, row[col++]);
	
			ranking_vec.push_back(tempMember);
		}
	}
}

void CBattleZone::RegisterBattleRanking(LPCHARACTER pChar, int iTempPoints)
{
	if (NULL == pChar)
		return;
	
	if (false == pChar->IsPC()) 
		return;
	
	char querySelectWeek[256];
	
	snprintf(querySelectWeek, sizeof(querySelectWeek), 
		"SELECT * FROM log.battle_week WHERE WEEKOFYEAR(last_update) = WEEKOFYEAR(NOW()) AND name = '%s';", pChar->GetName());
		
	std::auto_ptr<SQLMsg> pSelectWeekMsg(DBManager::instance().DirectQuery(querySelectWeek));
	SQLResult* resWeekSelect = pSelectWeekMsg->Get();
	
	if (resWeekSelect && resWeekSelect->uiNumRows > 0)
	{
		DBManager::instance().DirectQuery("UPDATE log.battle_week SET score = score + %d WHERE WEEKOFYEAR(last_update) = WEEKOFYEAR(NOW()) AND name = '%s';", iTempPoints, pChar->GetName());
	}
	else
	{
		DBManager::instance().DirectQuery("REPLACE INTO log.battle_week (name, empire, score, last_update) VALUES ('%s', %d, %d, NOW())", pChar->GetName(), pChar->GetEmpire(), iTempPoints);
	}
	
	char querySelectTotal[256];
	
	snprintf(querySelectTotal, sizeof(querySelectTotal), 
		"SELECT * FROM log.battle_total WHERE name = '%s';", pChar->GetName());
		
	std::auto_ptr<SQLMsg> pSelectTotalMsg(DBManager::instance().DirectQuery(querySelectTotal));
	SQLResult* resTotalSelect = pSelectTotalMsg->Get();
	
	if (resTotalSelect && resTotalSelect->uiNumRows > 0)
	{
		DBManager::instance().DirectQuery("UPDATE log.battle_total SET score = score + %d, last_update = NOW() WHERE name = '%s';", iTempPoints, pChar->GetName());
	}
	else
	{
		DBManager::instance().DirectQuery("REPLACE INTO log.battle_total (name, empire, score, last_update) VALUES ('%s', %d, %d, NOW())", pChar->GetName(), pChar->GetEmpire(), iTempPoints);
	}
}

int CBattleZone::GetRankingPosition(LPCHARACTER pChar)
{
	if (NULL == pChar)
		return 0;
	
	if (false == pChar->IsPC()) 
		return 0;
	
	char querySelect[256];
	
	snprintf(querySelect, sizeof(querySelect), 
		"SELECT name FROM log.battle_week WHERE WEEKOFYEAR(last_update) = WEEKOFYEAR(NOW() - INTERVAL 1 WEEK) ORDER BY score DESC LIMIT 3;");
		
	std::auto_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();
	
	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (uint i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			char szName[CHARACTER_NAME_MAX_LEN + 1];
			strlcpy(szName, row[0], sizeof(szName));

			if (!strcmp(pChar->GetName(), szName))
				return i + 1;
		}
		
		return 0;
	}
	
	return 0;
}

void CBattleZone::Update()
{
	time_t iTime;
	time(&iTime);
	tm* pTimeInfo = localtime(&iTime);
	
	for(DWORD i = 0; i < sizeof(openTimeInfo)/sizeof(openTimeInfo[0]); i++)
	{
		if(openTimeInfo[i].bDay == pTimeInfo->tm_wday)
		{
			if(openTimeInfo[i].bOpenHour == pTimeInfo->tm_hour && openTimeInfo[i].bOpenMinute == pTimeInfo->tm_min && GetEnterStatus() == false)
			{
				bool isEvent = (GetEventMonth() == pTimeInfo->tm_mon && GetEventMonth() < 12 && GetEventDay() == pTimeInfo->tm_mday && GetEventDay() < 32) ? true : false;
				OpenEnter(isEvent);
				break;
			}
			else if(openTimeInfo[i].bCloseHour == pTimeInfo->tm_hour && openTimeInfo[i].bCloseMinute == pTimeInfo->tm_min && GetEnterStatus() == true)
			{
				CloseEnter();
				break;
			}
		}
		
		if(openTimeInfo[i].bDay == pTimeInfo->tm_wday && !s_pkNoticeEvent)
		{
			int tempHour = GetEnterStatus() == false ? openTimeInfo[i].bOpenHour : openTimeInfo[i].bCloseHour;
			int tempMinute = GetEnterStatus() == false ? openTimeInfo[i].bOpenMinute : openTimeInfo[i].bCloseMinute;
			
			int hourP = tempMinute < 15 ? tempHour - 1 : tempHour;
			int minuteP = tempMinute < 15 ? 60 + tempMinute - 15 : tempMinute - 15;
		
			if(pTimeInfo->tm_hour == hourP && pTimeInfo->tm_min == minuteP)
			{
				notice_event_data* info = AllocEventInfo<notice_event_data>();
				info->seconds = 15*60;
				info->isClose = GetEnterStatus();
				
				s_pkNoticeEvent = event_create(notice_event, info, 1);
				break;
			}
		}
	}
}