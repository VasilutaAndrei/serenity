/*********************************************************************
* title_name		: Professional Biolog System
* date_created		: 2016.08.07
* filename			: biologDefines.h
* author			: VegaS
*/
#include "../../common/service.h"
/*********
* From here change the bonuses they offer each mission separately.
* All bonuses and value are as like official.
* If u want to change bonus you can find in enum EPointTypes on char.h other bonus type
* If u want you can put automatically value from bonus like - POINT_MOV_SPEED = 19 and , 10 is value for bonus MOVE_SPEED what be apply.
*/	
int tableBonus[10][10]=
{
	{POINT_ATTBONUS_MONSTER, 8}, // Biologist level 30
	
	{POINT_POISON_REDUCE, 10}, // Biologist level 40
	
	{POINT_DEF_GRADE_BONUS, 80}, // Biologist level 50
	
	{POINT_ATT_GRADE_BONUS, 60}, // Biologist level 60
	
	{POINT_MOV_SPEED, 10, POINT_NORMAL_HIT_DEFEND_BONUS, 15}, // Biologist level 70

	{POINT_ATT_SPEED, 5, POINT_NORMAL_HIT_DAMAGE_BONUS, 10}, // Biologist level 80

	{POINT_RESIST_WARRIOR, 10, POINT_RESIST_ASSASSIN, 10, POINT_RESIST_SURA, 10, POINT_RESIST_SHAMAN, 10, POINT_RESIST_WOLFMAN, 10}, // Biologist level 85

	{POINT_ATTBONUS_WARRIOR, 10, POINT_ATTBONUS_ASSASSIN, 10, POINT_ATTBONUS_SURA, 10, POINT_ATTBONUS_SHAMAN, 10, POINT_ATTBONUS_WOLFMAN, 10}, // Biologist level 90

	{POINT_MAX_HP, 1000, POINT_ATT_GRADE_BONUS, 60, POINT_DEF_GRADE_BONUS, 50}, // Biologist level 92
									
	{POINT_MAX_HP, 1800, POINT_ATT_GRADE_BONUS, 120, POINT_DEF_GRADE_BONUS, 80} // Biologist level 94								
};
							
/*********
* Table translation, here modify messages that are sent in chat client but also in the box when something is wrong or worked.
*/							
const char* pTable_Translate[] =
{
	"[Biolog] Felicitari itemul a fost luat cu succes!",
	"[Biolog] Scuze, acest item a fost putred!", 
	"[Biolog] Felicitari, ai terminat aceasta misiune!",
	"[Biolog] Trebuie sa astepti [%d Zile] [%d Ore] [%d Minute] [%d Secunde] pentru a putea apasa din nou butonul!",
	"[Biolog] Nu detii acest item in inventar pentru a putea face asta!",
	"[Biolog] Trebuie sa ai nivel %u pentru a putea face asta!",
	"[Biolog] Ai terminat deja toate misiunile biologului, nu poti face asta!"
}; 								
									
/*********
* Here are the settings for sending in box when you finish a mission.
* You would need to be change only these texts for the country you belong, values are taken directly bonuses configuration table tableBonus[i][i].
*/		
const char* pPopUp[10][4] =
{  
	{
		"[Felicitari] Recompensa biolog 30",
		"[2. Bonus] Tarie impotriva monstrilor (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 40",
		"[1. Bonus] Rezistenta la otrava (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 50",
		"[1. Bonus] Aparare (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 60",
		"[1. Bonus] Valoare atac (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 70",
		"[1. Bonus] Viteza de miscare +10% (permanent).",
		"[2. Bonus] Reducerea pagubelor +15 (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 80",
		"[1. Bonus] Viteza de atac +5% (permanent).",
		"[2. Bonus] Pagube pe hit +10% (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 85",
		"[1. Bonus] Reducerea pagubelor provocate de jucatori +10% (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 90",
		"[1. Bonus] Cresterea de daune impotriva altor jucatori +10% (permanent)."
	},	
	{
		"[Felicitari] Recompensa biolog 92",
		"[1. Bonus] Max. PV +1.000 (permanent)",						// Bonus is selected from player
		"[2. Bonus] Valoare atac +60 (permanent).",		// Bonus is selected from player
		"[3. Bonus] Aparare +50 (permanent)."			// Bonus is selected from player
	},	
	{
		"[Felicitari] Recompensa biolog 94",
		"[1. Bonus] Max. PV +1.800 (Permanent)",						// Bonus is selected from player
		"[2. Bonus] Valoare atac +120 (permanent).",		// Bonus is selected from player
		"[3. Bonus] Aparare +80 (permanent)."			// Bonus is selected from player
	}											
};

/*********
* You can change the items that you provide when you finish biologist. (default is like official)
*/		
int pTableRewardItem[10] =
{
	70500, // Biologist level 30
	85023, // Biologist level 40
	38051, // Biologist level 50
	72703, // Biologist level 60
	39122, // Biologist level 70
	80005, // Biologist level 80
	71136, // Biologist level 85
	30339, // Biologist level 90
	72703, // Biologist level 92
	70505 // Biologist level 94
};	
							
/*********
* From here you can change the probability chance to succeed when you send an object biologist. 
* If you do not own the object 71035 in inventory then the chance of probability will be random between 60 and 100, 
* and if you held your inventory item 71035 then the chance of probability will be increased to 90, which means that the chance will be between 90 and 100, which what a great success. (default is like official)
*/	
int pTablePercentage[3]	=
{
	71035, // Vnum item 
	60, // Actual chance, if you not have the item in inventory 71035
	95 // The chance probability automatically if you change the item in inventory 71035. (all like official)
};	
	
/*********
* From here you can change how many objects the maximum result for a mission. (default is like official)
*/											
int pTableCountMax[10] =
{
	10, // Biologist level 30
	15, // Biologist level 40
	15, // Biologist level 50
	20, // Biologist level 60
	25, // Biologist level 70
	30, // Biologist level 80
	40, // Biologist level 85
	50, // Biologist level 90
	10, // Biologist level 92
	20 // Biologist level 94
};

/*********
* From here you can change the levels missions at what level you want to start each mission. (default is like official)
*/										
int pTableLevel[10]	=
{
	30, // Biologist level 30
	40, // Biologist level 40
	50, // Biologist level 50
	60, // Biologist level 60
	70, // Biologist level 70
	80, // Biologist level 80
	85, // Biologist level 85
	90, // Biologist level 90
	92, // Biologist level 92
	94 // Biologist level 94
};

/*********
* From here you can change the volume of any items that are needed to achieve mission. (default is like official)
*/	
int pTableVnum[10] =
{
	30006, // Biologist level 30
	30047, // Biologist level 40
	30015, // Biologist level 50
	30050, // Biologist level 60
	30165, // Biologist level 70
	30166, // Biologist level 80
	30167, // Biologist level 85
	30168, // Biologist level 90
	30251, // Biologist level 92
	30252 // Biologist level 94
};

/*********
* From here change the timeout and how long until you can give another object biologist system. (default is like official)
*/

//#define ENABLE_TEST_SERVER_BIOLOG
#define ENABLE_DEFAULT_BIOLOG
#ifdef ENABLE_DEFAULT_BIOLOG
int pTableTime[10] =
{ 
	60*5, // Biologist level 30	//5minute										
	60*10, // Biologist level 40	//10minute										
	60*15, // Biologist level 50	//15minute										
	60*20, // Biologist level 60	//20minute										
	60*25, // Biologist level 70	//25minute										
	60*30, // Biologist level 80	//30minute										
	60*35, // Biologist level 85	//35minute										
	60*40, // Biologist level 90	//40minute										
	60*60, // Biologist level 92	//60minute										
	60*60 // Biologist level 94 	//60minute										
}; 
#endif
#ifdef ENABLE_TEST_SERVER_BIOLOG
int pTableTime[10] =
{ 
	1*1, // Biologist level 30	//10minute										
	1*1, // Biologist level 40	//10minute										
	1*1, // Biologist level 50	//10minute										
	1*1, // Biologist level 60	//10minute										
	1*1, // Biologist level 70	//10minute										
	1*1, // Biologist level 80	//10minute										
	1*1, // Biologist level 85	//30minute										
	1*1, // Biologist level 90	//30minute										
	1*1, // Biologist level 92	//90minute										
	1*1 // Biologist level 94 	//90minute										
}; 											
#endif
/*********
* From here change the timeout for time durating all bonus of biolog
*/
int unlimited = 60*60*60*365;