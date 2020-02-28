#define OXEVENT_MAP_INDEX 113

#ifdef ENABLE_FEATURES_OXEVENT

	#define PASSWORD_MANAGER "metin2team" // Change here password for login in event manager.

	enum ETypeInfoOxEvent
	{
		OXEVENT_TOP_LIMIT = 5,
		OXEVENT_FIRST_ANSWER = 1,
		OXEVENT_NO_DATA = 999,
		OXEVENT_OPEN_LOGIN = 0,
		OXEVENT_OPEN_EVENT = 1,
		OXEVENT_CLOSE_GATES = 2,
		OXEVENT_CLOSE_EVENT = 3,
		OXEVENT_REWARD_PLAYERS = 4,
		OXEVENT_ASK_QUESTION = 5,
		OXEVENT_FORCE_CLOSE_EVENT = 6,
	};
#endif

struct tag_Quiz
{
	char level;
	char Quiz[256];
	bool answer;
};

enum OXEventStatus
{
	OXEVENT_FINISH = 0, // OX�̺�Ʈ�� ������ ���� ����
	OXEVENT_OPEN = 1,	// OX�̺�Ʈ�� ���۵�. ������(20012)�� ���ؼ� ���尡��
	OXEVENT_CLOSE = 2,	// OX�̺�Ʈ�� ������ ����. ������(20012)�� ���� ������ ���ܵ�
	OXEVENT_QUIZ = 3,	// ��� ������.

	OXEVENT_ERR = 0xff
};

class COXEventManager : public singleton<COXEventManager>
{
	private :
		std::map<DWORD, DWORD> m_map_char;
		std::map<DWORD, DWORD> m_map_attender;
		std::map<DWORD, DWORD> m_map_miss;
#ifdef ENABLE_FEATURES_OXEVENT
		std::map<DWORD, DWORD> m_map_answers;
#endif

		std::vector<std::vector<tag_Quiz> > m_vec_quiz;
#ifdef ENABLE_SHOW_QUIZ_NUMBER_OXEVENT
		DWORD iCounterQuiz;
#endif
		LPEVENT m_timedEvent;
#ifdef ENABLE_FEATURES_OXEVENT
		LPEVENT m_counterEvent;
#endif

	protected :
		bool CheckAnswer();

		bool EnterAudience(LPCHARACTER pChar);
		bool EnterAttender(LPCHARACTER pChar);

	public :
		bool Initialize();
		void Destroy();

		OXEventStatus GetStatus();
		void SetStatus(OXEventStatus status);

		bool LoadQuizScript(const char* szFileName);

		bool Enter(LPCHARACTER pChar);

		bool CloseEvent();

		void ClearQuiz();
		bool AddQuiz(unsigned char level, const char* pszQuestion, bool answer);
		bool ShowQuizList(LPCHARACTER pChar);
#ifdef ENABLE_FEATURES_OXEVENT
		DWORD GetObserverCount() { return (m_map_char.size() - m_map_attender.size() > 0) ? m_map_char.size() - m_map_attender.size() : 0; }
		std::string GetGuildWinner(LPCHARACTER pkChar);

		bool IsExistVnum(DWORD dwItemVnum);
		bool IsExistCount(BYTE count);
		bool IsExistKeyAnswers(LPCHARACTER pChar, int * keyAnswers);
		//bool CheckPassword(LPCHARACTER pChar, const char* c_szPassword);
		
		bool IsLastManStanding();
		void InsertAnswers(LPCHARACTER pChar);
		void TruncateAnswers(LPCHARACTER pChar);
		void RegisterWinner();
		void InitializePacket(LPCHARACTER pChar);
		
		void RefreshWinners();
		void OpenGui(LPCHARACTER pChar);
		void Disconnect(LPCHARACTER pChar);
		
		void GetErrByStatus(LPCHARACTER pChar, int key);

		void NoticeAll(std::string msg);
		void Manager(LPCHARACTER ch, const char* c_pData);
#endif

		bool Quiz(unsigned char level, int timelimit);
		bool GiveItemToAttender(DWORD dwItemVnum, BYTE count);

		bool CheckAnswer(bool answer);
		void WarpToAudience();

		bool LogWinner();

		DWORD GetAttenderCount() { return m_map_attender.size(); }
};

