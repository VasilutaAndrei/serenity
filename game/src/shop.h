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
			DWORD	vnum;		// ������ ��ȣ
			#ifdef FULL_YANG
			long	long	price;		// �Ƣ���Y
			#else
			DWORD	price;		// �Ƣ���Y
			#endif
			BYTE	count;		// ������ ����

			LPITEM	pkItem;
			int		itemid;		// ������ �������̵�
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

		// �Խ�Ʈ �߰�/����
		virtual bool	AddGuest(LPCHARACTER ch,DWORD owner_vid, bool bOtherEmpire);
		void	RemoveGuest(LPCHARACTER ch);

		// ���� ����
		virtual long long	Buy(LPCHARACTER ch, BYTE pos);

		// �Խ�Ʈ���� ��Ŷ�� ����
		void	BroadcastUpdateItem(BYTE pos);

		// �Ǹ����� �������� ������ �˷��ش�.
		int		GetNumberByVnum(DWORD dwVnum);

		// �������� ������ ��ϵǾ� �ִ��� �˷��ش�.
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
		std::vector<SHOP_ITEM>		m_itemVector;	// �� �������� ����ϴ� ���ǵ�

		LPCHARACTER			m_pkPC;
};

#endif
