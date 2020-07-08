#ifndef _M2_CHECK_SERVER_KEY_H_
#define _M2_CHECK_SERVER_KEY_H_

#include <string>
#include <vector>
#include "CheckServerKey.h"

class CCheckServer : public singleton<CCheckServer>
{
public:
	CCheckServer()
		: m_bValid(FALSE)
	{}

	FORCEINLINE void AddServerKey(const char* pServerKey) {
		m_vKey.push_back(pServerKey);
	}

	FORCEINLINE BOOL CheckIP(const char* pIP) {
		m_bValid = FALSE;

		// 디파인 안걸리면 체크 안함
#ifndef _USE_SERVER_KEY_
		m_bValid = TRUE;
		return m_bValid;
#endif
		for (UINT i = 0; i < m_vKey.size(); i++) {
			// 하나라도 맞는 서버키가 있으면 ok
			std::string errorString;
			if (CheckServerKey(m_vKey[i].c_str(), pIP, "", errorString)) {
				m_bValid = TRUE;
				break;
			}
		}
		return m_bValid;
	}

	FORCEINLINE BOOL IsValid() const {
		return m_bValid;
	}
	
private:
	std::vector<std::string> m_vKey;
	BOOL m_bValid;
};

#endif // #ifndef _M2_CHECK_SERVER_KEY_H_
