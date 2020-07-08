#ifndef __HEADER_BELT_INVENTORY_HELPER__
#define __HEADER_BELT_INVENTORY_HELPER__

#include "char.h"
#include "item.h"

class CBeltInventoryHelper
{
public:
	typedef BYTE	TGradeUnit;

	static TGradeUnit GetBeltGradeByRefineLevel(int level)
	{
		static TGradeUnit beltGradeByLevelTable[] = 
		{
			0,			// ��Ʈ+0
			1,			// +1
			1,			// +2
			2,			// +3
			2,			// +4,
			3,			// +5
			4,			// +6,
			5,			// +7,
			6,			// +8,
			7,			// +9
		};

		if (level >= _countof(beltGradeByLevelTable))
		{
			sys_err("CBeltInventoryHelper::GetBeltGradeByRefineLevel - Overflow level (%d", level);
			return 0;
		}

		return beltGradeByLevelTable[level];
	}

	// ���� ��Ʈ ������ ��������, � ������ �̿��� �� �ִ��� ����
	static const TGradeUnit* GetAvailableRuleTableByGrade()
	{
		/**
			��Ʈ�� �� +0 ~ +9 ������ ���� �� ������, ������ ���� 7�ܰ� ������� ���еǾ� �κ��丮�� Ȱ�� ȭ �ȴ�.
			��Ʈ ������ ���� ��� ������ ���� �Ʒ� �׸��� ����. ���� ��� >= Ȱ������ ����̸� ��� ����.
			(��, ���� ������ 0�̸� ������ ��� �Ұ�, ��ȣ ���� ���ڴ� ���)
			
				2(1)  4(2)  6(4)  8(6)
				5(3)  5(3)  6(4)  8(6)
				7(5)  7(5)  7(5)  8(6)
				9(7)  9(7)  9(7)  9(7)

			��Ʈ �κ��丮�� ũ��� 4x4 (16ĭ)
		*/

		static TGradeUnit availableRuleByGrade[BELT_INVENTORY_SLOT_COUNT] = {
			1, 2, 4, 6,
			3, 3, 4, 6,
			5, 5, 5, 6,
			7, 7, 7, 7
		};

		return availableRuleByGrade;
	}

	static bool IsAvailableCell(WORD cell, int beltGrade /*int beltLevel*/)
	{
		// ��ȹ �� �ٲ�.. �Ƴ�...
		//const TGradeUnit beltGrade = GetBeltGradeByRefineLevel(beltLevel);		
		const TGradeUnit* ruleTable = GetAvailableRuleTableByGrade();

		return ruleTable[cell] <= beltGrade;
	}

	/// pc�� ��Ʈ �κ��丮�� �������� �ϳ��� �����ϴ� �� �˻��ϴ� �Լ�.
	static bool IsExistItemInBeltInventory(LPCHARACTER pc)
	{
		for (WORD i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			LPITEM beltInventoryItem = pc->GetInventoryItem(i);

			if (NULL != beltInventoryItem)
				return true;
		}

		return false;
	}

	/// item�� ��Ʈ �κ��丮�� �� �� �ִ� Ÿ������ �˻��ϴ� �Լ�. (�� ��Ģ�� ��ȹ�ڰ� ������)
	static bool CanMoveIntoBeltInventory(LPITEM item)
	{
		bool canMove = false;

		if (item->GetType() == ITEM_USE)
		{
			switch (item->GetSubType())
			{
			case USE_POTION:
			case USE_POTION_NODELAY:
			case USE_ABILITY_UP:
				canMove = true;
				break;
			}
		}

		return canMove;
	}

};

#endif //__HEADER_BELT_INVENTORY_HELPER__