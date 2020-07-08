#pragma once

#include <string.h>
#include <string>
#include <ctype.h>
#include <map>
#include <assert.h>

#ifdef _WIN32
#include <WinSock2.h>
#define FORCEINLINE __forceinline
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define FORCEINLINE inline
#define strcpy_s(__dst, __len, __src) strncpy(__dst, __src, __len)
#define _stricmp(__dst, __src) strcasecmp(__dst, __src)
#define strncpy_s(__dst, __dstlen, __src, __srclen) strncpy(__dst, __src, __srclen)
#define _snprintf snprintf
#endif

class SIM
{
public:
	SIM() {};

	enum
	{
		SIM_IP_MAC_TYPE,
		SIM_IP_RANGE_TYPE,
		SIM_IP_TYPE,
	};

	//
	// SIM ������ �� line�� �Ľ��Ѵ�.
	//
	bool ParseLine(const char* line)
	{
		char buf[1024];
		strcpy_s(buf, sizeof(buf), line);

		// \t�� ã�ƺ��� ������, [IP]\t[MAC]\n ��Ģ���� �Ľ��Ѵ�.
		char* tpos = strchr(buf, '\t');
		if (tpos)
		{
			*tpos = 0;

			// ip�� mac
			unsigned int ip = htonl(inet_addr(buf));
			char* mac = tpos + 1;

			if (false == checkmac(mac))
			{
				printf("invalid MAC address - mac(%s)\n", mac);
				return false;
			}

			printf("IP-MAC type added - ip(%u) mac(%s)\n", ip, mac);

			macips.insert(MAC_IP_map::value_type(mac, ip));
			return true;
		}

		// ~�� ã�ƺ��� ������, [IP]~[IP]\n ��Ģ���� �Ľ��Ѵ�.
		char* tilt = strchr(buf, '~');
		if (tilt)
		{
			*tilt = 0;

			// begin ip�� end ip
			char* bip = buf;
			char* eip = tilt + 1;

			IPRANGE ipr;
			ipr.fromip = htonl(inet_addr(bip));
			ipr.toip = htonl(inet_addr(eip));

			printf("IP RANGE type added - begin[%u] end[%u]\n", ipr.fromip, ipr.toip);

			ipranges.push_back(ipr);
			return true;
		}

		// \t�� ~�� ������, [IP]\n ��Ģ���� �Ľ��Ѵ�.
		unsigned int ip = htonl(inet_addr(buf));

		printf("single IP type added - ip(%u)\n", ip);

		ips.push_back(ip);
		return true;
	}

	// ParseLine()���� ���� �����͸� binary�� ��ȯ
	void MakeBinary(std::vector<unsigned char>& s)
	{
		//
		// buf�� binary ���·� ���
		//
		for (MAC_IP_map::iterator i = macips.begin(); i != macips.end(); ++i)
		{
			const std::string& mac = i->first;
			unsigned int ip = i->second;

			unsigned char macbuf[6];
			mac2bin(mac, macbuf);

			unsigned char type = SIM_IP_MAC_TYPE;

			// [type(1)][ip(4)][mac(6)] �� 11����Ʈ
			s.push_back(type);
			s.insert(s.end(), (unsigned char *)&ip, (unsigned char *)&ip + sizeof(ip));
			s.insert(s.end(), macbuf, macbuf + sizeof(macbuf));
		}

		for (IPRanges::iterator i = ipranges.begin(); i != ipranges.end(); ++i)
		{
			unsigned int bip = (*i).fromip;
			unsigned int eip = (*i).toip;

			unsigned char type = SIM_IP_RANGE_TYPE;

			// [type(1)][ip(4)][ip(4)]
			s.push_back(type);
			s.insert(s.end(), (unsigned char *)&bip, (unsigned char *)&bip + sizeof(bip));
			s.insert(s.end(), (unsigned char *)&eip, (unsigned char *)&eip + sizeof(eip));
		}

		for (IPs::iterator i = ips.begin(); i != ips.end(); ++i)
		{
			unsigned int ip = *i;

			unsigned char type = SIM_IP_TYPE;

			// [type(1)][ip(4)]
			s.push_back(type);
			s.insert(s.end(), (unsigned char *)&ip, (unsigned char *)&ip + sizeof(ip));
		}
	}

	// sim binary�� parsing�ؼ� ������ ä��
	bool ParseBinary(const char* buf, int buflen)
	{
		unsigned char* iter = (unsigned char *)buf;
		while (iter != (unsigned char *)buf + buflen)
		{
			unsigned char type = *iter++;
			switch (type)
			{
			case SIM_IP_MAC_TYPE:
				// [type(1)][ip(4)][mac(6)]
				{
					unsigned int ip = 0;
					memcpy(&ip, iter, sizeof(ip));
					iter += sizeof(ip);

					char mac[18];	// AA-BB-CC-DD-EE-FF
					bin2mac(iter, mac);
					iter += 6;

#ifdef _DEBUG
					printf("IP-MAC type added - ip(%u) mac(%s)\n", ip, mac);
#endif

					macips.insert(MAC_IP_map::value_type(mac, ip));
					break;
				}
			case SIM_IP_RANGE_TYPE:
				// [type(1)][ip(4)][ip(4)]
				{
					unsigned int bip = 0;
					memcpy(&bip, iter, sizeof(bip));
					iter += sizeof(bip);

					unsigned int eip = 0;
					memcpy(&eip, iter, sizeof(eip));
					iter += sizeof(eip);

					IPRANGE ipr;
					ipr.fromip = bip;
					ipr.toip = eip;

#ifdef _DEBUG
					printf("IP RANGE type added - begin[%u] end[%u]\n", ipr.fromip, ipr.toip);
#endif

					ipranges.push_back(ipr);
					break;
				}
			case SIM_IP_TYPE:
				// [type(1)][ip(4)]
				{
					unsigned int ip = 0;
					memcpy(&ip, iter, sizeof(ip));
					iter += sizeof(ip);

#ifdef _DEBUG
					printf("single IP type added - ip(%u)\n", ip);
#endif

					ips.push_back(ip);
					break;
				}
			default:
#ifdef _DEBUG
				printf("parseBinary: invalid type (%u)\n", type);
#endif
				return false;
			}
		}
		return true;
	}

	FORCEINLINE bool CheckIpAndMac(const char* ip, const char* mac)
	{
		assert(ip);
		assert(mac);

		unsigned int uip = htonl(inet_addr(ip));

		// ip & mac üũ
		for (MAC_IP_map::iterator i = macips.begin(); i != macips.end(); ++i)
		{
			const char* allowmac = i->first.c_str();
			unsigned int allowip = i->second;

			// ip�� ������ mac�� �ٸ� ���
			if (allowip == uip && _stricmp(allowmac, mac))
			{
				return false;
			}

			// mac�� ������ ip�� �ٸ� ���
			if (_stricmp(allowmac, mac) == 0 && allowip != uip)
			{
				return false;
			}

			// ip�� mac�� ��� ���� ���
			if (allowip == uip && _stricmp(allowmac, mac) == 0)
			{
				return true;
			}

			// ip�� mac�� ��� �ٸ� ���� pass
		}

		// ���� üũ
		for (size_t i = 0; i < ipranges.size(); i++)
		{
			if (ipranges[i].fromip <= uip && uip <= ipranges[i].toip)
			{
				return true;
			}
		}

		// �̱� ip üũ
		for (size_t i = 0; i < ips.size(); i++)
		{
			if (uip == ips[i])
			{
				return true;
			}
		}

		return false;
	}

	typedef std::map<std::string, unsigned int> MAC_IP_map;	// MAC(string)�� Ű�� ip(uint)�� ã��
	typedef std::vector<unsigned int> IPs;					// single ip(uint)
	struct IPRANGE
	{
		unsigned int fromip;	// ip range (begin)
		unsigned int toip;		// ip ragne (end)
	};
	typedef std::vector<IPRANGE> IPRanges;

private:
	bool checkmac(const char* mac)
	{
		size_t len = strlen(mac);
		if (len != 12 && len != 17)
		{
			// ���� �̻�
			printf("checkmac - invalid length (%s)(%u)\n", mac, len);
			return false;
		}

		for (size_t i = 0; i < len; i++)
		{
			// ������(-) üũ
			if (len == 17 && i % 3 == 2)
			{
				if (mac[i] == '-')
				{
					continue;
				}
				else
				{
					printf("checkmac - cannot find '-' (%s)\n", mac);
					return false;
				}
			}

			// ���� ���� �˻�
			if (mac[i] >= '0' && mac[i] <= '9') continue;
			if (mac[i] >= 'A' && mac[i] <= 'F') continue;
			if (mac[i] >= 'a' && mac[i] <= 'f') continue;

			// �˼� ���� ����
			printf("checkmac - invalid character (%s)(%c)\n", mac, mac[i]);
			return false;
		}

		return true;
	}

	bool mac2bin(const std::string& mac, unsigned char* outbuf)
	{
		assert(mac.size() == 12 || mac.size() == 17);	// AABBCCDDEEFF �����̰ų� AA-BB-CC-DD-EE-FF �����̰ų�

		if (mac.size() == 12)
		{
			for (size_t i = 0, k = 0; i < mac.size(); i += 2, k++)
			{
				char buf[3] = {0, };
				strncpy_s(buf, sizeof(buf), &mac[i], 2);

				unsigned char b = atoi(buf);
				outbuf[k] = b;
			}
			return true;
		}
		else if (mac.size() == 17)
		{
			for (size_t i = 0, k = 0; i < mac.size(); i += 3, k++)
			{
				char buf[3] = {0, };
				strncpy_s(buf, sizeof(buf), &mac[i], 2);

				char* endptr = buf + 2;
				unsigned char b = (unsigned char)strtol(buf, &endptr, 16);
				outbuf[k] = b;
			}
			return true;
		}
		return false;
	}

	void bin2mac(unsigned char* bin, char* outbuf)
	{
		for (int i = 0; i < 6; i++)
		{
			_snprintf(outbuf + (i*3), 3, "%02X", bin[i]);

			// �������� ������(-) �� ����
			if (i < 5)
			{
				strcat(outbuf, "-");
			}
		}
	}

private:
	MAC_IP_map macips;	// mac-ip ������ ���� �˻��Ѵ�. (�켱���� ����)
	IPRanges ipranges;
	IPs ips;
};
