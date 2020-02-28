#ifndef __INC_METIN_II_GAME_SHOP_H__
#define __INC_METIN_II_GAME_SHOP_H__

enum
{
	SHOP_MAX_DISTANCE = 1000
};

class CGrid;

/* ---------------------------------------------------------------------------------- */
class CShop
{
	public:
		typedef struct shop_item
		{
			DWORD	vnum;		// 아이템 번호
			#ifdef FULL_YANG
			long	long	price;		// °ˇ°Y
			#else
			DWORD	price;		// °ˇ°Y
			#endif
			BYTE	count;		// 아이템 개수

			LPITEM	pkItem;
			int		itemid;		// 아이템 고유아이디
			shop_item()
			{
				vnum = 0;
				price = 0;
				count = 0;
				itemid = 0;
				pkItem = NULL;
			}
		} SHOP_ITEM;

		CShop();
		virtual ~CShop(); // @fixme139 (+virtual)

		bool	Create(DWORD dwVnum, DWORD dwNPCVnum, TShopItemTable * pItemTable);
		void	SetShopItems(TShopItemTable * pItemTable, BYTE bItemCount);

		virtual void	SetPCShop(LPCHARACTER ch);
		virtual bool	IsPCShop()	{ return m_pkPC ? true : false; }

		// 게스트 추가/삭제
		virtual bool	AddGuest(LPCHARACTER ch,DWORD owner_vid, bool bOtherEmpire);
		void	RemoveGuest(LPCHARACTER ch);

		// 물건 구입
		virtual long long	Buy(LPCHARACTER ch, BYTE pos);

		// 게스트에게 패킷을 보냄
		void	BroadcastUpdateItem(BYTE pos);

		// 판매중인 아이템의 갯수를 알려준다.
		int		GetNumberByVnum(DWORD dwVnum);

		// 아이템이 상점에 등록되어 있는지 알려준다.
		virtual bool	IsSellingItem(DWORD itemID);
#if defined(SHOP_SEARCH)
 		bool SearchItem(DWORD vnum,long long price,DWORD book);
		
#endif
		DWORD	GetVnum() { return m_dwVnum; }
		DWORD	GetNPCVnum() { return m_dwNPCVnum; }
		
#ifdef OFFLINE_SHOP
	public:
		int		GetItemCount();
		bool	GetItems();
		void	SetLocked(bool val) { m_bLocked = val; }
		bool	IsLocked() { return m_bLocked; }
		void	RemoveGuests(LPCHARACTER except=NULL);
		void	ClearItems();
		void	SetPrivShopItems(std::vector<TShopItemTable *> map_shop);
		LPCHARACTER GetPC() { return m_pkPC; }
	protected:
		bool					m_bLocked;
#endif

	protected:
		void	Broadcast(const void * data, int bytes);

	protected:
		DWORD				m_dwVnum;
		DWORD				m_dwNPCVnum;

		CGrid *				m_pGrid;

		typedef TR1_NS::unordered_map<LPCHARACTER, bool> GuestMapType;
		GuestMapType m_map_guest;
		std::vector<SHOP_ITEM>		m_itemVector;	// 이 상점에서 취급하는 물건들

		LPCHARACTER			m_pkPC;
};

#endif
