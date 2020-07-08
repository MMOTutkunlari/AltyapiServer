#ifndef __METIN2_BUFF_ON_ATTRIBUTES_H
#define __METIN2_BUFF_ON_ATTRIBUTES_H

class CHARACTER;

class CBuffOnAttributes
{
public:
	CBuffOnAttributes(LPCHARACTER pOwner, BYTE m_point_type, std::vector <BYTE>* vec_buff_targets);
	~CBuffOnAttributes();

	// ���� �� �̸鼭, m_p_vec_buff_wear_targets�� �ش��ϴ� �������� ���, �ش� ���������� ���� ���� ȿ���� ����.
	void RemoveBuffFromItem(LPITEM pItem);
	// m_p_vec_buff_wear_targets�� �ش��ϴ� �������� ���, �ش� �������� attribute�� ���� ȿ�� �߰�.
	void AddBuffFromItem(LPITEM pItem);
	// m_bBuffValue�� �ٲٰ�, ������ ���� �ٲ�.
	void ChangeBuffValue(BYTE bNewValue);

	// m_p_vec_buff_wear_targets�� �ش��ϴ� ��� �������� attribute�� type���� �ջ��ϰ�,
	// �� attribute���� (m_bBuffValue)% ��ŭ�� ������ ��.
	bool On(BYTE bValue);
	// ���� ���� ��, �ʱ�ȭ
	void Off();

	void Initialize();
private:
	LPCHARACTER m_pBuffOwner;
	BYTE m_bPointType;
	BYTE m_bBuffValue;
	std::vector <BYTE>* m_p_vec_buff_wear_targets;
	
	// apply_type, apply_value ����� ��
	typedef std::map <BYTE, int> TMapAttr;
	// m_p_vec_buff_wear_targets�� �ش��ϴ� ��� �������� attribute�� type���� �ջ��Ͽ� ������ ����.
	TMapAttr m_map_additional_attrs;

};

#endif
