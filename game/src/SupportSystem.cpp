#include "stdafx.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "SupportSystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"
#include "affect.h"
#include "skill.h"

extern int passes_per_sec;
EVENTINFO(supportSystem_event_info)
{
	CSupportSystem* psupportSystem;
};

EVENTINFO(supportSystem_event_infoe)
{
	CSupportSystem* psupportSystem;
};

EVENTFUNC(supportSystem_update_event)
{
	supportSystem_event_info* info = dynamic_cast<supportSystem_event_info*>( event->info );
	if ( info == NULL )
	{
		sys_err( "check_speedhack_event> <Factor> Null pointer" );
		return 0;
	}

	CSupportSystem*	psupportSystem = info->psupportSystem;

	if (NULL == psupportSystem)
		return 0;

	
	psupportSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}

EVENTFUNC(buffi_expire_event)
{
	supportSystem_event_infoe* info = dynamic_cast<supportSystem_event_infoe*>(event->info);
	if (info == NULL)
	{
		sys_err("check_speedhack_event> <Factor> Null pointer");
		return 0;
	}

	CSupportSystem*	psupportSystem = info->psupportSystem;

	if (NULL == psupportSystem)
		return 0;


	psupportSystem->UpdateTime();
	return PASSES_PER_SEC(1);
}

const float SUPPORT_COUNT_LIMIT = 3;

///////////////////////////////////////////////////////////////////////////////////////
//  CSupportActor
///////////////////////////////////////////////////////////////////////////////////////

CSupportActor::CSupportActor(LPCHARACTER owner, DWORD vnum)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	// m_dwOptions = options;
	m_dwLastActionTime = 0;
	
	m_dwlevel = 1;
	m_dwExp = 0;
	m_dwNextExp = 0;
	m_dwIntSkill = 3;
	m_dwIntSkillBonus = 0;
	m_dwGiveSuppTime = 0;
	
	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;
	
	m_dwSummonItemVID = 0;
	m_dwSummonItemVnum = 0;
	m_dwduration = 0;
	m_dwExpireSupport = 0;
}

CSupportActor::~CSupportActor()
{
	this->Unsummon();

	m_pkOwner = 0;
}

void CSupportActor::SetName()
{
	std::string supportName = m_pkChar->GetOwner()->GetName();
	std::string supportnames = " - Shaman";
	
	if (m_dwSummonItemVnum == 91010 || m_dwSummonItemVnum == 91011)
	{
		supportName+=" - Buffi";
	}
	else if (m_dwSummonItemVnum == 91012 || m_dwSummonItemVnum == 91013)
	{
		supportName+=" - Healer";
	}
	
	if (true == IsSummoned())
		m_pkChar->SetName(supportName);
}

void CSupportActor::SetNextExp(int nextExp)
{	
	m_dwNextExp = nextExp;
}

void CSupportActor::SetIntSkill()
{	
	if(GetLevel()*1 >= 220)
		m_dwIntSkill = 220;
	else
		m_dwIntSkill = GetLevel()*1;
	
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if(pSummonItem != NULL)
	pSummonItem->SetForceAttribute(1, 0, m_dwIntSkill);
}

int CSupportActor::GetIntSkill()
{
	int k=0;
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if(pSummonItem != NULL)
		k = pSummonItem->GetAttributeValue(1) + pSummonItem->GetAttributeValue(2);	
	
	return k;	
}
void CSupportActor::SetLevel(DWORD level)
{
	m_pkChar->SetLevel(static_cast<char>(level));
	m_dwlevel = level;
	SetNextExp(m_pkChar->SupportGetNextExp());
	SetIntSkill();
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if(pSummonItem != NULL)
		pSummonItem->SetForceAttribute(0, 0, m_dwlevel);

}
void CSupportActor::SetCostume()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if(pSummonItem->GetSocket(1) == 0)
		m_pkChar->SetSupportArmor(armour_support_table[GetLevel()]);
	else
		m_pkChar->SetSupportArmor(pSummonItem->GetSocket(1));
}

void CSupportActor::SetHair()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if(pSummonItem != NULL)
	{
		if(pSummonItem->GetSocket(2) != 0)
		{
			m_pkChar->SetSupportHair(pSummonItem->GetSocket(2));
		}
	}
}
void CSupportActor::SetWeapon()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	m_pkChar->SetSupportWeapon(weapon_support_table[GetLevel()]);
}

void CSupportActor::SetExp(DWORD exp)
{
	if (exp < 0)
		exp = MAX(m_dwExp - exp, 0);
	
	if(GetExp() + exp >= GetNextExp())
	{
		SetLevel(GetLevel() + 1);
		m_pkChar->UpdatePacket();
		m_dwExp = 0;
		m_pkChar->SetExp(0);
		return;

	}
	this->SetCostume();
	this->SetHair();
	this->SetWeapon();
	m_dwExp += exp;
	m_pkChar->SetExp(m_dwExp);
}

bool CSupportActor::Mount()
{
	if (0 == m_pkOwner)
		return false;

	if (true == HasOption(ESupportOption_Mountable))
		m_pkOwner->MountVnum(m_dwVnum);

	return m_pkOwner->GetMountVnum() == m_dwVnum;;
}

void CSupportActor::UpdateTime()
{
	m_dwExpireSupport += 1;
	if (m_dwExpireSupport >= 60) {
		m_dwduration -= 1;
		m_dwExpireSupport = 0;
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
		if (pSummonItem != NULL){			
			pSummonItem->SetForceAttribute(3, 0, m_dwduration);
		}
	}

}

void CSupportActor::Unmount()
{
	if (0 == m_pkOwner)
		return;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
}

void CSupportActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
		this->ClearBuff();

		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
		if (pSummonItem != NULL){
			pSummonItem->SetForceAttribute(0, 0, m_dwlevel);
			pSummonItem->SetSocket(0, m_dwExp);
			pSummonItem->SetForceAttribute(1, 0, m_dwIntSkill);
			// pSummonItem->Lock(false);
		}

		this->SetSummonItem(NULL);
		if (NULL != m_pkOwner)
			m_pkOwner->ComputePoints();

		if (NULL != m_pkChar)
			M2_DESTROY_CHARACTER(m_pkChar);

		m_pkChar = 0;
		m_dwVID = 0;
		m_dwlevel = 1;
		m_dwExp = 0;
		m_dwNextExp = 0;
		m_dwIntSkill = 0;
		m_dwIntSkillBonus = 0;
		m_dwExpireSupport = 0;
	}
}


DWORD CSupportActor::Summon(LPITEM pSummonItem, bool bSpawnFar)
{
	long x = m_pkOwner->GetX();
	long y = m_pkOwner->GetY();
	long z = m_pkOwner->GetZ();
	
	if (true == bSpawnFar)
	{
		x += (number(0, 1) * 2 - 1) * number(2000, 2500);
		y += (number(0, 1) * 2 - 1) * number(2000, 2500);
	}
	else
	{
		x += number(-100, 100);
		y += number(-100, 100);
	}

	if (0 != m_pkChar)
	{
		m_pkChar->Show (m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkChar->GetVID();

		return m_dwVID;
	}
	
	m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(
				m_dwVnum, 
				m_pkOwner->GetMapIndex(), 
				x, y, z,
				false, (int)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CSupportSystem::Summon] Failed to summon the shaman. (vnum: %d)", m_dwVnum);
		return 0;
	}
	
	m_pkChar->SetSupport();
	
	m_pkChar->SetOwner(m_pkOwner);
	m_pkChar->SetEmpire(m_pkOwner->GetEmpire());

	m_dwVID = m_pkChar->GetVID();
	
	m_dwlevel = pSummonItem->GetAttributeValue(0);
	m_dwGiveSuppTime = pSummonItem->GetAttributeValue(3);
	m_dwExp = pSummonItem->GetSocket(0);
	m_dwIntSkill = pSummonItem->GetAttributeValue(1);
	m_dwIntSkillBonus = pSummonItem->GetAttributeValue(2);
	this->SetSummonItem(pSummonItem);
	// pSummonItem->Lock(true);
	this->SetCostume();
	this->SetHair();
	this->SetWeapon();
	
	if(m_dwGiveSuppTime == 0)
	{
		int m_dwduration = number(1, 1) * 24 * 60;
		m_dwGiveSuppTime = 1;
		pSummonItem->SetForceAttribute(3, 0, m_dwduration);
	}
	else
		m_dwduration = pSummonItem->GetAttributeValue(3);
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "Shamanul tau are o durabilitate de %d ore", m_dwduration /60);
	
	if(m_dwlevel == 0)
	{
		m_dwlevel = 1;
		m_dwExp = 0;
		this->SetLevel(1);
	}
	else
		this->SetLevel(m_dwlevel);

	m_dwNextExp = m_pkChar->SupportGetNextExp();
	
	this->SetName();
	
	std::string m_speciality;
	if (m_dwSummonItemVnum == 91010 || m_dwSummonItemVnum == 91011)
	{
		m_speciality = "Dragon";
	}
	else if (m_dwSummonItemVnum == 91012 || m_dwSummonItemVnum == 91013)
	{
		m_speciality = "Vindecare";
	}
	else
	{
		m_speciality = "/";
	}
	m_pkOwner->ComputePoints();
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);
	m_pkChar->UpdatePacket();
	return m_dwVID;
}
void CSupportActor::RefreshCostume()
{
	SetCostume();
	SetHair();
	SetWeapon();
	m_pkChar->UpdatePacket();
}
void CSupportActor::UpdatePacketsupportActor()
{
	m_pkChar->UpdatePacket();
}
bool CSupportActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	float fDist = number(fMinDist, fMaxDist);
	float r = (float)number (0, 359);
	float dest_x = GetOwner()->GetX() + fDist * cos(r);
	float dest_y = GetOwner()->GetY() + fDist * sin(r);

	m_pkChar->SetNowWalking(true);
	m_pkChar->UpdatePacket();
	this->SetCostume();
	this->SetHair();
	this->SetWeapon();
	if (!m_pkChar->IsStateMove() && m_pkChar->Goto(dest_x, dest_y))
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();

	return true;
}

bool CSupportActor::_UpdateFollowAI()
{
	if (0 == m_pkChar->m_pkMobData)
	{
		return false;
	}
	
	if (0 == m_originalMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);

		if (0 != mobData)
			m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
	}
	float	START_FOLLOW_DISTANCE = 300.0f;		
	float	START_RUN_DISTANCE = 400.0f;		

	float	RESPAWN_DISTANCE = 4500.f;			
	int		APPROACH = 533;						

	bool bDoMoveAlone = true;					
	bool bRun = false;							

	DWORD currentTime = get_dword_time();

	long ownerX = m_pkOwner->GetX();		long ownerY = m_pkOwner->GetY();
	long charX = m_pkChar->GetX();			long charY = m_pkChar->GetY();

	float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f;
		float fx = -APPROACH * cos(fOwnerRot);
		float fy = -APPROACH * sin(fOwnerRot);
		if (m_pkChar->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
		{
			m_pkChar->UpdatePacket();
			this->SetCostume();
			this->SetHair();
			this->SetWeapon();
			return true;
		}
	}
	
	
	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if( fDist >= START_RUN_DISTANCE)
		{
			bRun = true;
		}

		m_pkChar->SetNowWalking(!bRun);
		
		Follow(APPROACH);
		m_pkChar->UpdatePacket();
		this->SetCostume();
		this->SetHair();
		this->SetWeapon();
		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else 
		m_pkChar->UpdatePacket();
		this->SetCostume();
		this->SetHair();
		this->SetWeapon();
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	return true;
}

bool CSupportActor::Update(DWORD deltaTime)
{
	bool bResult = true;
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	m_dwduration = pSummonItem->GetAttributeValue(3);
	if (m_dwduration <= 0 || m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead() || (IsSummoned() && m_dwduration <= 0)) 
		|| NULL == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		this->Unsummon();
		return true;
	}
	if(get_dword_time() - this->GetLastExpUpdate() >= 3000)
	{
		this->UpdateItemExp();
		m_pkChar->SetSupportInt(this->GetIntSkill());
		this->SetLastExpUpdate(get_dword_time());
	}
	
	this->UseSkill();
	
	if (this->IsSummoned())
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}
void CSupportActor::UseSkill()
{
	if (m_dwSummonItemVnum == 91010 || m_dwSummonItemVnum == 91011)
	{
		if(m_pkChar->GetOwner()->IsAffectFlag(AFF_HOSIN) == false && get_dword_time() - this->GetLastSkillTime() >= 9000)
		{
			m_pkChar->ComputeSkill(94,m_pkChar->GetOwner(),40);
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_HOSIN);
		}
		else if(m_pkChar->GetOwner()->IsAffectFlag(AFF_GICHEON) == false && get_dword_time() - this->GetLastSkillTime() >= 18000)
		{
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->ComputeSkill(96,m_pkChar->GetOwner(),40);		
			m_pkChar->SendSupportSkillPacket(SKILL_GICHEON);
		}
		// else if(m_pkChar->GetOwner()->IsAffectFlag(AFF_BOHO) == false && get_dword_time() - this->GetLastSkillTime() >= 32000)
		// {
			// m_pkChar->ComputeSkill(95,m_pkChar->GetOwner(),40);		
			// this->SetLastSkillTime(get_dword_time());
			// m_pkChar->SendSupportSkillPacket(SKILL_REFLECT);
		// }

	}
	else if(m_dwSummonItemVnum == 91012 || m_dwSummonItemVnum == 91013)
	{
		if(m_pkChar->GetOwner()->IsAffectFlag(AFF_JEUNGRYEOK) == false && get_dword_time() - this->GetLastSkillTime() >= 9000)
		{
			m_pkChar->ComputeSkill(111,m_pkChar->GetOwner(),40);
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_JEUNGRYEOK);
		}
		else if(m_pkChar->GetOwner()->IsAffectFlag(AFF_KWAESOK) == false && get_dword_time() - this->GetLastSkillTime() >= 18000)
		{
			m_pkChar->ComputeSkill(110,m_pkChar->GetOwner(),40);		
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_KWAESOK);
		}
		// else if(m_pkChar->GetOwner()->GetHPPct() <= 50 && get_dword_time() - this->GetLastSkillTime() >= 3000)
		else if(m_pkChar->GetOwner()->GetHPPct() <= 50 && get_dword_time() - this->GetLastSkillTime() >= 300000)
		{
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->ComputeSkill(109,m_pkChar->GetOwner(),40);		
			m_pkChar->SendSupportSkillPacket(SKILL_JEONGEOP);
		}

	}
}
void CSupportActor::UpdateItemExp()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	m_dwIntSkillBonus = pSummonItem->GetAttributeValue(2);
	if (pSummonItem != NULL)
	{
		pSummonItem->SetSocket(0, m_dwExp);
		pSummonItem->SetForceAttribute(0, 0, m_dwlevel);
		pSummonItem->SetForceAttribute(1, 0, m_dwIntSkill + m_dwIntSkillBonus);
	}
}

bool CSupportActor::Follow(float fMinDistance)
{
	if( !m_pkOwner || !m_pkChar) 
		return false;

	float fOwnerX = m_pkOwner->GetX();
	float fOwnerY = m_pkOwner->GetY();

	float fSupportX = m_pkChar->GetX();
	float fSupportY = m_pkChar->GetY();

	float fDist = DISTANCE_SQRT(fOwnerX - fSupportX, fOwnerY - fSupportY);
	if (fDist <= fMinDistance)
		return false;

	m_pkChar->SetRotationToXY(fOwnerX, fOwnerY);

	float fx, fy;

	float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);
	
	if (!m_pkChar->Goto((int)(fSupportX+fx+0.5f), (int)(fSupportY+fy+0.5f)) )
		return false;

	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

void CSupportActor::SetSummonItem (LPITEM pItem)
{
	if (NULL == pItem)
	{
		m_dwSummonItemVID = 0;
		m_dwSummonItemVnum = 0;
		return;
	}

	m_dwSummonItemVID = pItem->GetVID();
	m_dwSummonItemVnum = pItem->GetVnum();
}

void CSupportActor::GiveBuff()
{
	if (34004 == m_dwVnum || 34009 == m_dwVnum)
	{
		if (NULL == m_pkOwner->GetDungeon())
		{
			return;
		}
	}
	return ;
}

void CSupportActor::ClearBuff()
{
	if (NULL == m_pkOwner)
		return ;
	// TItemTable* item_proto = ITEM_MANAGER::instance().GetTable(m_dwSummonItemVnum);
	// if (NULL == item_proto)
		// return;
	// for (int i = 0; i < ITEM_APPLY_MAX_NUM; i++)
	// {
		// if (item_proto->aApplies[i].bType == APPLY_NONE)
			// continue;
		// m_pkOwner->ApplyPoint(item_proto->aApplies[i].bType, -item_proto->aApplies[i].lValue);
	// }
	
	int m_nTableSkill[] = {94,95,96,109,110,111};		
	for (int i = 0; i < _countof(m_nTableSkill); i++)
	{
		m_pkOwner->RemoveAffect(m_nTableSkill[i]);
	}
	
	return ;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CSupportSystem
///////////////////////////////////////////////////////////////////////////////////////

CSupportSystem::CSupportSystem(LPCHARACTER owner)
{

	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CSupportSystem::~CSupportSystem()
{
	Destroy();
}

void CSupportSystem::Destroy()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor)
		{
			delete supportActor;
		}
	}
	event_cancel(&m_pksupportSystemUpdateEvent);
	event_cancel(&m_pksupportSystemExpireEvent);
	m_supportActorMap.clear();
}

void CSupportSystem::UpdateTime()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor && supportActor->IsSummoned())
		{
			supportActor->UpdateTime();
		}
	}
}

/// 펫 시스템 업데이트. 등록된 펫들의 AI 처리 등을 함.
bool CSupportSystem::Update(DWORD deltaTime)
{
	bool bResult = true;

	DWORD currentTime = get_dword_time();	
	
	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;
	
	std::vector <CSupportActor*> v_garbageActor;

	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor && supportActor->IsSummoned())
		{
			LPCHARACTER pSupport = supportActor->GetCharacter();
			
			if (NULL == CHARACTER_MANAGER::instance().Find(pSupport->GetVID()))
			{
				v_garbageActor.push_back(supportActor);
			}
			else
			{
				bResult = bResult && supportActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CSupportActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeleteSupport(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CSupportSystem::DeleteSupport(DWORD mobVnum)
{
	TsupportActorMap::iterator iter = m_supportActorMap.find(mobVnum);

	if (m_supportActorMap.end() == iter)
	{
		sys_err("[CSupportSystem::DeleteSupport] Can't find shaman on my list (VNUM: %d)", mobVnum);
		return;
	}

	CSupportActor* supportActor = iter->second;

	if (0 == supportActor)
		sys_err("[CSupportSystem::DeleteSupport] Null Pointer (supportActor)");
	else
		delete supportActor;

	m_supportActorMap.erase(iter);	
}

/// 관리 목록에서 펫을 지움
void CSupportSystem::DeleteSupport(CSupportActor* supportActor)
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		if (iter->second == supportActor)
		{
			delete supportActor;
			m_supportActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CSupportSystem::DeleteSupport] Can't find supportActor(0x%x) on my list(size: %d) ", supportActor, m_supportActorMap.size());
}

void CSupportSystem::Unsummon(DWORD vnum, bool bDeleteFromList)
{
	CSupportActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CSupportSystem::GetByVnum(%d)] Null Pointer (supportActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeleteSupport(actor);

	bool bActive = false;
	for (TsupportActorMap::iterator it = m_supportActorMap.begin(); it != m_supportActorMap.end(); it++)
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pksupportSystemUpdateEvent);
		event_cancel(&m_pksupportSystemExpireEvent);
		m_pksupportSystemUpdateEvent = NULL;
		m_pksupportSystemExpireEvent = NULL;
	}
}

void CSupportSystem::SetExp(int iExp)
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				supportActor->SetExp(iExp);
				break;
			}
		}
	}
}

int CSupportSystem::GetLevel()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				return supportActor->GetLevel();
			}
		}
	}
	return -1;
}

int CSupportSystem::GetExp()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				return supportActor->GetExp();
			}
		}
	}
	return 0;
}
bool CSupportSystem::IsActiveSupport()
{
	bool state = false;
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				state = true;
				break;
			}			
		}
	}
	return state;

}

CSupportActor* CSupportSystem::GetActiveSupport()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				return supportActor;
			}			
		}
	}
}

CSupportActor* CSupportSystem::Summon(DWORD mobVnum, LPITEM pSummonItem, bool bSpawnFar)
{
	CSupportActor* supportActor = this->GetByVnum(mobVnum);

	if (0 == supportActor)
	{
		supportActor = M2_NEW CSupportActor(m_pkOwner, mobVnum);
		m_supportActorMap.insert(std::make_pair(mobVnum, supportActor));
	}

	DWORD supportVID = supportActor->Summon(pSummonItem, bSpawnFar);

	if (NULL == m_pksupportSystemUpdateEvent)
	{
		supportSystem_event_info* info = AllocEventInfo<supportSystem_event_info>();

		info->psupportSystem = this;

		m_pksupportSystemUpdateEvent = event_create(supportSystem_update_event, info, PASSES_PER_SEC(1) / 4);	// 0.25초	
	}

	if (NULL == m_pksupportSystemExpireEvent)
	{
		supportSystem_event_infoe* infoe = AllocEventInfo<supportSystem_event_infoe>();

		infoe->psupportSystem = this;

		m_pksupportSystemExpireEvent = event_create(buffi_expire_event, infoe, PASSES_PER_SEC(1) );
	}
	
	
	return supportActor;
}


CSupportActor* CSupportSystem::GetByVID(DWORD vid) const
{
	CSupportActor* supportActor = 0;

	bool bFound = false;

	for (TsupportActorMap::const_iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		supportActor = iter->second;

		if (0 == supportActor)
		{
			sys_err("[CSupportSystem::GetByVID(%d)] Null Pointer (supportActor)", vid);
			continue;
		}

		bFound = supportActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? supportActor : 0;
}

CSupportActor* CSupportSystem::GetByVnum(DWORD vnum) const
{
	CSupportActor* supportActor = 0;

	TsupportActorMap::const_iterator iter = m_supportActorMap.find(vnum);

	if (m_supportActorMap.end() != iter)
		supportActor = iter->second;

	return supportActor;
}

size_t CSupportSystem::CountSummoned() const
{
	size_t count = 0;

	for (TsupportActorMap::const_iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor)
		{
			if (supportActor->IsSummoned())
				++count;
		}
	}

	return count;
}

void CSupportSystem::RefreshBuff()
{
	for (TsupportActorMap::const_iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor)
		{
			if (supportActor->IsSummoned())
			{
				supportActor->GiveBuff();
			}
		}
	}
}
