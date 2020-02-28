#ifndef __INC_METIN_II_MINI_GAME_H__
#define __INC_METIN_II_MINI_GAME_H__

#include <boost/unordered_map.hpp>

#include "../../common/stl.h"
#include "../../common/length.h"
#include "../../common/tables.h"
#include "packet.h"


class CMiniGame : public singleton<CMiniGame>
{
	public:
		CMiniGame ();
		virtual ~CMiniGame ();

		bool Initialize();
		void Destroy();
		
		void InitializeAttendance(int iEnable);
		void InitializeEasterEvent(int iEnable);
		
		void SpawnEventNPC(DWORD dwVnum);
		void StartAttendanceRegen();
		void StartEasterStoneRegen();
		
		void AttendanceMonsterAttack(LPCHARACTER pkAttacker, LPCHARACTER pkVictim);
		void AttendanceEventInfo(LPCHARACTER pChar);
		void AttendanceEventRequestReward(LPCHARACTER pChar);
		bool ReadRewardItemFile(const char * c_pszFileName);
	private:
		std::vector<TRewardItem>	attendanceRewardVec;
};

#endif

