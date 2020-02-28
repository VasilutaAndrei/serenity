#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "PetSystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"

int	DuratingBonus[] = {60*60*24*365};

EVENTINFO(petsystem_event_info)
{
	CPetSystem* pPetSystem;
};

EVENTFUNC(petsystem_update_event)
{
	petsystem_event_info* info = dynamic_cast<petsystem_event_info*>( event->info );
	if ( info == NULL )
	{
		sys_err( "check_speedhack_event> <Factor> Null pointer" );
		return 0;
	}

	CPetSystem*	pPetSystem = info->pPetSystem;

	if (NULL == pPetSystem)
		return 0;


	pPetSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}

const float PET_COUNT_LIMIT = 3;

CPetActor::CPetActor(LPCHARACTER owner, DWORD vnum)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_dwLastActionTime = 0;

	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;

	m_dwSummonItemVID = 0;
	m_dwSummonItemVnum = 0;
}

CPetActor::~CPetActor()
{
	this->Unsummon();

	m_pkOwner = 0;
}

void CPetActor::SetName()
{
	std::string petName = m_pkOwner->GetName();

	if (true == IsSummoned() && m_dwVnum == 34025)
	{
		petName += " - |cFFFF0049|H|hPhoenix de Foc";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34010)
	{
		petName += " - |cFFc9ff00|H|hBambi";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34003)
	{
		petName += " - |cFF009DFF|H|hPhoenix de Gheata";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34004)
	{
		petName += " - |cFF732222|H|hMicul Azrael";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34005)
	{
		petName += " - |cFF528508|H|hPorcusor";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34006)
	{
		petName += " - |cFF528508|H|hRufus";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34007)
	{
		petName += " - |cFF528508|H|hKhan";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34008)
	{
		petName += " - |cFF528508|H|hLeonidas";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34009)
	{
		petName += " - |cFFDCE627|H|hMicul Azrael (Aur)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34011)
	{
		petName += " - |cFF009DFF|H|hKnuud";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34012)
	{
		petName += " - |cFF009DFF|H|hBao Bao";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34013)
	{
		petName += " - |cFF732222|H|hMini Executor";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34014)
	{
		petName += " - |cFFDCE627|H|hMini Executor (Aur)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34015)
	{
		petName += " - |cFFc9ff00|H|hBoney";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34016)
	{
		petName += " - |cFFA1C718|H|hPhoenix de Jad";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34017)
	{
		petName += " - |cFF109EC7|H|hPorkie Polar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34019)
	{
		petName += " - |cFF109EC7|H|hArctis Khan";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34020)
	{
		petName += " - |cFF109EC7|H|hLeonidas Alb";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34021)
	{
		petName += " - |cFFc9ff00|H|hSheldon";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34022)
	{
		petName += " - |cFFc9ff00|H|hChooper";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34023)
	{
		petName += " - |cFFC97D87|H|hPui de Pantera";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34028)
	{
		petName += " - |cFF109EC7|H|hFulg de Nea";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34029)
	{
		petName += " - |cFF109EC7|H|hClopotel de Nea";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34030)
	{
		petName += " - |cFFF42B07|H|hElf Rosu";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34031)
	{
		petName += " - |cFF34962B|H|hElf Verde";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34032)
	{
		petName += " - |cFF6B6B6B|H|hJiangshi Junior";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34033)
	{
		petName += " - |cFF237A00|H|hMr Jiangshi";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34034)
	{
		petName += " - |cFF16ACB8|H|hShiver";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34035)
	{
		petName += " - |cFF16ACB8|H|hSnivel";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34036)
	{
		petName += " - |cFFA210AB|H|hLiliacul Bruce";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34037)
	{
		petName += " - |cFFA210AB|H|hLiliacul Wayne";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34038)
	{
		petName += " - |cFF77B8EA|H|hRamblue";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34039)
	{
		petName += " - |cFFD58DEE|H|hEweriel";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34040)
	{
		petName += " - |cFF732222|H|hMicul Huashin";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34100)
	{
		petName += " - |cFFE13066|H|hSemiluna Rosie";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34101)
	{
		petName += " - |cFF6271E3|H|hSemiluna Albastra";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34102)
	{
		petName += " - |cFFE13066|H|hLanterna Rosie";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34103)
	{
		petName += " - |cFF6271E3|H|hLanterna Albastra";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34104)
	{
		petName += " - |cFFF91ECE|H|hValentinia";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34105)
	{
		petName += " - |cFF6271E3|H|hValentino";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34106)
	{
		petName += " - |cFFE13066|H|hAmoras";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34107)
	{
		petName += " - |cFFE13066|H|hInima Iute";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34108)
	{
		petName += " - |cFF13A575|H|hBonsai";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34109)
	{
		petName += " - |cFF13A575|H|hDonnie";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 34110)
	{
		petName += " - |cFF13A575|H|hBonnie";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned())
	{
		petName += " Companiot";
		
		m_pkChar->SetName(petName);
	}

	m_name = petName;
}

bool CPetActor::Mount()
{
	if (0 == m_pkOwner)
		return false;

	if (true == HasOption(EPetOption_Mountable))
		m_pkOwner->MountVnum(m_dwVnum);

	return m_pkOwner->GetMountVnum() == m_dwVnum;;
}

void CPetActor::Unmount()
{
	if (0 == m_pkOwner)
		return;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
}

void CPetActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
		m_pkOwner->RemoveAffect(AFFECT_PET_SUMMON);
		this->SetSummonItem(NULL);

		if (NULL != m_pkChar)
			M2_DESTROY_CHARACTER(m_pkChar);
		
		m_pkChar = 0;
		m_dwVID = 0;
	}
}

DWORD CPetActor::Summon(LPITEM pSummonItem, bool bSpawnFar)
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

	m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(m_dwVnum, m_pkOwner->GetMapIndex(), x, y, z, false, (int)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CPetSystem::Summon] Failed to summon the pet. (vnum: %d)", m_dwVnum);
		return 0;
	}

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (pSummonItem->GetProto()->aApplies[i].bType == APPLY_NONE)
			continue;

		m_pkOwner->AddAffect(AFFECT_PET_SUMMON, aApplyInfo[pSummonItem->GetProto()->aApplies[i].bType].bPointType, pSummonItem->GetProto()->aApplies[i].lValue, AFF_NONE, DuratingBonus[0], 0, false);
	}
	
	
	m_pkChar->SetPet();
	m_dwVID = m_pkChar->GetVID();
	this->SetName();
	this->SetSummonItem(pSummonItem);
	m_pkOwner->ComputePoints();
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);

	return m_dwVID;
}

bool CPetActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	float fDist = number(fMinDist, fMaxDist);
	float r = (float)number (0, 359);
	float dest_x = GetOwner()->GetX() + fDist * cos(r);
	float dest_y = GetOwner()->GetY() + fDist * sin(r);

	m_pkChar->SetNowWalking(true);

	if (!m_pkChar->IsStateMove() && m_pkChar->Goto(dest_x, dest_y))
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();

	return true;
}

bool CPetActor::_UpdateFollowAI()
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
	float	START_RUN_DISTANCE = 900.0f;

	float	RESPAWN_DISTANCE = 4500.f;
	int		APPROACH = 623;

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

		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	return true;
}

bool CPetActor::Update(DWORD deltaTime)
{
	bool bResult = true;
	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead())
		|| NULL == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		this->Unsummon();
		return true;
	}

	if (this->IsSummoned())
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}

bool CPetActor::Follow(float fMinDistance)
{
	if( !m_pkOwner || !m_pkChar)
		return false;

	float fOwnerX = m_pkOwner->GetX();
	float fOwnerY = m_pkOwner->GetY();

	float fPetX = m_pkChar->GetX();
	float fPetY = m_pkChar->GetY();

	float fDist = DISTANCE_SQRT(fOwnerX - fPetX, fOwnerY - fPetY);
	if (fDist <= fMinDistance)
		return false;

	m_pkChar->SetRotationToXY(fOwnerX, fOwnerY);

	float fx, fy;

	float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);

	if (!m_pkChar->Goto((int)(fPetX+fx+0.5f), (int)(fPetY+fy+0.5f)) )
		return false;

	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

void CPetActor::SetSummonItem (LPITEM pItem)
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

CPetSystem::CPetSystem(LPCHARACTER owner)
{
	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CPetSystem::~CPetSystem()
{
	Destroy();
}

void CPetSystem::Destroy()
{
	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor)
		{
			delete petActor;
		}
	}
	event_cancel(&m_pkPetSystemUpdateEvent);
	m_petActorMap.clear();
}

bool CPetSystem::Update(DWORD deltaTime)
{
	bool bResult = true;

	DWORD currentTime = get_dword_time();

	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;

	std::vector <CPetActor*> v_garbageActor;

	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor && petActor->IsSummoned())
		{
			LPCHARACTER pPet = petActor->GetCharacter();

			if (NULL == CHARACTER_MANAGER::instance().Find(pPet->GetVID()))
			{
				v_garbageActor.push_back(petActor);
			}
			else
			{
				bResult = bResult && petActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CPetActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeletePet(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CPetSystem::DeletePet(DWORD mobVnum)
{
	TPetActorMap::iterator iter = m_petActorMap.find(mobVnum);

	if (m_petActorMap.end() == iter)
	{
		sys_err("[CPetSystem::DeletePet] Can't find pet on my list (VNUM: %d)", mobVnum);
		return;
	}

	CPetActor* petActor = iter->second;

	if (0 == petActor)
		sys_err("[CPetSystem::DeletePet] Null Pointer (petActor)");
	else
		delete petActor;

	m_petActorMap.erase(iter);
}

void CPetSystem::DeletePet(CPetActor* petActor)
{
	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		if (iter->second == petActor)
		{
			delete petActor;
			m_petActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CPetSystem::DeletePet] Can't find petActor(0x%x) on my list(size: %d) ", petActor, m_petActorMap.size());
}

void CPetSystem::Unsummon(DWORD vnum, bool bDeleteFromList)
{
	CPetActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CPetSystem::GetByVnum(%d)] Null Pointer (petActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeletePet(actor);

	bool bActive = false;
	for (TPetActorMap::iterator it = m_petActorMap.begin(); it != m_petActorMap.end(); it++)
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pkPetSystemUpdateEvent);
		m_pkPetSystemUpdateEvent = NULL;
	}
}


CPetActor* CPetSystem::Summon(DWORD mobVnum, LPITEM pSummonItem, bool bSpawnFar)
{
	CPetActor* petActor = this->GetByVnum(mobVnum);

	if (0 == petActor)
	{
		petActor = M2_NEW CPetActor(m_pkOwner, mobVnum);
		m_petActorMap.insert(std::make_pair(mobVnum, petActor));
	}

	DWORD petVID = petActor->Summon(pSummonItem, bSpawnFar);
#ifdef ENABLE_NEWSTUFF
	if (!petVID)
		sys_err("[CPetSystem::Summon(%d)] Null Pointer (petVID)", pSummonItem);
#endif
	if (NULL == m_pkPetSystemUpdateEvent)
	{
		petsystem_event_info* info = AllocEventInfo<petsystem_event_info>();

		info->pPetSystem = this;

		m_pkPetSystemUpdateEvent = event_create(petsystem_update_event, info, PASSES_PER_SEC(1) / 4);
	}

	return petActor;
}


CPetActor* CPetSystem::GetByVID(DWORD vid) const
{
	CPetActor* petActor = 0;

	bool bFound = false;

	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		petActor = iter->second;

		if (0 == petActor)
		{
			sys_err("[CPetSystem::GetByVID(%d)] Null Pointer (petActor)", vid);
			continue;
		}

		bFound = petActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? petActor : 0;
}

CPetActor* CPetSystem::GetByVnum(DWORD vnum) const
{
	CPetActor* petActor = 0;

	TPetActorMap::const_iterator iter = m_petActorMap.find(vnum);

	if (m_petActorMap.end() != iter)
		petActor = iter->second;

	return petActor;
}

size_t CPetSystem::CountSummoned() const
{
	size_t count = 0;

	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor)
		{
			if (petActor->IsSummoned())
				++count;
		}
	}

	return count;
}
