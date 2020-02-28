#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "MountSystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"

EVENTINFO(mountsystem_event_info)
{
	CMountSystem* pMountSystem;
};

EVENTFUNC(mountsystem_update_event)
{
	mountsystem_event_info* info = dynamic_cast<mountsystem_event_info*>( event->info );
	if ( info == NULL )
	{
		sys_err( "<mountsystem_update_event> <Factor> Null pointer" );
		return 0;
	}

	CMountSystem*	pMountSystem = info->pMountSystem;

	if (NULL == pMountSystem)
		return 0;


	pMountSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CMountActor
///////////////////////////////////////////////////////////////////////////////////////

CMountActor::CMountActor(LPCHARACTER owner, DWORD vnum)
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

CMountActor::~CMountActor()
{
	this->Unsummon();
	m_pkOwner = 0;
}

void CMountActor::SetName()
{
	std::string petName = m_pkOwner->GetName();

	if (true == IsSummoned() && m_dwVnum == 20110)
	{
		petName += " - Mistret";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20111)
	{
		petName += " - Lup";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20112)
	{
		petName += " - Tigru";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20113)
	{
		petName += " - Leu";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20114)
	{
		petName += " - Leu Alb";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20115)
	{
		petName += " - Mistret de Lupta";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20116)
	{
		petName += " - Lup de Lupta";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20117)
	{
		petName += " - Tigru al Furtunii";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20118)
	{
		petName += " - Leu Luptator";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20119)
	{
		petName += " - Cal Negru";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20120)
	{
		petName += " - Tigru Regal Albastru";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20121)
	{
		petName += " - Tigru Regal Rosu";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20122)
	{
		petName += " - Tigru Regal Galben";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20123)
	{
		petName += " - Tigru Regal Verde";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20124)
	{
		petName += " - Tigru Regal Gri";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20125)
	{
		petName += " - Tigru Regal Alb";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20201)
	{
		petName += " - Mistret Tanar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20202)
	{
		petName += " - Lup Tanar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20203)
	{
		petName += " - Tigru Tanar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20204)
	{
		petName += " - Leu Tanar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20205)
	{
		petName += " - Mistret Salbatic";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20206)
	{
		petName += " - Lup Salbatic";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20207)
	{
		petName += " - Tigru Salbatic";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20208)
	{
		petName += " - Leu Salbatic";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20209)
	{
		petName += " - Mistret Curajos";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20210)
	{
		petName += " - Lup Curajos";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20211)
	{
		petName += " - Tigru Curajos";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20212)
	{
		petName += " - Leu Curajos";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20213)
	{
		petName += " - Ren Tanar(M)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20214)
	{
		petName += " - Ren Salbatic(M)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20215)
	{
		petName += " - Ren Curajos(M)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20216)
	{
		petName += " - Ren Tanar(F)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20217)
	{
		petName += " - Ren Salbatic(F)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20218)
	{
		petName += " - Ren Curajos(F)";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20219)
	{
		petName += " - Polyphemos";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20220)
	{
		petName += " - Rudolph";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20221)
	{
		petName += " - Predator Polar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20222)
	{
		petName += " - Panda Blindat";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20223)
	{
		petName += " - Dragor Tanar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20224)
	{
		petName += " - Dragor Salbatic";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20225)
	{
		petName += " - Dragor Curajos";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20226)
	{
		petName += " - Vis Urat";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20227)
	{
		petName += " - Unicorn";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20228)
	{
		petName += " - Moa Tanar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20229)
	{
		petName += " - Moa Salbatic";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20230)
	{
		petName += " - Moa Curajos";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20231)
	{
		petName += " - Pantera Neagra";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20232)
	{
		petName += " - Leopard";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20233)
	{
		petName += " - Nugget";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20234)
	{
		petName += " - Nugget";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20235)
	{
		petName += " - Nugget";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20236)
	{
		petName += " - Muffin";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20237)
	{
		petName += " - Muffin";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20238)
	{
		petName += " - Muffin";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20239)
	{
		petName += " - Manny";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20240)
	{
		petName += " - Ellie";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20241)
	{
		petName += " - Tulu de Lupta";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20242)
	{
		petName += " - Tulu de Lupta";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20243)
	{
		petName += " - Dragon Nirvana";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20244)
	{
		petName += " - Dragon Samsara";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20245)
	{
		petName += " - Soparla de Lava";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20246)
	{
		petName += " - Soparla de Gheata";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20247)
	{
		petName += " - Appaloosa";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20248)
	{
		petName += " - Urs Grizli";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20249)
	{
		petName += " - Urs Polar";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20250)
	{
		petName += " - Dragon Imperial";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20251)
	{
		petName += " - Skeleton";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20252)
	{
		petName += " - Ren Imperial";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20253)
	{
		petName += " - Eagle";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20254)
	{
		petName += " - Lup Imperial";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20255)
	{
		petName += " - Soparla de Aur";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20256)
	{
		petName += " - Pegasus";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20257)
	{
		petName += " - Phoenix";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20258)
	{
		petName += " - Unicorn de Smarald";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20259)
	{
		petName += " - Unicorn de Aur";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20260)
	{
		petName += " - Unicorn de Ametist";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20261)
	{
		petName += " - Unicorn de Rubin";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20262)
	{
		petName += " - Unicorn de Safir";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20263)
	{
		petName += " - Zeus";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20264)
	{
		petName += " - Thor";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20265)
	{
		petName += " - Puicuta Turcoaz";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20266)
	{
		petName += " - Iepuras";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned() && m_dwVnum == 20267)
	{
		petName += " - Cocosel Rosu";
		
		m_pkChar->SetName(petName);
	}
	else if (true == IsSummoned())
	{
		petName += " Mount";
		
		m_pkChar->SetName(petName);
	}

	m_name = petName;
}

bool CMountActor::Mount(LPITEM mountItem)
{
	if (0 == m_pkOwner)
		return false;
	
	if(!mountItem)
		return false;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
	
	if (m_pkOwner->GetHorse())
		m_pkOwner->HorseSummon(false);
	
	Unmount();

	m_pkOwner->AddAffect(AFFECT_MOUNT, POINT_MOUNT, m_dwVnum, AFF_NONE, (DWORD)mountItem->GetSocket(0) - time(0), 0, true);
	
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (mountItem->GetProto()->aApplies[i].bType == APPLY_NONE)
			continue;

		m_pkOwner->AddAffect(AFFECT_MOUNT_BONUS, aApplyInfo[mountItem->GetProto()->aApplies[i].bType].bPointType, mountItem->GetProto()->aApplies[i].lValue, AFF_NONE, (DWORD)mountItem->GetSocket(0) - time(0), 0, false);
	}
	
	return m_pkOwner->GetMountVnum() == m_dwVnum;
}

void CMountActor::Unmount()
{
	if (0 == m_pkOwner)
		return;
	
	if (!m_pkOwner->GetMountVnum())
		return;
	
	m_pkOwner->RemoveAffect(AFFECT_MOUNT);
	m_pkOwner->RemoveAffect(AFFECT_MOUNT_BONUS);
	m_pkOwner->MountVnum(0);
	
	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
	
	if (m_pkOwner->GetHorse())
		m_pkOwner->HorseSummon(false);
	
	m_pkOwner->MountVnum(0);
}

void CMountActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
		this->SetSummonItem(NULL);
		
		if (NULL != m_pkChar)
			M2_DESTROY_CHARACTER(m_pkChar);

		m_pkChar = 0;
		m_dwVID = 0;
	}
}

DWORD CMountActor::Summon(LPITEM pSummonItem, bool bSpawnFar)
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
		m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkChar->GetVID();

		return m_dwVID;
	}

	m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(m_dwVnum, m_pkOwner->GetMapIndex(), x, y, z, false, (int)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CMountActor::Summon] Failed to summon the mount. (vnum: %d)", m_dwVnum);
		return 0;
	}

	m_pkChar->SetMount();

	m_pkChar->SetEmpire(m_pkOwner->GetEmpire());

	m_dwVID = m_pkChar->GetVID();

	this->SetName();

	this->SetSummonItem(pSummonItem);
	
	//m_pkOwner->ComputePoints();
	
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);

	return m_dwVID;
}

bool CMountActor::_UpdateFollowAI()
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
	int		APPROACH = 600;

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
		m_pkChar->SetNowWalking(false);

		Follow(APPROACH);

		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	return true;
}

bool CMountActor::Update(DWORD deltaTime)
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

bool CMountActor::Follow(float fMinDistance)
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

void CMountActor::SetSummonItem(LPITEM pItem)
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

///////////////////////////////////////////////////////////////////////////////////////
//  CMountSystem
///////////////////////////////////////////////////////////////////////////////////////

CMountSystem::CMountSystem(LPCHARACTER owner)
{
	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CMountSystem::~CMountSystem()
{
	Destroy();
}

void CMountSystem::Destroy()
{
	for (TMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			delete mountActor;
		}
	}
	event_cancel(&m_pkMountSystemUpdateEvent);
	m_mountActorMap.clear();
}

bool CMountSystem::Update(DWORD deltaTime)
{
	bool bResult = true;

	DWORD currentTime = get_dword_time();

	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;

	std::vector <CMountActor*> v_garbageActor;

	for (TMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CMountActor* mountActor = iter->second;

		if (0 != mountActor && mountActor->IsSummoned())
		{
			LPCHARACTER pMount = mountActor->GetCharacter();

			if (NULL == CHARACTER_MANAGER::instance().Find(pMount->GetVID()))
			{
				v_garbageActor.push_back(mountActor);
			}
			else
			{
				bResult = bResult && mountActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CMountActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeleteMount(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CMountSystem::DeleteMount(DWORD mobVnum)
{
	TMountActorMap::iterator iter = m_mountActorMap.find(mobVnum);

	if (m_mountActorMap.end() == iter)
	{
		sys_err("[CMountSystem::DeleteMount] Can't find mount on my list (VNUM: %d)", mobVnum);
		return;
	}

	CMountActor* mountActor = iter->second;

	if (0 == mountActor)
		sys_err("[CMountSystem::DeleteMount] Null Pointer (mountActor)");
	else
		delete mountActor;

	m_mountActorMap.erase(iter);
}

void CMountSystem::DeleteMount(CMountActor* mountActor)
{
	for (TMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		if (iter->second == mountActor)
		{
			delete mountActor;
			m_mountActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CMountSystem::DeleteMount] Can't find mountActor(0x%x) on my list(size: %d) ", mountActor, m_mountActorMap.size());
}

void CMountSystem::Unsummon(DWORD vnum, bool bDeleteFromList)
{
	//if (m_pkOwner->IncreaseMountCounter() >= 5)
	//{
	//	m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("_TRANSLATE_CHAT_TYPE_PACKETS_FROM_SOURCE_TO_GLOBAL__TRANSLATE_LIST_110"));
	//	return;
	//}
	
	CMountActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CMountSystem::Unsummon(%d)] Null Pointer (actor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeleteMount(actor);

	bool bActive = false;
	for (TMountActorMap::iterator it = m_mountActorMap.begin(); it != m_mountActorMap.end(); it++)
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pkMountSystemUpdateEvent);
		m_pkMountSystemUpdateEvent = NULL;
	}
}


void CMountSystem::Summon(DWORD mobVnum, LPITEM pSummonItem, bool bSpawnFar)
{	
	CMountActor* mountActor = this->GetByVnum(mobVnum);

	if (0 == mountActor)
	{
		mountActor = M2_NEW CMountActor(m_pkOwner, mobVnum);
		m_mountActorMap.insert(std::make_pair(mobVnum, mountActor));
	}

	DWORD mountVID = mountActor->Summon(pSummonItem, bSpawnFar);

	if (!mountVID)
		sys_err("[CMountSystem::Summon(%d)] Null Pointer (mountVID)", pSummonItem->GetID());

	if (NULL == m_pkMountSystemUpdateEvent)
	{
		mountsystem_event_info* info = AllocEventInfo<mountsystem_event_info>();

		info->pMountSystem = this;

		m_pkMountSystemUpdateEvent = event_create(mountsystem_update_event, info, PASSES_PER_SEC(1) / 4);
	}

	//return mountActor;
}

void CMountSystem::Mount(DWORD mobVnum, LPITEM mountItem)
{
	CMountActor* mountActor = this->GetByVnum(mobVnum);

	if (!mountActor)
	{
		sys_err("[CMountSystem::Mount] Null Pointer (mountActor)");
		return;
	}
	
	if(!mountItem)
		return;

	this->Unsummon(mobVnum, false);
	mountActor->Mount(mountItem);
}

void CMountSystem::Unmount(DWORD mobVnum)
{
	CMountActor* mountActor = this->GetByVnum(mobVnum);

	if (!mountActor)
	{
		sys_err("[CMountSystem::Mount] Null Pointer (mountActor)");
		return;
	}
	
	if(LPITEM pSummonItem = m_pkOwner->GetWear(WEAR_COSTUME_MOUNT))
	{
		this->Summon(mobVnum, pSummonItem, false);
	}
	
	mountActor->Unmount();
}

CMountActor* CMountSystem::GetByVID(DWORD vid) const
{
	CMountActor* mountActor = 0;

	bool bFound = false;

	for (TMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		mountActor = iter->second;

		if (0 == mountActor)
		{
			sys_err("[CMountSystem::GetByVID(%d)] Null Pointer (mountActor)", vid);
			continue;
		}

		bFound = mountActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? mountActor : 0;
}

CMountActor* CMountSystem::GetByVnum(DWORD vnum) const
{
	CMountActor* mountActor = 0;

	TMountActorMap::const_iterator iter = m_mountActorMap.find(vnum);

	if (m_mountActorMap.end() != iter)
		mountActor = iter->second;

	return mountActor;
}

size_t CMountSystem::CountSummoned() const
{
	size_t count = 0;

	for (TMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			if (mountActor->IsSummoned())
				++count;
		}
	}

	return count;
}