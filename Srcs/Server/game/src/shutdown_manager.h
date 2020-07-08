#ifndef __INC_METIN_II_SHUTDOWNMANAGER_H__
#define __INC_METIN_II_SHUTDOWNMANAGER_H__

// CONFIG에서 SHUTDOWN_ENABLE FLAG를 0이 아닌 숫자로 지정할경우 활성화됨.
class CShutdownManager : public singleton<CShutdownManager>
{
public:
	CShutdownManager();
	virtual ~CShutdownManager();

	void AddDesc(LPDESC pDesc);
	void RemoveDesc(LPDESC pDesc);
	int SearchDesc(LPDESC pDesc);	// m_lstDesc에서 찾으면 대상의 index 리턴. 없으면 -1.

	void Update();

private:
	std::list<LPDESC> m_lstDesc;	// AddDesc에서 선별.
	struct tm* m_pTime; time_t m_lTime;
	bool m_bShutdownAlarm;

	void inline UpdateTime() { time(&m_lTime); m_pTime = localtime(&m_lTime); }

public:
	bool CheckShutdownAge(const char* szSocialID);
	bool CheckShutdownTime();
	bool CheckCorrectSocialID(const char* szSocialID);
	bool inline CheckLocale() { return (LC_IsYMIR() || LC_IsWE_Korea()) && gShutdownEnable; }

private:
	int inline CharToInt(char c) { if(c >= '0' && c <= '9') return c - '0'; else return 0;} // char -> int
};
#endif
