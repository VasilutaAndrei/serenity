#ifndef __INC__METIN__WHISPER__ADMIN_SYSTEM__
#define __INC__METIN__WHISPER__ADMIN_SYSTEM__
/*********************************************************************
* title_name		: Whisper Admin
* date_created		: 2017.01.25
* filename			: whisper_admin.h
* author			: VegaS
* version_actual	: Version 0.0.1
*/
extern char M2_WHISPER_LANG[512 + 1];
extern int M2_WHISPER_COLOR;
#pragma once
class CWhisperAdmin : public singleton<CWhisperAdmin>
{
	public:
		CWhisperAdmin();
~CWhisperAdmin();

		bool IsEuropa(std::string lang);
		void SaveConfig(const char* c_pszLang, int color);
		bool file_is_empty(std::ifstream& file);
		
		std::string GetLang();
		int GetColor();

		void SaveLog(LPCHARACTER ch, const char* c_pszText, const char* c_pszLang, int color);
		
		void SendWhisper(const char* c_pszText);		
		int Whisper(LPDESC d, const char * c_pData, size_t uiBytes);
		void Manager(LPCHARACTER ch, const char* c_pData);	
};
#endif