#include "stdafx.h"
#ifdef __FreeBSD__
#include <md5.h>
#else
#include "../../libthecore/include/xmd5.h"
#endif

#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "dev_log.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "threeway_war.h"
#include "log.h"
#include "../../common/VnumHelper.h"
#ifdef __AUCTION__
#include "auction_manager.h"
#endif
#ifdef __ENABLE_BATTLE_FIELD__
#include "battle_zone.h"
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
#include "MountSystem.h"
#endif
#ifdef NEW_PET_SYSTEM
#include "New_PetSystem.h"
#endif
#include "SupportSystem.h"
#ifdef ENABLE_BIOLOG_SYSTEM
	#include "biolog.h"
#endif
#ifdef NEW_PET_SUMMON
#include "PetSystem.h"
#include "New_PetSystem.h"
#endif

ACMD(do_user_horse_ride)
{
	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding() == false)
	{
		// ���� �ƴ� �ٸ�Ż���� Ÿ���ִ�.
		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�̹� Ż���� �̿����Դϴ�."));
			return;
		}

		if (ch->GetHorse() == NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� ��ȯ���ּ���."));
			return;
		}

		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

ACMD(do_user_horse_back)
{
	if (ch->GetHorse() != NULL)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� �������½��ϴ�."));
	}
	else if (ch->IsHorseRiding() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ ���� ������ �մϴ�."));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� ��ȯ���ּ���."));
	}
}

ACMD(do_user_horse_feed)
{
	// ���λ����� �� ���¿����� �� ���̸� �� �� ����.
	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == NULL)
	{
		if (ch->IsHorseRiding() == false)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� ��ȯ���ּ���."));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ź ���¿����� ���̸� �� �� �����ϴ�."));
		return;
	}

	DWORD dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ %s%s �־����ϴ�."),
				ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName,
				"");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s �������� �ʿ��մϴ�"), ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName);
	}
}

#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int		subcmd;
	int         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo()
	: ch()
	, subcmd( 0 )
	, left_second( 0 )
	{
		::memset( szReason, 0, MAX_REASON_LEN );
	}
};

#ifdef ENABLE_CHANGE_CHANNEL
EVENTINFO(ChangeChannelEventInfo)
{
	DynamicCharacterPtr ch;
	int				channel_number;
	int         	left_second;

	ChangeChannelEventInfo()
	: ch()
	, channel_number( 0 )
	, left_second( 0 )
	{
	}
};
#endif

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int seconds;

	shutdown_event_data()
	: seconds( 0 )
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>( event->info );

	if ( info == NULL )
	{
		sys_err( "shutdown_event> <Factor> Null pointer" );
		return 0;
	}

	int * pSec = & (info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (--*pSec == -10)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--*pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), LC_TEXT("�˴ٿ��� %d�� ���ҽ��ϴ�."), *pSec);
		SendNotice(buf);

		--*pSec;
		return passes_per_sec;
	}
}

void Shutdown(int iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), LC_TEXT("%d�� �� ������ �˴ٿ� �˴ϴ�."), iSec);

	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	if (NULL == ch)
	{
		sys_err("Accept shutdown command from %s.", ch->GetName());
	}
	TPacketGGShutdown p;
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );

	if ( info == NULL )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = NULL;

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
			case SCMD_QUIT:
			case SCMD_PHASE_SELECT:
				{
					TPacketNeedLoginLogInfo acc_info;
					acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

					db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

					LogManager::instance().DetailLoginLog( false, ch );
				}
				break;
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
				if (d)
					d->SetPhase(PHASE_CLOSE);
				break;

			case SCMD_QUIT:
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				break;

			case SCMD_PHASE_SELECT:
				{
					ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

					if (d)
					{
						d->SetPhase(PHASE_SELECT);
					}
				}
				break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d�� ���ҽ��ϴ�."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

#ifdef ENABLE_CHANGE_CHANNEL
EVENTFUNC(change_channel_event)
{
	ChangeChannelEventInfo * info = dynamic_cast<ChangeChannelEventInfo *>( event->info );

	if ( info == NULL )
	{
		sys_err( "change_channel_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (info->left_second <= 0)
	{
		ch->m_pkChangeChannelEvent = NULL;

		ch->ChangeChannel(info->channel_number);
	
		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Canalul se schimba in  %d secunde."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}
#endif

ACMD(do_cmd)
{
	/* RECALL_DELAY
	   if (ch->m_pkRecallEvent != NULL)
	   {
	   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��� �Ǿ����ϴ�."));
	   event_cancel(&ch->m_pkRecallEvent);
	   return;
	   }
	// END_OF_RECALL_DELAY */

	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��� �Ǿ����ϴ�."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�α��� ȭ������ ���� ���ϴ�. ��ø� ��ٸ�����."));
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ ���� �մϴ�. ��ø� ��ٸ�����."));
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ĳ���͸� ��ȯ �մϴ�. ��ø� ��ٸ�����."));
			break;
	}

	int nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
		false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()) &&
	   	(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch		= ch;
				info->subcmd		= subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent	= event_create(timed_event, info, 1);
			}
			break;
	}
}

#ifdef ENABLE_CHANGE_CHANNEL
ACMD(do_change_channel)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	DWORD channel_number = 0;
	str_to_number(channel_number, arg1);
	
	//if (ch->m_pkChangeChannelEvent)
	//{
	//	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Change channel canceled."));
	//	event_cancel(&ch->m_pkChangeChannelEvent);
	//	return;
	//}
	
	if(!ch)
	{
		return;	
	}
	
	if(channel_number == 99 || g_bChannel == 99){
		ch->ChatPacket(CHAT_TYPE_INFO, ("Nu poti schimba ch in aceasta mapa."));
		return;		
	}
	
	if (ch->GetMapIndex() == 64 || ch->GetMapIndex() == 210)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Change Channel> Nu poti schimba canalul in aceasta mapa!");
		return;
	}
	
	if(channel_number == g_bChannel)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, ("Esti deja pe acest canal."));
		return;		
	}
	
    if (ch->IsDead() || ch->GetExchange())
	{
        ch->ChatPacket(CHAT_TYPE_INFO, ("Nu poti schimba canalul acum, asteapta 10 secunde."));
        return;
	}
	
	if(channel_number <= 0 || channel_number > 6)
	{
        ch->ChatPacket(CHAT_TYPE_INFO, ("Acest canal nu este pornit."));
        return;
	}
	
	if (channel_number != 0)
	{
		if (ch->m_pkChangeChannelEvent)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, ("Schimba canal anulat."));
			event_cancel(&ch->m_pkChangeChannelEvent);
			return;
		}
	
		ChangeChannelEventInfo* info = AllocEventInfo<ChangeChannelEventInfo>();
	
		{
			if (ch->IsPosition(POS_FIGHTING))
				info->left_second = 10;
			else
				info->left_second = 3;
		}
	
		info->ch					= ch;
		info->channel_number		= channel_number;
	
		ch->m_pkChangeChannelEvent	= event_create(change_channel_event, info, 1);
	}
}
#endif

ACMD(do_mount)
{
	/*
	   char			arg1[256];
	   struct action_mount_param	param;

	// �̹� Ÿ�� ������
	if (ch->GetMountingChr())
	{
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	return;

	param.x		= atoi(arg1);
	param.y		= atoi(arg2);
	param.vid	= ch->GetMountingChr()->GetVID();
	param.is_unmount = true;

	float distance = DISTANCE_SQRT(param.x - (DWORD) ch->GetX(), param.y - (DWORD) ch->GetY());

	if (distance > 600.0f)
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�� �� ������ ���� ��������."));
	return;
	}

	action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
	return;
	}

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	return;

	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(atoi(arg1));

	if (!tch->IsNPC() || !tch->IsMountable())
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�ű⿡�� Ż �� �����."));
	return;
	}

	float distance = DISTANCE_SQRT(tch->GetX() - ch->GetX(), tch->GetY() - ch->GetY());

	if (distance > 600.0f)
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�� �� ������ ���� Ÿ����."));
	return;
	}

	param.vid		= tch->GetVID();
	param.is_unmount	= false;

	action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
	 */
}

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

#ifdef ENABLE_BIOLOG_SYSTEM
ACMD(do_biolog)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	
	if (!*arg1)
	{
		BiologManager::instance().SendButton(ch);
		return;
	}	
	
	if (!strcmp(arg1, "92_reward_1")) {
		BiologManager::instance().SelectBonusType(ch, "92_reward_1"); return; }		
	if (!strcmp(arg1, "92_reward_2")) {
		BiologManager::instance().SelectBonusType(ch, "92_reward_2"); return; }				
	if (!strcmp(arg1, "92_reward_3")) {
		BiologManager::instance().SelectBonusType(ch, "92_reward_3"); return; }		
	if (!strcmp(arg1, "94_reward_1")) {
		BiologManager::instance().SelectBonusType(ch, "94_reward_1"); return; }			
	if (!strcmp(arg1, "94_reward_2")) {
		BiologManager::instance().SelectBonusType(ch, "94_reward_2"); return; }					
	if (!strcmp(arg1, "94_reward_3")) {
		BiologManager::instance().SelectBonusType(ch, "94_reward_3"); return; }			
}	
#endif

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (NULL == ch->m_pkDeadEvent)
		return;

	int iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);
#ifdef __ENABLE_BATTLE_FIELD__
	bool isBattleMap = CBattleZone::instance().IsBattleZoneMapIndex(ch->GetMapIndex());
	int toWaitSeconds = isBattleMap ? MINMAX(g_nPortalLimitTime, 5 + (ch->GetBattleDeadCount() * 5), 30) : g_nPortalLimitTime;
#endif
#ifdef __ENABLE_BATTLE_FIELD__
	if (subcmd != SCMD_RESTART_TOWN && subcmd != SCMD_RESTART_BATTLE && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
#else
	if (subcmd != SCMD_RESTART_TOWN && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
#endif
	{
		if (!test_server)
		{
			if (ch->IsHack())
			{
				//���� ���ϰ�쿡�� üũ ���� �ʴ´�.
				if (false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
				{
#ifdef __ENABLE_BATTLE_FIELD__
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - (180 - toWaitSeconds));
#else
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - (180 - g_nPortalLimitTime));
#endif
					return;
				}
			}
#ifdef __ENABLE_BATTLE_FIELD__
			if (iTimeToDead > (180 - toWaitSeconds))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - (180 - toWaitSeconds));
				return;
			}
#else
			if (iTimeToDead > 170)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - 170);
				return;
			}
#endif
		}
	}

	//PREVENT_HACK
	//DESC : â��, ��ȯ â �� ��Ż�� ����ϴ� ���׿� �̿�ɼ� �־
	//		��Ÿ���� �߰�
#ifdef __ENABLE_BATTLE_FIELD__
	if (subcmd == SCMD_RESTART_BATTLE)
	{	
		if(!isBattleMap)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can use instant restart just in battle map."));
			return;
		}
		
		if (ch->CountSpecifyItem(50289) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need revive item for instant restart."));
			return;
		}
	}
#endif	
	
	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{
			//����, �����ʿ����� üũ ���� �ʴ´�.
			if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG) ||
			   	false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
#ifdef __ENABLE_BATTLE_FIELD__
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - (180 - toWaitSeconds));
#else
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - (180 - g_nPortalLimitTime));
#endif
				return;
			}
		}

#ifdef __ENABLE_BATTLE_FIELD__
		if (iTimeToDead > (180 - toWaitSeconds))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - (180 - toWaitSeconds));
			return;
		}
#else
		if (iTimeToDead > 173)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����� �� �� �����ϴ�. (%d�� ����)"), iTimeToDead - 173);
			return;
		}
#endif
	}
	//END_PREVENT_HACK

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
	ch->StartRecoveryEvent();

	//FORKED_LOAD
	//DESC: ��Ÿ� ������ ��Ȱ�� �Ұ�� ���� �Ա��� �ƴ� ��Ÿ� ������ ������������ �̵��ϰ� �ȴ�.
	if (1 == quest::CQuestManager::instance().GetEventFlag("threeway_war"))
	{
		if (subcmd == SCMD_RESTART_TOWN || subcmd == SCMD_RESTART_HERE)
		{
			if (true == CThreeWayWar::instance().IsThreeWayWarMapIndex(ch->GetMapIndex()) &&
					false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
				ch->CheckMount();
#endif
#ifdef NEW_PET_SUMMON
				ch->CheckPet();
				ch->CheckBuff();
#endif
				ch->ReviveInvisible(5);
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

				return;
			}

			//����
			if (true == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				if (CThreeWayWar::instance().GetReviveTokenForPlayer(ch->GetPlayerID()) <= 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�������� ��Ȱ ��ȸ�� ��� �Ҿ����ϴ�! ������ �̵��մϴ�!"));
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
				}
				else
				{
					ch->Show(ch->GetMapIndex(), GetSungziStartX(ch->GetEmpire()), GetSungziStartY(ch->GetEmpire()));
				}

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
				ch->CheckMount();
#endif
#ifdef NEW_PET_SUMMON
				ch->CheckPet();
				ch->CheckBuff();
#endif
				ch->ReviveInvisible(5);

				return;
			}
		}
	}
	//END_FORKED_LOAD

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap * pMap = ch->GetWarMap();
		DWORD dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
				{
					sys_log(0, "do_restart: restart town");
					PIXEL_POSITION pos;

					if (CWarMapManager::instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					ch->CheckMount();
#endif
#ifdef NEW_PET_SUMMON
					ch->CheckPet();
					ch->CheckBuff();
#endif
					ch->ReviveInvisible(5);
				}
					break;

				case SCMD_RESTART_HERE:
				{
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					ch->CheckMount();
#endif
#ifdef NEW_PET_SUMMON
					ch->CheckPet();
					ch->CheckBuff();
#endif
					ch->ReviveInvisible(5);
				}
					break;
			}

			return;
		}
	}
	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
		{
			sys_log(0, "do_restart: restart town");
#ifdef __ENABLE_BATTLE_FIELD__
			if(isBattleMap)
			{
				CBattleZone::instance().RestartAtRandomPos(ch);
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			}
			else
			{
				PIXEL_POSITION pos;
	
				if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
					ch->WarpSet(pos.x, pos.y);
				else
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			
				ch->PointChange(POINT_HP, 50 - ch->GetHP());
			}
#else
			PIXEL_POSITION pos;

			if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
#endif
			ch->DeathPenalty(1);
		}
			break;

		case SCMD_RESTART_HERE:
		{
			sys_log(0, "do_restart: restart here");
			ch->RestartAtSamePos();
#ifdef __ENABLE_BATTLE_FIELD__
			if(isBattleMap)
			{
				ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP());
				ch->DeathPenalty(1);
			}
			else
			{
				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(0);
			}
			
#else
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(0);
#endif
			ch->DeathPenalty(0);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			ch->CheckMount();
#endif
#ifdef NEW_PET_SUMMON
			ch->CheckPet();
			ch->CheckBuff();
#endif
			ch->ReviveInvisible(5);
		}

			break;
#ifdef __ENABLE_BATTLE_FIELD__
		case SCMD_RESTART_BATTLE:
		{
			sys_log(0, "do_restart: restart battle");
			ch->RemoveSpecifyItem(50289, 1);
			ch->RestartAtSamePos();
			ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			ch->DeathPenalty(1);
			ch->ReviveInvisible(5);
		}
		break;
#endif
	}
}

#define MAX_STAT g_iStatusPointSetMaxValue

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�а� �߿��� �ɷ��� �ø� �� �����ϴ�."));
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�а� �߿��� �ɷ��� �ø� �� �����ϴ�."));
		return;
	}

	if (ch->GetPoint(POINT_STAT) <= 0)
		return;

	BYTE idx = 0;

	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	// ch->ChatPacket(CHAT_TYPE_INFO, "%s GRP(%d) idx(%u), MAX_STAT(%d), expr(%d)", __FUNCTION__, ch->GetRealPoint(idx), idx, MAX_STAT, ch->GetRealPoint(idx) >= MAX_STAT);
	if (ch->GetRealPoint(idx) >= MAX_STAT)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
	ch->SetPoint(idx, ch->GetPoint(idx) + 1);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

	ch->PointChange(POINT_STAT, -1);
	ch->ComputePoints();
}

#ifdef ENABLE_PVP_ADVANCED
#include <string>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
const char* m_nDuelTranslate[] = /* You can translate here */
{
	"[PvP] Trebuie sa introduci numere.",
	"[PvP] Suma introdusa este mai mica de 0.", 
	"[PvP] Valoare introdusa depaseste limita admina.",
	"[PvP] Nu detii aceasta valoare deyang.",
	"[PvP] Limita de yang v-a fi depasita.",
	"[PvP] Limita de yang v-a fi depasita.",
	"[PvP] Oponentul nu detine aceasta suma de yang.",
	"Actum esti in duel.",
	"Oponentul se afla deja intr-un duel, nu poti trimite alta invitatie.",
	"Vizualizarea echipament este blocata.",
	"Vizualizarea echipament este blocata, acum nimeni nu poate vedea itemele pe care le ai echipat.",
	"Vizualizarea echipament este deblocata, acum oponentul poate vedea itemele pe care le ai echipate.",
	"Vizualizarea echipament este deja deblocata."	
}; 

const char* szTableStaticPvP[] = {BLOCK_CHANGEITEM, BLOCK_BUFF, BLOCK_POTION, BLOCK_RIDE, BLOCK_PET, BLOCK_POLY, BLOCK_PARTY, BLOCK_EXCHANGE_, BET_WINNER, CHECK_IS_FIGHT};

void Duel_SendMessage(LPCHARACTER ch, const char* msg)
{
	if (!ch)
		return;

	std::string textLine;
	
	textLine = msg;
	boost::algorithm::replace_all(textLine, " ", "$");
				 
	char buf[512+1];
	snprintf(buf, sizeof(buf), "BINARY_Duel_SendMessage %s", textLine.c_str());
	ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
}	

ACMD(do_pvp)
{
	if (!ch)
		return;
	
	if (ch->GetArena() != NULL || CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����忡�� ����Ͻ� �� �����ϴ�."));
		return;
	}

	char arg1[256], arg2[256], arg3[256], arg4[256], arg5[256], arg6[256], arg7[256], arg8[256], arg9[256], arg10[256];
	
	pvp_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4), arg5, sizeof(arg5), arg6, sizeof(arg6), arg7, sizeof(arg7), arg8, sizeof(arg8), arg9, sizeof(arg9), arg10, sizeof(arg10));	
	
	DWORD vid = 0;
	str_to_number(vid, arg1);	
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;
	
#ifdef __ENABLE_BATTLE_FIELD__
	if (CBattleZone::instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_ZONE_CANT_FIGHT"));
		return;
	}
#endif

	if (pkVictim->GetArena() != NULL)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ ������Դϴ�."));
		return;
	}
	
	if (ch->GetExchange() || pkVictim->GetExchange())
	{
		CPVPManager::instance().Decline(ch, pkVictim);
		CPVPManager::instance().Decline(pkVictim, ch);
		return;
	}
	
	if (*arg2 && !strcmp(arg2, "accept"))
	{	
		int chA_nBetMoney = ch->GetQuestFlag(szTableStaticPvP[8]);
		int chB_nBetMoney = pkVictim->GetQuestFlag(szTableStaticPvP[8]);

		if ((ch->GetGold() < chA_nBetMoney) || (pkVictim->GetGold() < chB_nBetMoney))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<PvP> Duelul nu poate incepe, a aparut o porblema cu miza.");
			pkVictim->ChatPacket(CHAT_TYPE_INFO, "<PvP> Duelul nu poate incepe, a aparut o porblema cu miza.");
			CPVPManager::instance().Decline(ch, pkVictim);
			CPVPManager::instance().Decline(pkVictim, ch);
			return;
		}

		ch->SetDuel("IsFight", 1);
		pkVictim->SetDuel("IsFight", 1);
		
		if (chA_nBetMoney > 0 && chA_nBetMoney > 0)
		{
			ch->PointChange(POINT_GOLD, - chA_nBetMoney, true);	
			pkVictim->PointChange(POINT_GOLD, - chB_nBetMoney, true);	
		}
		
		CPVPManager::instance().Insert(ch, pkVictim);
		return;
	}
	
	int m_BlockChangeItem = 0, m_BlockBuff = 0, m_BlockPotion = 0, m_BlockRide = 0, m_BlockPet = 0, m_BlockPoly = 0, m_BlockParty = 0, m_BlockExchange = 0, m_BetMoney = 0;
	
	str_to_number(m_BlockChangeItem, arg2);
	str_to_number(m_BlockBuff, arg3);
	str_to_number(m_BlockPotion, arg4);
	str_to_number(m_BlockRide, arg5);
	str_to_number(m_BlockPet, arg6);
	str_to_number(m_BlockPoly, arg7);
	str_to_number(m_BlockParty, arg8);
	str_to_number(m_BlockExchange, arg9);
	str_to_number(m_BetMoney, arg10);
	
	if (!isdigit(*arg2) && !isdigit(*arg3) && !isdigit(*arg4) && !isdigit(*arg5) && !isdigit(*arg6) && !isdigit(*arg7) && !isdigit(*arg8) && !isdigit(*arg9) && !isdigit(*arg10))
	{
		Duel_SendMessage(ch, m_nDuelTranslate[0]);
		return;
	}	
	
	if (m_BetMoney < 0)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[1]);
		return;
	}	
	
	if (m_BetMoney >= GOLD_MAX)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[2]);
		return;
	}
	
	if (ch->GetGold() < m_BetMoney)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[3]);
		return;
	}
	
	if ((ch->GetGold() + m_BetMoney) > GOLD_MAX)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[4]);
		return;
	}
	
	if ((pkVictim->GetGold() + m_BetMoney) > GOLD_MAX)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[5]);		
		return;
	}
	
	if (pkVictim->GetGold() < m_BetMoney)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[6]);
		return;
	}
	
	if (*arg1 && *arg2 && *arg3 && *arg4 && *arg5 && *arg6 && *arg7 && *arg8 && *arg9 && *arg10)
	{
		ch->SetDuel("BlockChangeItem", m_BlockChangeItem);			ch->SetDuel("BlockBuff", m_BlockBuff);
		ch->SetDuel("BlockPotion", m_BlockPotion);					ch->SetDuel("BlockRide", m_BlockRide);
		ch->SetDuel("BlockPet", m_BlockPet);						ch->SetDuel("BlockPoly", m_BlockPoly);	
		ch->SetDuel("BlockParty", m_BlockParty);					ch->SetDuel("BlockExchange", m_BlockExchange);
		ch->SetDuel("BetMoney", m_BetMoney);

		pkVictim->SetDuel("BlockChangeItem", m_BlockChangeItem);	pkVictim->SetDuel("BlockBuff", m_BlockBuff);
		pkVictim->SetDuel("BlockPotion", m_BlockPotion);			pkVictim->SetDuel("BlockRide", m_BlockRide);
		pkVictim->SetDuel("BlockPet", m_BlockPet);					pkVictim->SetDuel("BlockPoly", m_BlockPoly);	
		pkVictim->SetDuel("BlockParty", m_BlockParty);				pkVictim->SetDuel("BlockExchange", m_BlockExchange);
		pkVictim->SetDuel("BetMoney", m_BetMoney);
			
		CPVPManager::instance().Insert(ch, pkVictim); 
	}	
}

ACMD(do_pvp_advanced)
{   
	if (!ch)
		return;

	if (ch->GetArena() != NULL || CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����忡�� ����Ͻ� �� �����ϴ�."));
		return;
	}
	
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

    if (!pkVictim)
        return;

    if (pkVictim->IsNPC())
        return;

#ifdef __ENABLE_BATTLE_FIELD__
	if (CBattleZone::instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_ZONE_CANT_FIGHT"));
		return;
	}
#endif	
	
	if (pkVictim->GetArena() != NULL)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ ������Դϴ�."));
		return;
	}
	
	if (ch->GetQuestFlag(szTableStaticPvP[9]) > 0)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[7]);
		return;
	}
	
	if (pkVictim->GetQuestFlag(szTableStaticPvP[9]) > 0)
	{
		Duel_SendMessage(ch, m_nDuelTranslate[8]);
		return;
	}
	
	int statusEq = pkVictim->GetQuestFlag(BLOCK_EQUIPMENT_);
	
	CGuild * g = pkVictim->GetGuild();

	const char* m_Name = pkVictim->GetName();	
	const char* m_GuildName = "-";
		
	int m_Vid = pkVictim->GetVID();	
	int m_Level = pkVictim->GetLevel();
	int m_PlayTime = pkVictim->GetRealPoint(POINT_PLAYTIME);
	int m_MaxHP = pkVictim->GetMaxHP();
	int m_MaxSP = pkVictim->GetMaxSP();
	
	DWORD m_Race = pkVictim->GetRaceNum();
	
	if (g)
	{ 
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Duel_GetInfo %d %s %s %d %d %d %d %d", m_Vid, m_Name, g->GetName(), m_Level, m_Race, m_PlayTime, m_MaxHP, m_MaxSP);
		
		if (statusEq < 1)
			pkVictim->SendEquipment(ch);
	}
	else
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Duel_GetInfo %d %s %s %d %d %d %d %d", m_Vid, m_Name, m_GuildName, m_Level, m_Race, m_PlayTime, m_MaxHP, m_MaxSP);
		
		if (statusEq < 1)
			pkVictim->SendEquipment(ch);
}

ACMD(do_decline_pvp)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	
	if (!*arg1)
		return;
	
	DWORD vid = 0;
	str_to_number(vid, arg1);
	
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);
	
	if (!pkVictim)
		return;
	
	if (pkVictim->IsNPC())
		return;
	
#ifdef __ENABLE_BATTLE_FIELD__
	if (CBattleZone::instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_ZONE_CANT_FIGHT"));
		return;
	}
#endif	
	
	CPVPManager::instance().Decline(ch, pkVictim);
}

ACMD(do_block_equipment)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));
	
	if (!ch->IsPC() || NULL == ch)
		return;
	
	int statusEq = ch->GetQuestFlag(BLOCK_EQUIPMENT_);
	
	if (!strcmp(arg1, "BLOCK"))
	{	
		if (statusEq > 0)
		{	
			Duel_SendMessage(ch, m_nDuelTranslate[9]);
			return;
		}	
		else
			ch->SetQuestFlag(BLOCK_EQUIPMENT_, 1);
			Duel_SendMessage(ch, m_nDuelTranslate[10]);
	}
	
	if (!strcmp(arg1, "UNBLOCK"))
	{
		if (statusEq == 0)
		{	
			Duel_SendMessage(ch, m_nDuelTranslate[12]);
			return;
		}	
		else	
			ch->SetQuestFlag(BLOCK_EQUIPMENT_, 0);
			Duel_SendMessage(ch, m_nDuelTranslate[11]);
	}
}
#endif

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ��忡 �������� �ʽ��ϴ�."));
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ��� ��ų ������ ������ ������ �����ϴ�."));
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch(vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:
			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
#ifdef __7AND8TH_SKILLS__
			case SKILL_ANTI_PALBANG:
			case SKILL_ANTI_AMSEOP:
			case SKILL_ANTI_SWAERYUNG:
			case SKILL_ANTI_YONGBI:
			case SKILL_ANTI_GIGONGCHAM:
			case SKILL_ANTI_HWAJO:
			case SKILL_ANTI_MARYUNG:
			case SKILL_ANTI_BYEURAK:
#ifdef __WOLFMAN_CHARACTER__
			case SKILL_ANTI_SALPOONG:
#endif
			case SKILL_HELP_PALBANG:
			case SKILL_HELP_AMSEOP:
			case SKILL_HELP_SWAERYUNG:
			case SKILL_HELP_YONGBI:
			case SKILL_HELP_GIGONGCHAM:
			case SKILL_HELP_HWAJO:
			case SKILL_HELP_MARYUNG:
			case SKILL_HELP_BYEURAK:
#ifdef __WOLFMAN_CHARACTER__
			case SKILL_HELP_SALPOONG:
#endif
#endif
				ch->SkillLevelUp(vnum);
				break;
		}
	}
}

//
// @version	05/06/20 Bang2ni - Ŀ�ǵ� ó�� Delegate to CHARACTER class
//
ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

//
// @version	05/06/20 Bang2ni - Ŀ�ǵ� ó�� Delegate to CHARACTER class
//
ACMD(do_safebox_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<â��> �߸��� ��ȣ�� �Է��ϼ̽��ϴ�."));
		return;
	}

	if (!*arg2 || strlen(arg2)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<â��> �߸��� ��ȣ�� �Է��ϼ̽��ϴ�."));
		return;
	}

	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<â��> �߸��� ��ȣ�� �Է��ϼ̽��ϴ�."));
		return;
	}

	int iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<â��> â���� �̹� �����ֽ��ϴ�."));
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10) // 10�ʿ� �ѹ��� ��û ����
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<â��> â���� ������ 10�� �ȿ��� �� �� �����ϴ�."));
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<��Ƽ> ���� ������ ��Ƽ ���� ó���� �� �� �����ϴ�."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<��Ƽ> ���� �ȿ����� ��Ƽ���� ���� �� �����ϴ�."));
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		// party disband
		CPartyManager::instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<��Ƽ> ��Ƽ���� �����̽��ϴ�."));
		//pParty->SendPartyRemoveOneToAll(ch);
		pParty->Quit(ch->GetPlayerID());
		//pParty->SendPartyRemoveAllToOne(ch);
	}
}

#ifdef GIFT_SYSTEM
#include "db.h"
#include <string>
#include <boost/algorithm/string.hpp>

ACMD(do_gift_show)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift_show");
}
bool GetGift(LPCHARACTER ch, DWORD id,bool all=false)
{
	char szSockets[1024] = { '\0' };
	char *tempSockets = szSockets;
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		tempSockets += sprintf(tempSockets, "socket%d", i);

		if (i<ITEM_SOCKET_MAX_NUM - 1)
			tempSockets += sprintf(tempSockets, ",");
	}
	char szAttrs[1024] = { '\0' };
	char *tempAttrs = szAttrs;
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
	{
		if (i < 7)
			tempAttrs += sprintf(tempAttrs, "attrtype%d,attrvalue%d", i, i);
		else
			tempAttrs += sprintf(tempAttrs, "applytype%d,applyvalue%d", i - 7, i - 7);
		if (i<ITEM_ATTRIBUTE_MAX_NUM - 1)
			tempAttrs += sprintf(tempAttrs, ",");
	}
	char query[8192];
	if (!all)
		snprintf(query, sizeof(query), "SELECT id,vnum,count,%s,%s from player_gift where id='%d' and owner_id=%d and status='WAIT'", szSockets, szAttrs, id, ch->GetPlayerID());
	else
		snprintf(query, sizeof(query), "SELECT id,vnum,count,%s,%s from player_gift where owner_id=%d and status='WAIT'", szSockets, szAttrs, ch->GetPlayerID());
	SQLMsg * pkMsg(DBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();
	if (pRes->uiNumRows > 0)
	{
		ch->SetQuestFlag("gift.time", get_global_time()+(1*pRes->uiNumRows)+2);
		MYSQL_ROW row;
		bool force = false;
		while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
		{
			DWORD vnum, socket[ITEM_SOCKET_MAX_NUM], attr[ITEM_ATTRIBUTE_MAX_NUM][2];
			int col = 0;
			long long count = 0;
			str_to_number(id, row[col++]);
			str_to_number(vnum, row[col++]);
			str_to_number(count, row[col++]);
			if (vnum == 1)
			{
#ifndef FULL_YANG
				long long nTotalMoney = ch->GetGold()+count;

				if (GOLD_MAX <= nTotalMoney)
				{
					sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", ch->GetPlayerID(), ch->GetName());
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20?? ��E?�� ??��uC??�� ��oA��?�� ?��?o�Ƣ� ??��?��?��?"));
					return true;
				}
#endif
#ifdef FULL_YANG_OWN
				ch->ChangeGold(count);
#else
				ch->PointChange(POINT_GOLD, count, false);
#endif

			}
			else {
				if (force)
					continue;
				for (int s = 0; s < ITEM_SOCKET_MAX_NUM; s++)
					str_to_number(socket[s], row[col++]);

				for (int a = 0; a < ITEM_ATTRIBUTE_MAX_NUM; a++)
				{
					str_to_number(attr[a][0], row[col++]);
					str_to_number(attr[a][1], row[col++]);
				}
				LPITEM item = ITEM_MANAGER::instance().CreateItem(vnum, count, 0, true);
				if (item)
				{
					for (int s = 0; s < ITEM_SOCKET_MAX_NUM; s++)
						item->SetSocket(s, socket[s], false);
					item->ClearAttribute();
					for (int a = 0; a < ITEM_ATTRIBUTE_MAX_NUM; a++)
						item->SetForceAttribute(a, attr[a][0], attr[a][1]);


					if (item->IsDragonSoul())
					{
						int iEmptyPos = ch->GetEmptyDragonSoulInventory(item);

						if (iEmptyPos != -1)
						{
							item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
						}
						else
						{
							M2_DESTROY_ITEM(item);
							if (!ch->DragonSoul_IsQualified())
							{
								ch->ChatPacket(CHAT_TYPE_INFO, "<Gift> %s #3", LC_TEXT( "UNKNOW_ERROR"));
								force = true;
								continue;
							}
							else {
								ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT( "INVENTORY_FULL_ERROR"));
								force = true;
								continue;
							}
						}
					}
					else if (item->IsUpgradeItem())
					{
						int iEmptyPos = ch->GetEmptyUpgradeInventory(item);

						if (iEmptyPos != -1)
						{
							item->AddToCharacter(ch, TItemPos(UPGRADE_INVENTORY, iEmptyPos));
							ch->ChatPacket(CHAT_TYPE_INFO,("%s a fost adaugat in depozitul special"), item->GetName());
						}
						else
						{
							M2_DESTROY_ITEM(item);
							ch->ChatPacket(CHAT_TYPE_INFO, "Nu ai suficient spatiu in depozitul speecial.");
						}
					}
					else if (item->IsBook())
					{
						int iEmptyPos = ch->GetEmptyBookInventory(item);

						if (iEmptyPos != -1)
						{
							item->AddToCharacter(ch, TItemPos(BOOK_INVENTORY, iEmptyPos));
							ch->ChatPacket(CHAT_TYPE_INFO,("%s a fost adaugat in depozitul special"), item->GetName());
						}
						else
						{
							M2_DESTROY_ITEM(item);
							ch->ChatPacket(CHAT_TYPE_INFO, "Nu ai suficient spatiu in depozitul speecial.");
						}
					}
					else if (item->IsStone())
					{
						int iEmptyPos = ch->GetEmptyStoneInventory(item);

						if (iEmptyPos != -1)
						{
							item->AddToCharacter(ch, TItemPos(STONE_INVENTORY, iEmptyPos));
							ch->ChatPacket(CHAT_TYPE_INFO,("%s a fost adaugat in depozitul special"), item->GetName());
						}
						else
						{
							M2_DESTROY_ITEM(item);
							ch->ChatPacket(CHAT_TYPE_INFO, "Nu ai suficient spatiu in depozitul speecial.");
						}
					}
					else
					{
						int iEmptyPos = ch->GetEmptyInventory(item->GetSize());

						if (iEmptyPos != -1)
						{
							item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
						}
						else
						{
							M2_DESTROY_ITEM(item);
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT( "INVENTORY_FULL_ERROR"));
							force = true;
							continue;
						}
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Gift> %s #4", LC_TEXT( "UNKNOW_ERROR"));
					force = true;
					continue;
				}

			}
			DBManager::instance().DirectQuery("UPDATE player_gift SET status='OK',date_get=NOW() where id=%d;", id);
		}
		if (force)
			return true;
		if (all)
			ch->ChatPacket(CHAT_TYPE_INFO,LC_TEXT("GIFT_ADD_ALL_SUCCESS"));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT( "GIFT_ADD_SUCCESS"));
		ch->SetQuestFlag("gift.time", get_global_time()+2);
		return true;
	}
	return false;
}
ACMD(do_gift_get)
{
	if(ch->GetQuestFlag("gift.time") > get_global_time())
		return;
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	bool full = !isdigit(*arg1);
	DWORD id;
	str_to_number(id, arg1);
	if (GetGift(ch, id,full))
	{
		ch->RefreshGift();
		ch->LoadGiftPage(ch->GetLastGiftPage());
	}

}
ACMD(do_gift_refresh)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
	{
		if (ch->GetGiftPages() > 0)
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gift_info %d", ch->GetGiftPages());
	}
	else{
		int page;
		str_to_number(page, arg1);
		ch->LoadGiftPage(page);
		
	}

	
}
#endif

#ifdef OFFLINE_SHOP
void DeleteShop(DWORD id)
{
	CharacterVectorInteractor i;
	if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(30000, i))
	{
		CharacterVectorInteractor::iterator it = i.begin();

		while (it != i.end()) {
			LPCHARACTER pc = *it++;
			if (pc)
				if (pc->GetRaceNum() == 30000 && pc->GetPrivShop() == id) {
					pc->DeleteMyShop();
					return;
				}


		}
	}
	TPacketShopClose packet;
	packet.shop_id = id;
	packet.pid = 0;
	db_clientdesc->DBPacket(HEADER_GD_SHOP_CLOSE, 0, &packet, sizeof(packet));
}

ACMD(do_close_shop)
{
#ifdef ACCOUNT_SHIELD
	if (ch->IsBlockAccount())
		return;
#endif
	if (ch->IsObserverMode() || ch->GetExchange())
		return;
	DWORD id;
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
	{
		ch->CloseMyShop();
	}else{
		str_to_number(id, arg1);
		char pid[4096];
		sprintf(pid, "and player_id=%d", ch->GetPlayerID());
		if (id > 0)
		{
			SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT player_id,channel from player_shop WHERE id = %d %s", id, (ch->GetGMLevel() >= SHOP_GM_PRIVILEGES ? "" : pid)));
			SQLResult * pRes = pkMsg->Get();
			if (pRes->uiNumRows > 0)
				DeleteShop(id);
		}



	}
}
#include "banword.h"
ACMD(do_set_name_shop)
{
#ifdef ACCOUNT_SHIELD
	if (ch->IsBlockAccount())
		return;
#endif
	if (ch->IsObserverMode() || ch->GetExchange())
		return;
	DWORD id;
	char arg1[256];
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
		return;
	str_to_number(id, arg1);
	char pid[4096];
	sprintf(pid, "and player_id=%d", ch->GetPlayerID());
	std::string m_stShopSign(arg2);
	boost::algorithm::replace_all(m_stShopSign, "\\", " ");
	boost::algorithm::replace_all(m_stShopSign, "%", "%%");
	if (m_stShopSign.length()>SHOP_SIGN_MAX_LEN)
		m_stShopSign.resize(SHOP_SIGN_MAX_LEN);
	if (m_stShopSign.length()<1) {
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???O?i��? ???i�Ƣ� ?��CO��? ��oA�� ??����?����I ��oA��?�� ?�� ?o ??��?��?��?."));
		return;
	}
#ifdef STRING_PROTECTION
	if (CBanwordManager::instance().CheckString(m_stShopSign.c_str(), m_stShopSign.length()) != "")
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???O?i��? ???i�Ƣ� ?��CO��? ��oA�� ??����?����I ��oA��?�� ?�� ?o ??��?��?��?."));
		return;
	}
#else

	if (CBanwordManager::instance().CheckString(m_stShopSign.c_str(), m_stShopSign.length()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???O?i��? ???i�Ƣ� ?��CO��? ��oA�� ??����?����I ��oA��?�� ?�� ?o ??��?��?��?."));
		return;
	}
#endif
	if (id>0)
	{
		SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT id,player_id,channel from player_shop WHERE id = %d %s", id, (ch->GetGMLevel() >= SHOP_GM_PRIVILEGES ? "" : pid)));
		SQLResult * pRes = pkMsg->Get();
		if (pRes->uiNumRows > 0)
		{
			char szName[256];
			DBManager::instance().EscapeString(szName, 256, m_stShopSign.c_str(), m_stShopSign.length());
			DBManager::Instance().DirectQuery("UPDATE player_shop SET name='%s' WHERE id=%d", szName, id);
			ch->LoadPrivShops();
			CharacterVectorInteractor i;
			if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(30000, i))
			{
				CharacterVectorInteractor::iterator it = i.begin();

				while (it != i.end()) {
					LPCHARACTER pc = *it++;
					if (pc)
						if (pc->GetMyShop() && pc->GetPrivShop() == id) {
							pc->SetShopSign(m_stShopSign.c_str());
							return;
						}


				}
			}
			TPacketShopName packet;
			packet.shop_id = id;
			strlcpy(packet.szSign, m_stShopSign.c_str(), sizeof(packet.szSign) - 1);
			db_clientdesc->DBPacket(HEADER_GD_SHOP_NAME, 0, &packet, sizeof(packet));


		}
	}
}

ACMD(do_shop_refresh)
{
	ch->SendShops();
}

ACMD(do_shop_yang)
{
	if (ch->IsObserverMode() || ch->GetExchange())
		return;
	DWORD id;
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	str_to_number(id, arg1);
	if (*arg1)
	{
		SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT gold from player_shop WHERE id = %d and player_id=%d", id,ch->GetPlayerID()));
		SQLResult * pRes = pkMsg->Get();
		if (pRes->uiNumRows>0)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
			{
				long long gold;
				str_to_number(gold, row[0]);
				if (gold >0)
				{
#ifndef FULL_YANG
					long long nTotalMoney = ch->GetGold() + gold;

					if (GOLD_MAX <= nTotalMoney)
					{
						sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", ch->GetPlayerID(), ch->GetName());
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20?? ��E?�� ??��uC??�� ��oA��?�� ?��?o�Ƣ� ??��?��?��?"));
						return;
					}
#endif
#ifdef FULL_YANG_OWN
					ch->ChangeGold(gold);
#else
					ch->PointChange(POINT_GOLD,gold, false);
#endif

					TPrivShop s = ch->GetPrivShopTable(id);
					s.gold = s.gold - gold;
					s.rest_count = s.item_count - 1;
					ch->UpdatePrivShopTable(s.shop_id, s);
					ch->SendShops();
					DBManager::instance().DirectQuery("UPDATE player_shop SET gold=gold - %lld WHERE id = %d", gold, id);
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SHOP_YANG_ADD"));
				}
				}
			}
		}
	}
#include <boost/algorithm/string.hpp>
ACMD(do_shop_update_item)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	std::vector<std::string> args;
	boost::split(args, arg1, boost::is_any_of("|"));
	if (args.size()<3) {
		return;
	}
	DWORD shop_id;
	str_to_number(shop_id, args[1].c_str());
	
	if (ch->IsObserverMode() || ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen() || ch->GetMyShop())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Nu poti face asta acum."));
		return;
	}
	
	bool myShop=false;
	{
		CharacterVectorInteractor i;
		if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(30000, i))
		{
			CharacterVectorInteractor::iterator it = i.begin();
			while (it != i.end()) {
				LPCHARACTER pc = *it++;
				if (pc)
					if (pc->GetRaceNum() == 30000 && pc->GetPrivShop() == shop_id) {
						myShop=true;
						break;
					}


			}
		}
	}
	if(!myShop)
	{
		SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT id from player_shop WHERE id = %d and player_id=%d", shop_id, ch->GetPlayerID()));
		SQLResult * pRes = pkMsg->Get();
		myShop=pRes->uiNumRows>0;
	}
	if (myShop)
	{

		{
			TPacketShopUpdateItem packet;
			packet.shop_id = shop_id;
			packet.shop_locked = true;
			db_clientdesc->DBPacket(HEADER_GD_SHOP_UPDATE_ITEM, 0, &packet, sizeof(packet));
		}
		/*
		/update_shop_item price|%d|%d|%d)
		/update_shop_item remove|%d|%d
		/update_shop_item add|%d|%d|%d|%d|%s

		*/
		if (args.size() == 4)
		{
			DWORD item_id;
			str_to_number(item_id, args[2].c_str());
			long long price;
			str_to_number(price, args[3].c_str());
			if (price <= 0 || item_id <= 0)
				return;
			else
			{
				long long nTotalMoney=price;
				SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT price from player_shop_items where shop_id='%d'", shop_id));
				SQLResult * pRes = pkMsg->Get();
				if (pRes->uiNumRows>0)
				{
					MYSQL_ROW row;
					while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
					{
						long long temp;
						str_to_number(temp,row[0]);
						nTotalMoney+=temp;
					}
				}
				if (GOLD_MAX <= nTotalMoney)
				{
				//	sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", ch->GetPlayerID(), ch->GetName());
					ch->ChatPacket(CHAT_TYPE_INFO, "Ai depasit limita de yang !");
					return;
				}
				else
				{
					SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT id from player_shop_items WHERE id = %d and shop_id=%d", item_id, shop_id));
					SQLResult * pRes = pkMsg->Get();
					if (pRes->uiNumRows > 0)
						DBManager::instance().DirectQuery("UPDATE player_shop_items SET price=%lld WHERE id = %d and shop_id=%d", price, item_id, shop_id);
				}
			}
		}
		else if (args.size() == 3)
		{
			DWORD item_id;
			str_to_number(item_id, args[2].c_str());
			if (item_id <= 0)
				return;
			{

				std::string shop_name(LC_TEXT( "SHOP_NAME"));
				boost::replace_all(shop_name, "#PLAYER_NAME#", ch->GetName());
				boost::replace_all(shop_name, "#ID#", "");


				char szSockets[1024] = { '\0' };
				char *tempSockets = szSockets;
				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
				{
					tempSockets += sprintf(tempSockets, "socket%d", i);

					if (i<ITEM_SOCKET_MAX_NUM - 1)
						tempSockets += sprintf(tempSockets, ",");
				}
				char szAttrs[1024] = { '\0' };
				char *tempAttrs = szAttrs;
				for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
				{
					if (i < 7)
						tempAttrs += sprintf(tempAttrs, "attrtype%d,attrvalue%d", i, i);
					else
						tempAttrs += sprintf(tempAttrs, "applytype%d,applyvalue%d", i - 7, i - 7);
					if (i<ITEM_ATTRIBUTE_MAX_NUM - 1)
						tempAttrs += sprintf(tempAttrs, ",");
				}

				SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT vnum,count,%s,%s from player_shop_items where shop_id='%d' and id=%d", szSockets,szAttrs,shop_id, item_id));
				SQLResult * pRes = pkMsg->Get();
				if (pRes->uiNumRows>0)
				{
					MYSQL_ROW row;
					while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
					{

						int col = 0;
						char query[8192];
						sprintf(query, "INSERT INTO player_gift SET owner_id=%d,reason='%s',`from`=replace(\"%s\",' ','_'),status='WAIT',date_add=NOW()", ch->GetPlayerID(), LC_TEXT( "SHOP_ITEM_REASON"), shop_name.c_str());
						sprintf(query, "%s, vnum='%s'", query, row[col++]);
						sprintf(query, "%s, count='%s'", query, row[col++]);
						for (int s = 0; s < ITEM_SOCKET_MAX_NUM; s++)
							sprintf(query, "%s, socket%d='%s'", query, s, row[col++]);
				
						for (int ia = 0; ia < ITEM_ATTRIBUTE_MAX_NUM; ia++)
						{
							if (ia < 7)
							{
								sprintf(query, "%s, attrtype%d='%s'", query, ia, row[col++]);
								sprintf(query, "%s, attrvalue%d='%s'", query, ia, row[col++]);
							}
							else
							{
								sprintf(query, "%s, applytype%d='%s'", query, ia-7,row[col++]);
								sprintf(query, "%s, applyvalue%d='%s'", query, ia-7,row[col++]);
							}
						}
						SQLMsg * pkMsg(DBManager::instance().DirectQuery(query));
						SQLResult * pRes = pkMsg->Get();
						DWORD gift_id = pRes->uiInsertID;
						if (gift_id > 0)
							GetGift(ch, gift_id);						
						
						DBManager::instance().DirectQuery("delete from player_shop_items where id='%d'", item_id);
						DBManager::instance().DirectQuery("UPDATE player_shop SET item_count=item_count-1 WHERE id = %d", shop_id);
					}
				}

			}
		}

		else if (args.size() == 6)
		{
			BYTE display_pos;
			str_to_number(display_pos, args[2].c_str());

			if (display_pos < 0)
				return;
			WORD pos;
			str_to_number(pos, args[3].c_str());
			if (pos < 0)
				return;

			BYTE window_type;
			str_to_number(window_type, args[4].c_str());
			if (window_type < 0)
				return;

			long long price;

			str_to_number(price, args[5].c_str());
			if (price <= 0)
				return;
		//#ifndef FULL_YANG
			{
				long long nTotalMoney=price;
				SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT price from player_shop_items where shop_id='%d'", shop_id));
				SQLResult * pRes = pkMsg->Get();
				if (pRes->uiNumRows>0)
				{
					MYSQL_ROW row;
					while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
					{
						long long temp;
						str_to_number(temp,row[0]);
						nTotalMoney+=temp;
					}
				}
				if (GOLD_MAX <= nTotalMoney)
				{
					//sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", ch->GetPlayerID(), ch->GetName());
					ch->ChatPacket(CHAT_TYPE_INFO, "Ai depasit limita de yang !");
					return;
				}
			}
		//#endif
			LPITEM item = ch->GetItem(TItemPos(window_type, pos));
			if (item)
			{
				const TItemTable * item_table = item->GetProto();

				if (item_table && (IS_SET(item_table->dwAntiFlags, ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_MYSHOP)))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Nu poti (1)");
					return;
				}

				if (item->IsEquipped())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Ai armura sau un item echipat  ! Nu poti face asta !");
					return;
				}

				if (item->isLocked())
				{
					ch->ChatPacket(CHAT_TYPE_INFO,  "Itemul este blocat");
					return;
				}
				
				if (item->GetTransmutation() != 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Nu poti pune la vanzare un obiect transmutat.");
					return;
				}
				
				
				if (item->GetOwner() != ch)
				{
					return;
				}
#ifdef SOULBIND_SYSTEM
				if (item->IsSoulBind())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT( "You can't sell in private shop item with soul bind."));
					return;
				}
#endif
				char query[1024];
				sprintf(query, "INSERT INTO player_shop_items SET");
				sprintf(query, "%s player_id='%d'", query, ch->GetPlayerID());
				sprintf(query, "%s, shop_id='%d'", query, shop_id);
				sprintf(query, "%s, vnum='%d'", query, item->GetVnum());
				sprintf(query, "%s, count='%d'", query, item->GetCount());
				sprintf(query, "%s, price='%lld'", query, price);
				// sprintf(query, "%s, look='%d'", query, item->GetItemLook());

				sprintf(query, "%s, display_pos='%u'", query, display_pos);
				for (int s = 0; s < ITEM_SOCKET_MAX_NUM; s++)
				{
					sprintf(query, "%s, socket%d='%ld'", query, s, item->GetSocket(s));

				}

				for (int ia = 0; ia < ITEM_ATTRIBUTE_MAX_NUM; ia++)
				{
					const TPlayerItemAttribute& attr = item->GetAttribute(ia);
					if (ia < 7)
					{
						sprintf(query, "%s, attrtype%d='%u'", query, ia, attr.bType);
						sprintf(query, "%s, attrvalue%d='%d'", query, ia, attr.sValue);
					}
					else
					{
						sprintf(query, "%s, applytype%d='%u'", query, ia-7, attr.bType);
						sprintf(query, "%s, applyvalue%d='%d'", query, ia-7, attr.sValue);
					}
				}

				DBManager::instance().DirectQuery(query);
				ITEM_MANAGER::Instance().RemoveItem(item, "Priv shop");
				DBManager::instance().DirectQuery("UPDATE player_shop SET item_count=item_count +1 WHERE id = %d", shop_id);
			}
			else
				return;



		}
		else
			return;
		ch->LoadPrivShops();
		char buf[32];
		sprintf(buf, "refresh_shop_items %d", shop_id);
		interpret_command(ch, buf, strlen(buf));
		CharacterVectorInteractor i;
		if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(30000, i))
		{
			CharacterVectorInteractor::iterator it = i.begin();
			while (it != i.end()) {
				LPCHARACTER pc = *it++;
				if (pc)
					if (pc->GetRaceNum() == 30000 && pc->GetPrivShop() == shop_id) {
						pc->UpdateShopItems();
						return;
					}


			}
		}

		TPacketShopUpdateItem packet;
		packet.shop_id = shop_id;
		packet.shop_locked = false;
		db_clientdesc->DBPacket(HEADER_GD_SHOP_UPDATE_ITEM, 0, &packet, sizeof(packet));
		}
	}

ACMD(do_shop_refresh_items)
{
	if (ch->IsObserverMode() || ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen() || ch->GetMyShop())
	{
		
		ch->ChatPacket(CHAT_TYPE_COMMAND, "shop_item_clear");
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't edit shop while you have opened normal shop."));
		return;
	}

	DWORD id;
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	str_to_number(id, arg1);
	char szSockets[1024] = { '\0' };
	char *tempSockets = szSockets;
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		tempSockets += sprintf(tempSockets, "socket%d", i);

		if (i<ITEM_SOCKET_MAX_NUM - 1)
			tempSockets += sprintf(tempSockets, ",");
	}
	char szAttrs[1024] = { '\0' };
	char *tempAttrs = szAttrs;
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
	{
		if (i < 7)
			tempAttrs += sprintf(tempAttrs, "attrtype%d,attrvalue%d", i, i);
		else
			tempAttrs += sprintf(tempAttrs, "applytype%d,applyvalue%d", i - 7, i - 7);
		if (i<ITEM_ATTRIBUTE_MAX_NUM - 1)
			tempAttrs += sprintf(tempAttrs, ",");
	}
	SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT id,vnum,count,display_pos,price,%s,%s from player_shop_items where shop_id='%d'", szSockets,szAttrs,id));

	SQLResult * pRes = pkMsg->Get();
	BYTE bItemCount = pRes->uiNumRows;
	std::vector<TShopItemTable *> map_shop;
	ch->ChatPacket(CHAT_TYPE_COMMAND, "shop_item_clear");
	if (bItemCount>0)
	{
		bItemCount = 0;
		MYSQL_ROW row;
		int c = 0;
		while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
		{
			int col = 5;
			char attrs[1024] = { '\0' };
			char sockets[1024] = { '\0' };
			for (int i = 0; i<ITEM_SOCKET_MAX_NUM; i++)
				sprintf(sockets, "%s%s%s", sockets, row[col++], (i<ITEM_SOCKET_MAX_NUM-1 ? "|" : ""));
			//col--;

			for (int i = 0; i<ITEM_ATTRIBUTE_MAX_NUM; i++)
				sprintf(attrs, "%s%s,%s%s", attrs, row[col++], row[col++], (i<ITEM_ATTRIBUTE_MAX_NUM-1 ? "|" : ""));
			ch->ChatPacket(CHAT_TYPE_COMMAND, "shop_item %s#%s#%s#%s#%s#%s#%s", row[0], row[1], row[2], row[3], row[4], sockets, attrs);
		}
	}

}
ACMD(do_shop_update)
{
	if (ch->IsObserverMode() || ch->GetExchange())
		return;

	DWORD id;
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	str_to_number(id, arg1);
	if (*arg1)
	{
		SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT player_id from player_shop WHERE id = %d", id));
		SQLResult * pRes = pkMsg->Get();
		if (pRes->uiNumRows>0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Shop %s has been updated", arg1);
			CharacterVectorInteractor i;
			if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(30000, i))
			{
				CharacterVectorInteractor::iterator it = i.begin();

				while (it != i.end()) {
					LPCHARACTER pc = *it++;
					if (pc)
						if (pc->GetRaceNum() == 30000 && pc->GetPrivShop() == id) {
							pc->UpdateShopItems();
							return;
						}


				}
			}
			TPacketShopUpdateItem packet;
			packet.shop_id = id;
			db_clientdesc->DBPacket(HEADER_GD_SHOP_UPDATE_ITEM, 0, &packet, sizeof(packet));
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Shop %s does exists", arg1);
	}
}
ACMD(do_shop_delete)
{
	if (ch->IsObserverMode() || ch->GetExchange())
		return;
	char arg1[256];
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage:");
		ch->ChatPacket(CHAT_TYPE_INFO, "/delete_shop <type> <arg> ");
		ch->ChatPacket(CHAT_TYPE_INFO, "Types:");
		ch->ChatPacket(CHAT_TYPE_INFO, "		shopid - Delete shop using ID");
		ch->ChatPacket(CHAT_TYPE_INFO, "		player - Delete all player shops by player name");
		ch->ChatPacket(CHAT_TYPE_INFO, "Example:");
		ch->ChatPacket(CHAT_TYPE_INFO, "		/delete_shop player Best4ever");
		ch->ChatPacket(CHAT_TYPE_INFO, "		/delete_shop shopid 1");
		return;
	}
	if (!strcmp(arg1, "player"))
	{
		SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT id from player_shop WHERE player_id=(select id from player where name='%s')", arg2));
		SQLResult * pRes = pkMsg->Get();
		if (pRes->uiNumRows>0)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
			{
				DWORD id;
				str_to_number(id, row[0]);
				DeleteShop(id);
			}
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Player %s does have any shop", arg2);
	}
	if (!strcmp(arg1, "shopid"))
	{
		SQLMsg * pkMsg(DBManager::instance().DirectQuery("SELECT id from player_shop WHERE id='%s'", arg2));
		SQLResult * pRes = pkMsg->Get();
		if (pRes->uiNumRows>0)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(pRes->pSQLResult)) != NULL)
			{
				DWORD id;
				str_to_number(id, arg2);
				DeleteShop(id);
			}
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Shop %s does exists", arg2);
	}
}
	
ACMD(do_open_shop)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
}
#else
	
ACMD(do_close_shop)
{
	if (ch->IsObserverMode())
		return;
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
}

#endif

#ifdef NEW_PET_SYSTEM
ACMD(do_CubePetAdd) {

	int pos = 0;
	int invpos = 0;

	const char *line;
	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
		return;
	const std::string& strArg1 = std::string(arg1);
	switch (LOWER(arg1[0]))
	{
	case 'a':	// add cue_index inven_index
	{
		if (0 == arg2[0] || !isdigit(*arg2) ||
			0 == arg3[0] || !isdigit(*arg3))
			return;

		str_to_number(pos, arg2);
		str_to_number(invpos, arg3);

	}
	break;

	default:
		return;
	}

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->SetItemCube(pos, invpos);
	else
		return;

}

ACMD(do_PetSkill) {
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD skillslot = 0;
	str_to_number(skillslot, arg1);
	if (skillslot > 2 || skillslot < 0)
		return;

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->DoPetSkill(skillslot);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Nu aveti un insotitor chemat");
}

ACMD(do_FeedCubePet) {
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD feedtype = 0;
	str_to_number(feedtype, arg1);
	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->ItemCubeFeed(feedtype);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Nu aveti un insotitor chemat");
}

ACMD(do_PetEvo) {

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen()) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[Sistem] Nu poti evolua insotitorul cu o fereastra deschisa!");
		return;
	}
	if (ch->GetNewPetSystem()->IsActivePet()) {

		int it[3][3] = { 
						{55003, 55001, 27992},
						{55004, 55001, 27993},
						{55005, 55001, 27994}
		};
		int ic[3][3] = {{10, 25, 5},
						{20, 50, 10},
						{30, 100, 20}
		};
		int tmpevo = ch->GetNewPetSystem()->GetEvolution();

		if (ch->GetNewPetSystem()->GetLevel() >= 40 && tmpevo == 0 ||
			ch->GetNewPetSystem()->GetLevel() >= 80 && tmpevo == 1 ||
			ch->GetNewPetSystem()->GetLevel() >= 81 && tmpevo == 2) {
			for (int b = 0; b < 1; b++) {
				if (ch->CountSpecifyItem(it[tmpevo][b]) < ic[tmpevo][b]) {
					ch->ChatPacket(CHAT_TYPE_INFO, "[Sistem] Iteme Necesare:");
					for (int c = 0; c < 1; c++) {
						DWORD vnum = it[tmpevo][c];
						ch->ChatPacket(CHAT_TYPE_INFO, "%s X%d", ITEM_MANAGER::instance().GetTable(vnum)->szLocaleName , ic[tmpevo][c]);
					}
					return;
				}
			}
			for (int c = 0; c < 1; c++) {
				ch->RemoveSpecifyItem(it[tmpevo][c], ic[tmpevo][c]);
			}
			ch->GetNewPetSystem()->IncreasePetEvolution();

		}
		else {
			ch->ChatPacket(CHAT_TYPE_INFO, "Nu poti evolua insotitorul!");
			return;
		}

	}else
		ch->ChatPacket(CHAT_TYPE_INFO, "Nu ai un insotitor chemat!");

}

ACMD(do_PetExtraBns) {
	int tmpevo = ch->GetNewPetSystem()->GetEvolution();
	int tmplvl = ch->GetNewPetSystem()->GetLevel();
	if (tmplvl < 50 && tmpevo == 0) {
		ch->ChatPacket(CHAT_TYPE_INFO, "Ai nevoie de nivelul 50 la insotitor");
		ch->ChatPacket(CHAT_TYPE_INFO, "Pentru a primii primul bonus extra");
		ch->ChatPacket(CHAT_TYPE_INFO, "Bonusurile extra vor venii la urmatoarele nivele:");
		ch->ChatPacket(CHAT_TYPE_INFO, "Bonus 1) +10% Tarie impotriva monstrilor (Nivel 50, Evolutia: 1)");
		ch->ChatPacket(CHAT_TYPE_INFO, "Bonus 2) +10% Paguba Abilitatii (Nivel 81, Evolutia 2)");
		ch->ChatPacket(CHAT_TYPE_INFO, "Bonus 3) +10% Tarie impotriva semi-oamenilor (Nivel 81, Evolutia 3)");
		ch->ChatPacket(CHAT_TYPE_INFO, "Bonus 4) +10% Rezistenta la Abilitati (Nivel 120, Evolutia 3)");
	}
	
	if (tmplvl >= 50 || tmplvl <= 80 && tmpevo == 1) {
		ch->ChatPacket(CHAT_TYPE_INFO, "Bonusuri Extra:");
		ch->ChatPacket(CHAT_TYPE_INFO, "1) +10 Tarie impotriva Monstrilor");
	}
	
	if (tmplvl >= 81 || tmplvl <= 100 && tmpevo == 2) {
		ch->ChatPacket(CHAT_TYPE_INFO, "2) +10 Paguba Abiliatii");
	}
	
	if (tmplvl >= 101 || tmplvl <= 119 && tmpevo == 3) {
		ch->ChatPacket(CHAT_TYPE_INFO, "3) +10 Tarie impotriva Semi-Oamenilor");
	}
	
	if (tmplvl >= 120 && tmpevo == 3) {
		ch->ChatPacket(CHAT_TYPE_INFO, "4) +10 Rezistenta la Abilitati");
	}
}

#endif

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

#if defined(ENABLE_AFFECT_POLYMORPH_REMOVE)
ACMD(do_remove_polymorph)
{
	if (!ch)
		return;
	
	if (!ch->IsPolymorphed())
		return;
	
	ch->SetPolymorph(0);
	ch->RemoveAffect(AFFECT_POLYMORPH);
}
#endif

ACMD(do_channel_switch)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));
  	ch->ChatPacket(CHAT_TYPE_INFO, "Ai schimbat canalul pe canal %s ", arg1);

    if (!*arg1)
            return;
    
    int new_ch;
    str_to_number(new_ch, arg1);
    if( new_ch <1 || new_ch >4)   // REPLACE 2 WITH YOUR MAX_CHANNEL 
        return;
    if (!ch->IsPC())
        return;
    ch->ChannelSwitch(new_ch);
}

ACMD(do_war)
{
	//�� ��� ������ ������
	CGuild * g = ch->GetGuild();

	if (!g)
		return;

	//���������� üũ�ѹ�!
	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> �̹� �ٸ� ���￡ ���� �� �Դϴ�."));
		return;
	}

	//�Ķ���͸� �ι�� ������
	char arg1[256], arg2[256];
	DWORD type = GUILD_WAR_TYPE_FIELD; //fixme102 base int modded uint
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type >= GUILD_WAR_TYPE_MAX_NUM)
			type = GUILD_WAR_TYPE_FIELD;
	}

	//����� ������ ���̵� ���µ�
	DWORD gm_pid = g->GetMasterPID();

	//���������� üũ(������ ����常�� ����)
	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ������� ���� ������ �����ϴ�."));
		return;
	}

	//��� ��带 ������
	CGuild * opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> �׷� ��尡 �����ϴ�."));
		return;
	}

	//�������� ���� üũ
	switch (g->GetGuildWarState(opp_g->GetID()))
	{
		case GUILD_WAR_NONE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ��尡 �̹� ���� �� �Դϴ�."));
					return;
				}

				int iWarPrice = KOR_aGuildWarInfo[type].iWarPrice;

				if (g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� �����Ͽ� ������� �� �� �����ϴ�."));
					return;
				}

				if (opp_g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ����� ���� �����Ͽ� ������� �� �� �����ϴ�."));
					return;
				}
			}
			break;

		case GUILD_WAR_SEND_DECLARE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�̹� �������� ���� ����Դϴ�."));
				return;
			}
			break;

		case GUILD_WAR_RECV_DECLARE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ��尡 �̹� ���� �� �Դϴ�."));
					g->RequestRefuseWar(opp_g->GetID());
					return;
				}
			}
			break;

		case GUILD_WAR_RESERVE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> �̹� ������ ����� ��� �Դϴ�."));
				return;
			}
			break;

		case GUILD_WAR_END:
			return;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> �̹� ���� ���� ����Դϴ�."));
			g->RequestRefuseWar(opp_g->GetID());
			return;
	}

	if (!g->CanStartWar(type))
	{
		// ������� �� �� �ִ� ������ ���������ʴ´�.
		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ������ ���ڶ� ������� �� �� �����ϴ�."));
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ������� �ϱ� ���ؼ� �ּ��� %d���� �־�� �մϴ�."), GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}

	// �ʵ��� üũ�� �ϰ� ������ üũ�� ������ �³��Ҷ� �Ѵ�.
	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ����� ���� ������ ���ڶ� ������� �� �� �����ϴ�."));
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ����� ���� ���� �����Ͽ� ������� �� �� �����ϴ�."));
		return;
	}

	do
	{
		if (g->GetMasterCharacter() != NULL)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ����� ������� �������� �ƴմϴ�."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	do
	{
		if (opp_g->GetMasterCharacter() != NULL)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(opp_g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ���� ����� ������� �������� �ƴմϴ�."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	g->RequestDeclareWar(opp_g->GetID(), type);
}

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> ������� ���� ������ �����ϴ�."));
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<���> �׷� ��尡 �����ϴ�."));
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
	ch->DetailLog();
}

ACMD(do_monsterlog)
{
	ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	BYTE mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

#ifdef __ENABLE_BATTLE_FIELD__
	if (CBattleZone::instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
		return;
#endif	
	
	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;
	
	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����忡�� ����Ͻ� �� �����ϴ�."));
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	char answer = LOWER(*arg1);
	// @fixme130 AuthToAdd void -> bool
	bool bIsDenied = answer != 'y';
	bool bIsAdded = MessengerManager::instance().AuthToAdd(ch->GetName(), arg2, bIsDenied); // DENY
	if (bIsAdded && bIsDenied)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg2);

		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ������ ���� ģ�� ����� �ź� ���߽��ϴ�."), ch->GetName());
	}

}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		BYTE flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if(ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* mountSystem = ch->GetMountSystem();
		LPITEM mount = ch->GetWear(WEAR_COSTUME_MOUNT);
		DWORD mobVnum = 0;
		
		if (!mountSystem && !mount)
			return;
		
		if(mount->GetValue(1) != 0)
			mobVnum = mount->GetValue(1);

		if (ch->GetMountVnum())
		{
			if(mountSystem->CountSummoned() == 0)
			{
				mountSystem->Unmount(mobVnum);
			}
		}
		return;
	}
#endif
	if (true == ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
		{
			ch->StopRiding();
		}
	}
	else
	{
		ch->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("�κ��丮�� �� ���� ���� �� �����ϴ�."));
	}

}

ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(NULL);

		if (ch->GetArena() != NULL || ch->GetArenaObserverMode() == true)
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != NULL)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(NULL);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else {
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	if (ch->GetGMLevel() <= GM_PLAYER)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		DWORD vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;
		/*
		   int iSPCost = ch->GetMaxSP() / 3;

		   if (ch->GetSP() < iSPCost)
		   {
		   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���ŷ��� �����Ͽ� �ٸ� ����� ��� �� �� �����ϴ�."));
		   return;
		   }
		   ch->PointChange(POINT_SP, -iSPCost);
		 */
		tch->SendEquipment(ch);
	}
}

ACMD(do_party_request)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����忡�� ����Ͻ� �� �����ϴ�."));
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�̹� ��Ƽ�� ���� �����Ƿ� ���Խ�û�� �� �� �����ϴ�."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
}

ACMD(do_monarch_warpto)
{
	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���ָ��� ��� ������ ����Դϴ�"));
		return;
	}

	//���� ��Ÿ�� �˻�
	if (!ch->IsMCOK(CHARACTER::MI_WARP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d �ʰ� ��Ÿ���� �������Դϴ�."), ch->GetMCLTime(CHARACTER::MI_WARP));
		return;
	}

	//���� �� ��ȯ ���
	const int WarpPrice = 10000;

	//���� ���� �˻�
	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, WarpPrice);
		return;
	}

	int x = 0, y = 0;
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����: warpto <character name>"));
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("Ÿ���� �������Դ� �̵��Ҽ� �����ϴ�"));
				return;
			}

			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�ش� ������ %d ä�ο� �ֽ��ϴ�. (���� ä�� %d)"), pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
				return;
			}

			PIXEL_POSITION pos;

			if (!SECTREE_MANAGER::instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
				ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map (index %d)", pkCCI->lMapIndex);
			else
			{
				//ch->ChatPacket(CHAT_TYPE_INFO, "You warp to (%d, %d)", pos.x, pos.y);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���Է� �̵��մϴ�"), arg1);
				ch->WarpSet(pos.x, pos.y);

				//���� �� �谨
				CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

				//��Ÿ�� �ʱ�ȭ
				ch->SetMC(CHARACTER::MI_WARP);
			}
		}
		else if (NULL == CHARACTER_MANAGER::instance().FindPC(arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
		}

		return;
	}
	else
	{
		if (tch->GetEmpire() != ch->GetEmpire())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ÿ���� �������Դ� �̵��Ҽ� �����ϴ�"));
			return;
		}
		if (!IsMonarchWarpZone(tch->GetMapIndex()))
		{
			ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
			return;
		}
		x = tch->GetX();
		y = tch->GetY();
	}

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���Է� �̵��մϴ�"), arg1);
	ch->WarpSet(x, y);
	ch->Stop();

	//���� �� �谨
	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

	//��Ÿ�� �ʱ�ȭ
	ch->SetMC(CHARACTER::MI_WARP);
}

ACMD(do_monarch_transfer)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����: transfer <name>"));
		return;
	}

	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���ָ��� ��� ������ ����Դϴ�"));
		return;
	}

	//���� ��Ÿ�� �˻�
	if (!ch->IsMCOK(CHARACTER::MI_TRANSFER))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d �ʰ� ��Ÿ���� �������Դϴ�."), ch->GetMCLTime(CHARACTER::MI_TRANSFER));
		return;
	}

	//���� ���� ���
	const int WarpPrice = 10000;

	//���� ���� �˻�
	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, WarpPrice);
		return;
	}


	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�ٸ� ���� ������ ��ȯ�� �� �����ϴ�."));
				return;
			}
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� %d ä�ο� ���� �� �Դϴ�. (���� ä��: %d)"), arg1, pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
				return;
			}
			if (!IsMonarchWarpZone(ch->GetMapIndex()))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� ��ȯ�� �� �����ϴ�."));
				return;
			}

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� ��ȯ�Ͽ����ϴ�."), arg1);

			//���� �� �谨
			CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
			//��Ÿ�� �ʱ�ȭ
			ch->SetMC(CHARACTER::MI_TRANSFER);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�Է��Ͻ� �̸��� ���� ����ڰ� �����ϴ�."));
		}

		return;
	}


	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�ڽ��� ��ȯ�� �� �����ϴ�."));
		return;
	}

	if (tch->GetEmpire() != ch->GetEmpire())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�ٸ� ���� ������ ��ȯ�� �� �����ϴ�."));
		return;
	}
	if (!IsMonarchWarpZone(tch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
		return;
	}
	if (!IsMonarchWarpZone(ch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� ��ȯ�� �� �����ϴ�."));
		return;
	}

	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());

	//���� �� �谨
	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
	//��Ÿ�� �ʱ�ȭ
	ch->SetMC(CHARACTER::MI_TRANSFER);
}

ACMD(do_monarch_info)
{
	if (CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� ����"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			if (n == ch->GetEmpire())
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s����] : %s  �����ݾ� %lld "), EMPIRE_NAME(n), p->name[n], p->money[n]);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s����] : %s  "), EMPIRE_NAME(n), p->name[n]);

		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ����"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s����] : %s  "), EMPIRE_NAME(n), p->name[n]);

		}
	}

}

ACMD(do_elect)
{
	db_clientdesc->DBPacketHeader(HEADER_GD_COME_TO_VOTE, ch->GetDesc()->GetHandle(), 0);
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	BYTE 	empire;
	int 	mapIndex;
	DWORD 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

		x = 0;
		y = 0;
	}

	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void __copy__(const GotoInfo& c_src)
	{
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

ACMD(do_monarch_tax)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: monarch_tax <1-50>");
		return;
	}

	// ���� �˻�
	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���ָ��� ����Ҽ� �ִ� ����Դϴ�"));
		return;
	}

	// ���ݼ���
	int tax = 0;
	str_to_number(tax,  arg1);

	if (tax < 1 || tax > 50)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("1-50 ������ ��ġ�� �������ּ���"));

	quest::CQuestManager::instance().SetEventFlag("trade_tax", tax);

	// ���ֿ��� �޼��� �ϳ�
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ %d %�� �����Ǿ����ϴ�"));

	// ����
	char szMsg[1024];

	snprintf(szMsg, sizeof(szMsg), "������ ������ ������ %d %% �� ����Ǿ����ϴ�", tax);
	BroadcastNotice(szMsg);

	snprintf(szMsg, sizeof(szMsg), "�����δ� �ŷ� �ݾ��� %d %% �� ������ ���Ե˴ϴ�.", tax);
	BroadcastNotice(szMsg);

	// ��Ÿ�� �ʱ�ȭ
	ch->SetMC(CHARACTER::MI_TAX);
}

static const DWORD cs_dwMonarchMobVnums[] =
{
	191, //	��߽�
	192, //	����
	193, //	����
	194, //	ȣ��
	391, //	����
	392, //	����
	393, //	����
	394, //	����
	491, //	��ȯ
	492, //	����
	493, //	����
	494, //	����
	591, //	����ܴ���
	691, //	���� ����
	791, //	�б�����
	1304, // ��������
	1901, // ����ȣ
	2091, // ���հŹ�
	2191, // �Ŵ�縷�ź�
	2206, // ȭ����i
	0,
};

ACMD(do_monarch_mob)
{
	char arg1[256];
	LPCHARACTER	tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���ָ��� ����Ҽ� �ִ� ����Դϴ�"));
		return;
	}

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mmob <mob name>");
		return;
	}

#ifdef ENABLE_MONARCH_MOB_CMD_MAP_CHECK // @warme006
	BYTE pcEmpire = ch->GetEmpire();
	BYTE mapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(ch->GetMapIndex());
	if (mapEmpire != pcEmpire && mapEmpire != 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("�ڱ� ���信���� ����� �� �ִ� ����Դϴ�"));
		return;
	}
#endif

	// ���� �� ��ȯ ���
	const int SummonPrice = 5000000;

	// ���� ��Ÿ�� �˻�
	if (!ch->IsMCOK(CHARACTER::MI_SUMMON))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d �ʰ� ��Ÿ���� �������Դϴ�."), ch->GetMCLTime(CHARACTER::MI_SUMMON));
		return;
	}

	// ���� ���� �˻�
	if (!CMonarch::instance().IsMoneyOk(SummonPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("������ ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, SummonPrice);
		return;
	}

	const CMob * pkMob;
	DWORD vnum = 0;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == NULL)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	DWORD count;

	// ��ȯ ���� �� �˻�
	for (count = 0; cs_dwMonarchMobVnums[count] != 0; ++count)
		if (cs_dwMonarchMobVnums[count] == vnum)
			break;

	if (0 == cs_dwMonarchMobVnums[count])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��ȯ�Ҽ� ���� ���� �Դϴ�. ��ȯ������ ���ʹ� Ȩ�������� �����ϼ���"));
		return;
	}

	tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
			ch->GetMapIndex(),
			ch->GetX() - number(200, 750),
			ch->GetY() - number(200, 750),
			ch->GetX() + number(200, 750),
			ch->GetY() + number(200, 750),
			true,
			pkMob->m_table.bType == CHAR_TYPE_STONE,
			true);

	if (tch)
	{
		// ���� �� �谨
		CMonarch::instance().SendtoDBDecMoney(SummonPrice, ch->GetEmpire(), ch);

		// ��Ÿ�� �ʱ�ȭ
		ch->SetMC(CHARACTER::MI_SUMMON);
	}
}

static const char* FN_point_string(int apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:	return LC_TEXT("�ִ� ������ +%d");
		case POINT_MAX_SP:	return LC_TEXT("�ִ� ���ŷ� +%d");
		case POINT_HT:		return LC_TEXT("ü�� +%d");
		case POINT_IQ:		return LC_TEXT("���� +%d");
		case POINT_ST:		return LC_TEXT("�ٷ� +%d");
		case POINT_DX:		return LC_TEXT("��ø +%d");
		case POINT_ATT_SPEED:	return LC_TEXT("���ݼӵ� +%d");
		case POINT_MOV_SPEED:	return LC_TEXT("�̵��ӵ� %d");
		case POINT_CASTING_SPEED:	return LC_TEXT("��Ÿ�� -%d");
		case POINT_HP_REGEN:	return LC_TEXT("������ ȸ�� +%d");
		case POINT_SP_REGEN:	return LC_TEXT("���ŷ� ȸ�� +%d");
		case POINT_POISON_PCT:	return LC_TEXT("������ %d");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_BLEEDING_PCT:	return LC_TEXT("������ %d");
#endif
		case POINT_STUN_PCT:	return LC_TEXT("���� +%d");
		case POINT_SLOW_PCT:	return LC_TEXT("���ο� +%d");
		case POINT_CRITICAL_PCT:	return LC_TEXT("%d%% Ȯ���� ġ��Ÿ ����");
		case POINT_RESIST_CRITICAL:	return LC_TEXT("����� ġ��Ÿ Ȯ�� %d%% ����");
		case POINT_PENETRATE_PCT:	return LC_TEXT("%d%% Ȯ���� ���� ����");
		case POINT_RESIST_PENETRATE: return LC_TEXT("����� ���� ���� Ȯ�� %d%% ����");
		case POINT_ATTBONUS_HUMAN:	return LC_TEXT("�ΰ��� ���� Ÿ��ġ +%d%%");
		case POINT_ATTBONUS_ANIMAL:	return LC_TEXT("������ ���� Ÿ��ġ +%d%%");
		case POINT_ATTBONUS_ORC:	return LC_TEXT("������ Ÿ��ġ +%d%%");
		case POINT_ATTBONUS_MILGYO:	return LC_TEXT("�б��� Ÿ��ġ +%d%%");
		case POINT_ATTBONUS_UNDEAD:	return LC_TEXT("��ü�� Ÿ��ġ +%d%%");
		case POINT_ATTBONUS_DEVIL:	return LC_TEXT("�Ǹ��� Ÿ��ġ +%d%%");
		case POINT_STEAL_HP:		return LC_TEXT("Ÿ��ġ %d%% �� ���������� ����");
		case POINT_STEAL_SP:		return LC_TEXT("Ÿ��ġ %d%% �� ���ŷ����� ����");
		case POINT_MANA_BURN_PCT:	return LC_TEXT("%d%% Ȯ���� Ÿ�ݽ� ��� ���ŷ� �Ҹ�");
		case POINT_DAMAGE_SP_RECOVER:	return LC_TEXT("%d%% Ȯ���� ���ؽ� ���ŷ� ȸ��");
		case POINT_BLOCK:			return LC_TEXT("����Ÿ�ݽ� ���� Ȯ�� %d%%");
		case POINT_DODGE:			return LC_TEXT("Ȱ ���� ȸ�� Ȯ�� %d%%");
		case POINT_RESIST_SWORD:	return LC_TEXT("�Ѽհ� ��� %d%%");
		case POINT_RESIST_TWOHAND:	return LC_TEXT("��հ� ��� %d%%");
		case POINT_RESIST_DAGGER:	return LC_TEXT("�μհ� ��� %d%%");
		case POINT_RESIST_BELL:		return LC_TEXT("��� ��� %d%%");
		case POINT_RESIST_FAN:		return LC_TEXT("��ä ��� %d%%");
		case POINT_RESIST_BOW:		return LC_TEXT("Ȱ���� ���� %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_RESIST_CLAW:		return LC_TEXT("�μհ� ��� %d%%");
#endif
		case POINT_RESIST_FIRE:		return LC_TEXT("ȭ�� ���� %d%%");
		case POINT_RESIST_ELEC:		return LC_TEXT("���� ���� %d%%");
		case POINT_RESIST_MAGIC:	return LC_TEXT("���� ���� %d%%");
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
		case POINT_RESIST_MAGIC_REDUCTION:	return LC_TEXT("���� ���� %d%%");
#endif
		case POINT_RESIST_WIND:		return LC_TEXT("�ٶ� ���� %d%%");
		case POINT_RESIST_ICE:		return LC_TEXT("�ñ� ���� %d%%");
		case POINT_RESIST_EARTH:	return LC_TEXT("���� ���� %d%%");
		case POINT_RESIST_DARK:		return LC_TEXT("��� ���� %d%%");
		case POINT_REFLECT_MELEE:	return LC_TEXT("���� Ÿ��ġ �ݻ� Ȯ�� : %d%%");
		case POINT_REFLECT_CURSE:	return LC_TEXT("���� �ǵ����� Ȯ�� %d%%");
		case POINT_POISON_REDUCE:	return LC_TEXT("�� ���� %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_BLEEDING_REDUCE:	return LC_TEXT("�� ���� %d%%");
#endif
		case POINT_KILL_SP_RECOVER:	return LC_TEXT("%d%% Ȯ���� ����ġ�� ���ŷ� ȸ��");
		case POINT_EXP_DOUBLE_BONUS:	return LC_TEXT("%d%% Ȯ���� ����ġ�� ����ġ �߰� ���");
		case POINT_GOLD_DOUBLE_BONUS:	return LC_TEXT("%d%% Ȯ���� ����ġ�� �� 2�� ���");
		case POINT_ITEM_DROP_BONUS:	return LC_TEXT("%d%% Ȯ���� ����ġ�� ������ 2�� ���");
		case POINT_POTION_BONUS:	return LC_TEXT("���� ���� %d%% ���� ����");
		case POINT_KILL_HP_RECOVERY:	return LC_TEXT("%d%% Ȯ���� ����ġ�� ������ ȸ��");
//		case POINT_IMMUNE_STUN:	return LC_TEXT("�������� ���� %d%%");
//		case POINT_IMMUNE_SLOW:	return LC_TEXT("�������� ���� %d%%");
//		case POINT_IMMUNE_FALL:	return LC_TEXT("�Ѿ����� ���� %d%%");
//		case POINT_SKILL:	return LC_TEXT("");
//		case POINT_BOW_DISTANCE:	return LC_TEXT("");
		case POINT_ATT_GRADE_BONUS:	return LC_TEXT("���ݷ� +%d");
		case POINT_DEF_GRADE_BONUS:	return LC_TEXT("���� +%d");
		case POINT_MAGIC_ATT_GRADE:	return LC_TEXT("���� ���ݷ� +%d");
		case POINT_MAGIC_DEF_GRADE:	return LC_TEXT("���� ���� +%d");
//		case POINT_CURSE_PCT:	return LC_TEXT("");
		case POINT_MAX_STAMINA:	return LC_TEXT("�ִ� ������ +%d");
		case POINT_ATTBONUS_WARRIOR:	return LC_TEXT("���翡�� ���� +%d%%");
		case POINT_ATTBONUS_ASSASSIN:	return LC_TEXT("�ڰ����� ���� +%d%%");
		case POINT_ATTBONUS_SURA:		return LC_TEXT("���󿡰� ���� +%d%%");
		case POINT_ATTBONUS_SHAMAN:		return LC_TEXT("���翡�� ���� +%d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_ATTBONUS_WOLFMAN:	return LC_TEXT("���翡�� ���� +%d%%");
#endif
		case POINT_ATTBONUS_MONSTER:	return LC_TEXT("���Ϳ��� ���� +%d%%");
		case POINT_MALL_ATTBONUS:		return LC_TEXT("���ݷ� +%d%%");
		case POINT_MALL_DEFBONUS:		return LC_TEXT("���� +%d%%");
		case POINT_MALL_EXPBONUS:		return LC_TEXT("����ġ %d%%");
		case POINT_MALL_ITEMBONUS:		return LC_TEXT("������ ����� %.1f��");
		case POINT_MALL_GOLDBONUS:		return LC_TEXT("�� ����� %.1f��");
		case POINT_MAX_HP_PCT:			return LC_TEXT("�ִ� ������ +%d%%");
		case POINT_MAX_SP_PCT:			return LC_TEXT("�ִ� ���ŷ� +%d%%");
		case POINT_SKILL_DAMAGE_BONUS:	return LC_TEXT("��ų ������ %d%%");
		case POINT_NORMAL_HIT_DAMAGE_BONUS:	return LC_TEXT("��Ÿ ������ %d%%");
		case POINT_SKILL_DEFEND_BONUS:		return LC_TEXT("��ų ������ ���� %d%%");
		case POINT_NORMAL_HIT_DEFEND_BONUS:	return LC_TEXT("��Ÿ ������ ���� %d%%");
//		case POINT_PC_BANG_EXP_BONUS:	return LC_TEXT("");
//		case POINT_PC_BANG_DROP_BONUS:	return LC_TEXT("");
//		case POINT_EXTRACT_HP_PCT:	return LC_TEXT("");
		case POINT_RESIST_WARRIOR:	return LC_TEXT("������ݿ� %d%% ����");
		case POINT_RESIST_ASSASSIN:	return LC_TEXT("�ڰ����ݿ� %d%% ����");
		case POINT_RESIST_SURA:		return LC_TEXT("������ݿ� %d%% ����");
		case POINT_RESIST_SHAMAN:	return LC_TEXT("������ݿ� %d%% ����");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_RESIST_WOLFMAN:	return LC_TEXT("������ݿ� %d%% ����");
#endif		
		default:					return NULL;
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char *buf, size_t bufsiz)
{
	if (NULL == ch || NULL == buf)
		return false;

	CAffect* aff = NULL;
	time_t expire = 0;
	struct tm ltm;
	int	year, mon, day;
	int	offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (NULL == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_global_time())
		return false;

	// set apply string
	offset = snprintf(buf, bufsiz, FN_point_string(aff->bApplyOn), aff->lApplyValue);

	if (offset < 0 || offset >= (int) bufsiz)
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year	= ltm.tm_year + 1900;
	mon		= ltm.tm_mon + 1;
	day		= ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, LC_TEXT(" (������ : %d�� %d�� %d��)"), year, mon, day);

	return true;
}

ACMD(do_costume)
{
	#ifdef __SASH_SYSTEM__
	char buf[768];
	#else
	char buf[512];
	#endif
	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
	#ifdef __SASH_SYSTEM__
	CItem * pSash = ch->GetWear(WEAR_COSTUME_SASH);
	#endif

	ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

		for (int i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.bType)
			{
				snprintf(buf, bufferSize, FN_point_string(attr.bType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	#ifdef __SASH_SYSTEM__
	if (pSash)
	{
		const char * itemName = pSash->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  SASH: %s", itemName);
		for (int i = 0; i < pSash->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pSash->GetAttribute(i);
			if (attr.bType > 0)
			{
				const char * pAttrName = FN_point_string(attr.bType);
				if (pAttrName == NULL)
					continue;
				
				snprintf(buf, sizeof(buf), FN_point_string(attr.bType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pSash->IsEquipped() && arg1[0] == 's')
			ch->UnequipItem(pSash);
	}
	#endif
	
	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}
}

ACMD(do_hair)
{
	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	int	index = 0;
	int	count		= 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int i = 0; i < count; ++i)
	{
		if (index >= INVENTORY_MAX_NUM)
			break;

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s",
						index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

//gift notify quest command
ACMD(do_gift)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

ACMD(do_cube)
{
	if (!ch->CanDoCube())
		return;

	dev_log(LOG_DEB0, "CUBE COMMAND <%s>: %s", ch->GetName(), argument);
	int cube_index = 0, inven_index = 0;
#ifdef ENABLE_SPECIAL_STORAGE
	int inven_type = 0;
	
	char arg1[256], arg2[256], arg3[256], arg4[256];
	two_arguments (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));
#else
	const char *line;
	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));
#endif

	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)
	// /cube r_info     ==> (Client -> Server) ���� NPC�� ���� �� �ִ� ������ ��û
	//					    (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//
	// /cube r_info 3   ==> (Client -> Server) ���� NPC�� ����� �ִ� ������ �� 3��° �������� ����� �� �ʿ��� ������ ��û
	// /cube r_info 3 5 ==> (Client -> Server) ���� NPC�� ����� �ִ� ������ �� 3��° �����ۺ��� ���� 5���� �������� ����� �� �ʿ��� ��� ������ ��û
	//					   (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
		case 'o':	// open
			Cube_open(ch);
			break;

		case 'c':	// close
			Cube_close(ch);
			break;

		case 'l':	// list
			Cube_show_list(ch);
			break;

		case 'a':	// add cue_index inven_index
			{
				if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3))
					return;

				str_to_number(cube_index, arg2);
				str_to_number(inven_index, arg3);
#ifdef ENABLE_SPECIAL_STORAGE
				str_to_number(inven_type, arg4);
				Cube_add_item (ch, cube_index, inven_index, inven_type);
#else
				Cube_add_item (ch, cube_index, inven_index);
#endif
			}
			break;

		case 'd':	// delete
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;

				str_to_number(cube_index, arg2);
				Cube_delete_item (ch, cube_index);
			}
			break;

		case 'm':	// make
			if (0 != arg2[0])
			{
				while (true == Cube_make(ch))
					dev_log (LOG_DEB0, "cube make success");
			}
			else
				Cube_make(ch);
			break;

		default:
			return;
	}
}

ACMD(do_in_game_mall)
{
	if (LC_IsEurope() == true)
	{
		char country_code[3];

		switch (LC_GetLocalType())
		{
			case LC_GERMANY:	country_code[0] = 'd'; country_code[1] = 'e'; country_code[2] = '\0'; break;
			case LC_FRANCE:		country_code[0] = 'f'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_ITALY:		country_code[0] = 'i'; country_code[1] = 't'; country_code[2] = '\0'; break;
			case LC_SPAIN:		country_code[0] = 'e'; country_code[1] = 's'; country_code[2] = '\0'; break;
			case LC_UK:			country_code[0] = 'e'; country_code[1] = 'n'; country_code[2] = '\0'; break;
			case LC_TURKEY:		country_code[0] = 't'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_POLAND:		country_code[0] = 'p'; country_code[1] = 'l'; country_code[2] = '\0'; break;
			case LC_PORTUGAL:	country_code[0] = 'p'; country_code[1] = 't'; country_code[2] = '\0'; break;
			case LC_GREEK:		country_code[0] = 'g'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_RUSSIA:		country_code[0] = 'r'; country_code[1] = 'u'; country_code[2] = '\0'; break;
			case LC_DENMARK:	country_code[0] = 'd'; country_code[1] = 'k'; country_code[2] = '\0'; break;
			case LC_BULGARIA:	country_code[0] = 'b'; country_code[1] = 'g'; country_code[2] = '\0'; break;
			case LC_CROATIA:	country_code[0] = 'h'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_MEXICO:		country_code[0] = 'm'; country_code[1] = 'x'; country_code[2] = '\0'; break;
			case LC_ARABIA:		country_code[0] = 'a'; country_code[1] = 'e'; country_code[2] = '\0'; break;
			case LC_CZECH:		country_code[0] = 'c'; country_code[1] = 'z'; country_code[2] = '\0'; break;
			case LC_ROMANIA:	country_code[0] = 'r'; country_code[1] = 'o'; country_code[2] = '\0'; break;
			case LC_HUNGARY:	country_code[0] = 'h'; country_code[1] = 'u'; country_code[2] = '\0'; break;
			case LC_NETHERLANDS: country_code[0] = 'n'; country_code[1] = 'l'; country_code[2] = '\0'; break;
			case LC_USA:		country_code[0] = 'u'; country_code[1] = 's'; country_code[2] = '\0'; break;
			case LC_CANADA:	country_code[0] = 'c'; country_code[1] = 'a'; country_code[2] = '\0'; break;
			default:
				if (test_server == true)
				{
					country_code[0] = 'd'; country_code[1] = 'e'; country_code[2] = '\0';
				}
				break;
		}

		char buf[512+1];
		char sas[33];
		MD5_CTX ctx;
		const char sas_key[] = "GF9001";

		snprintf(buf, sizeof(buf), "%u%u%s", ch->GetPlayerID(), ch->GetAID(), sas_key);

		MD5Init(&ctx);
		MD5Update(&ctx, (const unsigned char *) buf, strlen(buf));
#ifdef __FreeBSD__
		MD5End(&ctx, sas);
#else
		static const char hex[] = "0123456789abcdef";
		unsigned char digest[16];
		MD5Final(digest, &ctx);
		int i;
		for (i = 0; i < 16; ++i) {
			sas[i+i] = hex[digest[i] >> 4];
			sas[i+i+1] = hex[digest[i] & 0x0f];
		}
		sas[i+i] = '\0';
#endif

		snprintf(buf, sizeof(buf), "mall http://%s/ishop?pid=%u&c=%s&sid=%d&sas=%s",
				g_strWebMallURL.c_str(), ch->GetPlayerID(), country_code, g_server_id, sas);

		ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
	}
}

ACMD(do_support_system)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	
	DWORD items = 0;
	str_to_number(items, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		CSupportSystem* supportSystem = ch->GetSupportSystem();
		for (int i = 0; i < INVENTORY_MAX_NUM; i++)
		{
			LPITEM item = ch->GetInventoryItem(i);
			LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
			if (item != NULL)
				if (item->GetVnum() == items)
				{
					if (item->GetType() == ITEM_COSTUME)
					{
						if (item->IsExchanging())
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Buffi> Nu poti face acest lucru cand negociezi!");
							return;
						}
						if(item->GetSubType() == COSTUME_BODY)
						{
							item->ClearAttribute();
							pSummonItem->SetSocket(1,item->GetVnum());
							item->RemoveFromCharacter();
							break;
						}
						else
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Biffi> Nu poti adauga acest item!");
							return;
						}	
					}
					
				}
		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "<Buffi> Trebuie sa chemi mai intai Buffi-ul pentru a face acest lucru.");
}
ACMD(do_support_system_d)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	
	DWORD items = 0;
	str_to_number(items, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		CSupportSystem* supportSystem = ch->GetSupportSystem();
		for (int i = 0; i < INVENTORY_MAX_NUM; i++)
		{
			LPITEM item = ch->GetInventoryItem(i);
			LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
			if (item != NULL)
				if (item->GetVnum() == items)
				{
					if (item->GetType() == ITEM_COSTUME)
					{
						if (item->IsExchanging())
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Support System> You can not do this when you are exchanging!");
							return;
						}
						if (item->GetSubType() == COSTUME_HAIR)
						{
							item->ClearAttribute();
							pSummonItem->SetSocket(2,item->GetValue(3));
							item->RemoveFromCharacter();
							break;
						}
						else
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Support System> You can not put this item!");
							return;
						}
					}
					
				}
		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "<Support System> You have to call your support for use this function.");
}

// �ֻ���
ACMD(do_dice)
{
	char arg1[256], arg2[256];
	int start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int n = number(start, end);

#ifdef ENABLE_DICE_SYSTEM
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, LC_TEXT("%s���� �ֻ����� ���� %d�� ���Խ��ϴ�. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_DICE_INFO, LC_TEXT("����� �ֻ����� ���� %d�� ���Խ��ϴ�. (%d-%d)"), n, start, end);
#else
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, LC_TEXT("%s���� �ֻ����� ���� %d�� ���Խ��ϴ�. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("����� �ֻ����� ���� %d�� ���Խ��ϴ�. (%d-%d)"), n, start, end);
#endif
}

#ifdef ENABLE_NEWSTUFF
ACMD(do_click_safebox)
{
	ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}
ACMD(do_force_logout)
{
	LPDESC pDesc=DESC_MANAGER::instance().FindByCharacterName(ch->GetName());
	if (!pDesc)
		return;
	pDesc->DelayedDisconnect(0);
}
#endif

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

ACMD(do_ride)
{
    dev_log(LOG_DEB0, "[DO_RIDE] start");
    if (ch->IsDead() || ch->IsStun())
		return;

#ifdef ENABLE_OX_EVENT_RENEWAL
	if (ch->GetMapIndex() == 113)
		return;
#endif
	
	if (ch->IsPolymorphed() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Nu poti folosi un mount atat timp cat esti transformat.");
		return;
	}

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Nu poti folosi un mount in aceasta zona.");
		return;
	}	
	
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if(ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* mountSystem = ch->GetMountSystem();
		LPITEM mount = ch->GetWear(WEAR_COSTUME_MOUNT);
		DWORD mobVnum = 0;
		
		if (!mountSystem && !mount)
			return;
		
		if(mount->GetValue(1) != 0)
			mobVnum = mount->GetValue(1);

		if (ch->GetMountVnum())
		{
			if(mountSystem->CountSummoned() == 0)
			{
				mountSystem->Unmount(mobVnum);
			}
		}
		else
		{
			if(mountSystem->CountSummoned() == 1)
			{
				mountSystem->Mount(mobVnum, mount);
			}
		}
		
		return;
	}
#endif

	if (ch->IsHorseRiding())
	{
		ch->StopRiding();
		return;
	}

	if (ch->GetHorse() != NULL)
	{
	    ch->StartRiding();
	    return;
	}

	for (BYTE i=0; i<INVENTORY_MAX_NUM; ++i)
	{
	    LPITEM item = ch->GetInventoryItem(i);
	    if (NULL == item)
			continue;

		if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_MOUNT)	{
			ch->UseItem(TItemPos (INVENTORY, i));
		    return;
		}
	}

    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� ��ȯ���ּ���."));
}

#ifdef __AUCTION__
// temp_auction
ACMD(do_get_item_id_list)
{
	for (int i = 0; i < INVENTORY_MAX_NUM; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item != NULL)
			ch->ChatPacket(CHAT_TYPE_INFO, "nume : %s id : %d", item->GetProto()->szName, item->GetID());
	}
}

// temp_auction

ACMD(do_enroll_auction)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	char arg4[256];
	two_arguments (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));

	DWORD item_id = strtoul(arg1, NULL, 10);
	BYTE empire = strtoul(arg2, NULL, 10);
	int bidPrice = strtol(arg3, NULL, 10);
	int immidiatePurchasePrice = strtol(arg4, NULL, 10);

	LPITEM item = ITEM_MANAGER::instance().Find(item_id);
	if (item == NULL)
		return;

	AuctionManager::instance().enroll_auction(ch, item, empire, bidPrice, immidiatePurchasePrice);
}

ACMD(do_enroll_wish)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	one_argument (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	DWORD item_num = strtoul(arg1, NULL, 10);
	BYTE empire = strtoul(arg2, NULL, 10);
	int wishPrice = strtol(arg3, NULL, 10);

	AuctionManager::instance().enroll_wish(ch, item_num, empire, wishPrice);
}

ACMD(do_enroll_sale)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	one_argument (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	DWORD item_id = strtoul(arg1, NULL, 10);
	DWORD wisher_id = strtoul(arg2, NULL, 10);
	int salePrice = strtol(arg3, NULL, 10);

	LPITEM item = ITEM_MANAGER::instance().Find(item_id);
	if (item == NULL)
		return;

	AuctionManager::instance().enroll_sale(ch, item, wisher_id, salePrice);
}

// temp_auction
// packet���� ����ϰ� �ϰ�, �̰� �����ؾ��Ѵ�.
ACMD(do_get_auction_list)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	two_arguments (one_argument (argument, arg1, sizeof(arg1)), arg2, sizeof(arg2), arg3, sizeof(arg3));

	AuctionManager::instance().get_auction_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10), strtoul(arg3, NULL, 10));
}
//
//ACMD(do_get_wish_list)
//{
//	char arg1[256];
//	char arg2[256];
//	char arg3[256];
//	two_arguments (one_argument (argument, arg1, sizeof(arg1)), arg2, sizeof(arg2), arg3, sizeof(arg3));
//
//	AuctionManager::instance().get_wish_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10), strtoul(arg3, NULL, 10));
//}
ACMD (do_get_my_auction_list)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_my_auction_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_get_my_purchase_list)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_my_purchase_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_auction_bid)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().bid (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_auction_impur)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().immediate_purchase (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_get_auctioned_item)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_auctioned_item (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_buy_sold_item)
{
	char arg1[256];
	char arg2[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().get_auctioned_item (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_cancel_auction)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_auction (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_cancel_wish)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_wish (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_cancel_sale)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_sale (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_rebid)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().rebid (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_bid_cancel)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().bid_cancel (ch, strtoul(arg1, NULL, 10));
}
#endif

#ifdef SHOP_SEARCH
#include "target.h"
#include "shop.h"
class CFuncShopView
{
private:
	int dwViewRange;

public:
	LPENTITY m_me;
	DWORD dwVnum,dwSkill;
	long long llPrice;
	CFuncShopView(LPENTITY ent, DWORD vnum, long long price,DWORD skill) :
		dwViewRange(VIEW_RANGE + VIEW_BONUS_RANGE),
		m_me(ent), dwVnum(vnum),dwSkill(skill), llPrice(price)
	{
	}

	void operator () (LPENTITY ent)
	{
		/*if (!ent->IsType(ENTITY_OBJECT))
		if (DISTANCE_APPROX(ent->GetX() - m_me->GetX(), ent->GetY() - m_me->GetY()) > dwViewRange)
		return;*/
		if (!m_me->GetDesc())
			return;
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER chMe = (LPCHARACTER)m_me;
			LPCHARACTER chEnt = (LPCHARACTER)ent;
			if (chEnt->GetRaceNum() == 30000 && chEnt->GetMyShop())
			{
				if (llPrice == 0 && dwVnum == 0)
				{
					TargetInfo * pInfo_check = CTargetManager::instance().GetTargetInfo(chMe->GetPlayerID(), TARGET_TYPE_SHOP, chEnt->GetVID());
					if (pInfo_check)
						CTargetManager::Instance().DeleteTarget(chMe->GetPlayerID(), chEnt->IsPrivShop() ? chEnt->GetPrivShop() : chEnt->GetPlayerID(), "SHOP_SEARCH_TARGET");
					return;
				}
				LPSHOP shop = chEnt->GetMyShop();
				if (!shop)
					return;
				if (!shop->SearchItem(dwVnum, llPrice,dwSkill))
					return;

				if (!CTargetManager::instance().GetTargetInfo(chMe->GetPlayerID(), TARGET_TYPE_SHOP, chEnt->GetVID()))
					CTargetManager::Instance().CreateTarget(chMe->GetPlayerID(), chEnt->IsPrivShop() ? chEnt->GetPrivShop() : chEnt->GetPlayerID(), "SHOP_SEARCH_TARGET", TARGET_TYPE_SHOP, chEnt->GetVID(), 0, chEnt->GetMapIndex(), "shop");
				chMe->SetQuestFlag("shop.search", chMe->GetQuestFlag("shop.search") + 1);
			}
		}
	}
};
void ClearShopSearch(LPCHARACTER ch)
{
	CFuncShopView f(ch, 0, 0,0);
	ch->GetSectree()->ForEachAround(f);
}
ACMD(do_search_shop)
{
	if (get_global_time() < ch->GetQuestFlag("search.searchlast")) {
		ch->ChatPacket(CHAT_TYPE_INFO, "You can not use too fast");
		return;
	}
	char arg1[256], arg2[256], arg3[256], arg4[256];
	DWORD vnum = 0;
	long long price = 0;
	DWORD book = 0;

	two_arguments (two_arguments (argument, arg1, sizeof (arg1), arg2, sizeof (arg2)), arg3, sizeof (arg3), arg4, sizeof (arg4));

	if (!*arg1 || !*arg2 || !*arg3 || !isdigit (*arg1) || !isdigit (*arg2)|| !isdigit (*arg3))
		return;
	str_to_number(vnum,arg1);
	str_to_number(price,arg2);
	str_to_number(book,arg3);
	if(vnum == 0)
		ClearShopSearch(ch);
	else{
		ClearShopSearch(ch);
		TItemTable * pTable = ITEM_MANAGER::instance().GetTable(vnum);
		if (!pTable)
			return;
		CFuncShopView f(ch, vnum, price,book);
		ch->SetQuestFlag("shop.search", 0);
		ch->GetSectree()->ForEachAround(f);
		int c = ch->GetQuestFlag("shop.search");
		if (c>0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Found %s in %d shops"), pTable->szLocaleName, c);
			ch->SetQuestFlag("shop.search", 0);
		}
		ch->SetQuestFlag("shop.searchlast", get_global_time() + 2);
	}
}

#endif

#ifdef ENABLE_OFFLINE_SHOP_HIDE
ACMD(do_show_hide_shops)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int option = 0;
	str_to_number(option, arg1);
	
	ch->SetShowOfflineShop(option);
}
#endif

ACMD(do_search_cofres){
	
	int posicion = 0;
	bool g_item_cofres = false;
	const char *line;
	char arg1[256], arg2[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (0 == arg1[0])
		return;

	int slot = atoi(arg2);
	LPITEM	item = ch->GetItem(TItemPos(INVENTORY, slot));

	if(NULL == item){ 
		ch->ChatPacket(CHAT_TYPE_INFO,"[System]Error 000");
		return;
	}
	DWORD dwBoxVnum = item->GetVnum();
	std::vector <DWORD> dwVnums;
	std::vector <DWORD> dwCounts;
	std::vector <LPITEM> item_gets(NULL);
	int count = 0;

	std::vector <DWORD> dwVnums1;
	std::vector <DWORD> dwCounts1;
	std::vector <LPITEM> item_gets1(NULL);
	int count1 = 0;

	const std::string& strArg1 = std::string(arg1);

	if (ch->InformationSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
	{
		g_item_cofres = true;
	}

	if (g_item_cofres == false || dwBoxVnum == 50300){
		ch->ChatPacket(CHAT_TYPE_INFO,"[System]Acest cufar nu contine iteme!");
		return;
	}

	if (strArg1 == "search"){

		if (ch->InformationSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
		{
			for (int i = 0; i < count; i++)
			{
				ch->ChatPacket(CHAT_TYPE_COMMAND, "search_cofre_ids %d %d", dwVnums[i],dwCounts[i]);
			}
			ch->ChatPacket(CHAT_TYPE_COMMAND,"search_cofre_refresh");
		}
	}
	else if(strArg1 == "open"){
		if (item->GetType() == ITEM_TREASURE_BOX)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[System]No puede abrir items que necesiten llaves");
			return;
		}
		if (ch->GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums1, dwCounts1, item_gets1, count1))
		{
			ch->RemoveSpecifyItem(dwBoxVnum, 1);
			for (int i = 0; i < count1; i++){
				switch (dwVnums1[i])
				{
					case CSpecialItemGroup::GOLD:
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ai primit %d yang."), dwCounts1[i]);
						break;
					case CSpecialItemGroup::EXP:
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ai primit %d puncte de experienta."), dwCounts1[i]);
						break;
				}
			}
			ch->ChatPacket(CHAT_TYPE_COMMAND,"search_cofre_refresh_open");
		}

	}
}

ACMD(do_cards)
{
	const char *line;

	char arg1[256], arg2[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	switch (LOWER(arg1[0]))
	{
		case 'o':	// open
			if (isdigit(*arg2))
			{
				DWORD safemode;
				str_to_number(safemode, arg2);
				ch->Cards_open(safemode);
			}
			break;
		case 'p':	// open
			ch->Cards_pullout();
			break;
		case 'e':	// open
			ch->CardsEnd();
			break;
		case 'd':	// open
			if (isdigit(*arg2))
			{
				DWORD destroy_index;
				str_to_number(destroy_index, arg2);
				ch->CardsDestroy(destroy_index);
			}
			break;
		case 'a':	// open
			if (isdigit(*arg2))
			{
				DWORD accpet_index;
				str_to_number(accpet_index, arg2);
				ch->CardsAccept(accpet_index);
			}
			break;
		case 'r':	// open
			if (isdigit(*arg2))
			{
				DWORD restore_index;
				str_to_number(restore_index, arg2);
				ch->CardsRestore(restore_index);
			}
			break;
		default:
			return;
	}
}