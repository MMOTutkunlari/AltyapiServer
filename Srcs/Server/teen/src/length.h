#ifndef __INC_METIN_LENGTH_H__
#define __INC_METIN_LENGTH_H__

#define MAX_SQL_LOGIN_LENGTH		8
#define MAX_SQL_PWD_LENGTH         	12
#define MAX_SQL_REAL_NAME_LENGTH        20
#define MAX_SQL_SOCIAL_ID_LENGTH        18
#define MAX_SQL_EMAIL_LENGTH            40
#define MAX_SQL_PHONE_LENGTH            14
#define MAX_SQL_ADDRESS_LENGTH          80
#define MAX_SQL_GAME_ID_LENGTH          20

#define MAX_SQL_IP_ADDR_LENGTH          15
#define MAX_SQL_SERIAL_LENGTH           25
#define MAX_SQL_BANK_PWD_LENGTH         8

#define MAX_NAME_LENGTH			16
#define MAX_ITEM_NAME_LENGTH            32
#define MAX_SOCIAL_LENGTH		16
#define MAX_TITLE_LENGTH		16
#define MAX_PWD_LENGTH			16

#define MAX_GAME_STORES			2

#define HOST_LENGTH			30
#define MAX_HOST_LENGTH			HOST_LENGTH

enum EDescTypes
{
	CLIENT_TYPE_GAME,
	CLIENT_TYPE_ACCOUNT,
	CLIENT_TYPE_UTILS
};

#define HANDSHAKE_XOR			0x6AB3D224

#define MAX_DRAWLIST                    256

#define MAX_ITEMS_ON_CHARACTER          80      // ������ �� �ִ� ������ ����
#define MAX_EXCHANGE_ITEMS_ON_CHARACTER 4       // ��ȯ�� �� �ִ� ������ ����
#define MAX_QITEMS_ON_CHARACTER         10      // ��â�� ���� �� �ִ� ������ ����
#define MAX_ITEMS_ON_BANK               80      // ���࿡ ���� �� �ִ� ������ ����

#define MAX_AFFECTS                     116     // ���� �� �ִ� ȿ��

#define MAX_QUESTS                      256     // �ִ� ����Ʈ ����
#define MAX_QUEST_QUESTIONS             16      // �ִ� ����Ʈ ������
#define MAX_GROUP_MEMBERS               16      // �ִ� �׷� ��� ��

#define MAX_SKILL_PER_TYPE              240
#define MAX_QSKILLS                     10

#define SKILL_TYPE_SKILL                0       // ���
#define SKILL_TYPE_MAGIC                1       // ����
#define SKILL_TYPE_KNOWLEDGE            2       // ����
#define SKILL_TYPE_SPECIAL              2       // ��Ÿ
#define MAX_SKILL_TYPES                 3       // ��� ����

#define GUILD_TYPE_COMBAT               0       // ���� ���� ���
#define GUILD_TYPE_ETC                  1       // ��Ÿ (����) ���� ���
#define GUILD_TYPE_ECONOMY              2       // ���� ���� ���
#define MAX_GUILD_TYPES                 3       // ��� ����

#define MAX_ATTACK_LIST                 32      // ���� ����Ʈ

#define MAX_VOICE_MEMBERS               4

//
// For Characters
//
/* ���� */
enum ESex
{
	SEX_NEUTRAL,
	SEX_FEMALE,
	SEX_MALE,
	NUM_SEX
};

/* ���� */
#define RACE_SENT			 0 // ��Ʈ ����
#define RACE_ELF			 1 // ���� ����
#define RACE_DWARF			 2 // �����
#define NUM_RACES			 3 // ���� ����

/* ���� �� �ִ� ���� */
#define WEAR_HEAD			 1  // �Ӹ�
#define WEAR_NECK			 2  // ��
#define WEAR_BODY			 3  // ��
#define WEAR_HAND			 4  // ��� (�尩)
#define WEAR_FEET			 5  // ���
#define WEAR_R_FINGER			 6  // ������ �հ���
#define WEAR_L_FINGER			 7  // ���� �հ���
#define WEAR_WIELD			 8  // ����
#define WEAR_HOLD			 9  // ���
#define NUM_WEARS			10 // ���� ������ �� ����

//
// For items
//
#define MAX_ITEM_AFFECT                  4       // 0 1 2 3
#define MAX_ITEM_VALUES                  5       // 0 1 2 3 4
#define MAX_ITEM_RESOURCES               4
#define MAX_ITEM_IMPROVE		24	// ������� ������ ������ �� �ִ� �Ѱ�


// by mhh
#define	MAX_COMBINE_COUNT		200


#endif


