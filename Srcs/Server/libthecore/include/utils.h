#ifndef __INC_LIBTHECORE_UTILS_H__
#define __INC_LIBTHECORE_UTILS_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define SAFE_FREE(p)		{ if (p) { free( (void *) p);		(p) = NULL;  } }
#define SAFE_DELETE(p)		{ if (p) { delete (p);			(p) = NULL;  } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);		(p) = NULL;  } }
#define SAFE_RELEASE(p)		{ if (p) { (p)->Release();		(p) = NULL;  } }

#define LOWER(c)	(((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)	(((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c))

#define str_cmp strcasecmp
#define STRNCPY(dst, src, len)          do {strncpy(dst, src, len); dst[len] = '\0'; } while(0)

    extern char *	str_dup(const char * source);	// �޸� �Ҵ� �ؼ� source ���� �Ѱ� ����
    extern void		printdata(const unsigned char * data, int bytes); // data�� hex�� ascii�� ��� (��Ŷ �м� � ����)
    extern int		filesize(FILE * fp);	// ���� ũ�� ����

#define core_dump()	core_dump_unix(__FILE__, __LINE__)
    extern void		core_dump_unix(const char *who, WORD line);	// �ھ ������ ����

#define TOKEN(string) if (!str_cmp(token_string, string))
    // src = ��ū : ��
    extern void		parse_token(char * src, char * token, char * value);

    extern void		trim_and_lower(const char * src, char * dest, size_t dest_size);

    // ���ڿ��� �ҹ��ڷ�
    extern void		lower_string(const char * src, char * dest, size_t dest_len);

    // arg1�� arg2�� �����ϴ°�? (��ҹ��� �������� ����)
    extern int		is_abbrev(char *arg1, char *arg2);

    // a�� b�� �ð��� �󸶳� ���̳����� ����
    extern struct timeval *	timediff(const struct timeval *a, const struct timeval *b);

    // a�� �ð��� b�� �ð��� ���� ����
    extern struct timeval *	timeadd(struct timeval *a, struct timeval *b);

    // ���� �ð� curr_tm���� ���� days�� ���� ���� ����
    extern struct tm *		tm_calc(const struct tm *curr_tm, int days);

    extern int MAX(int a, int b); // ���߿� ū ���� ����
    extern int MIN(int a, int b); // ���߿� ���� ���� ����
    extern int MINMAX(int min, int value, int max); // �ּ� �ִ� ���� �Բ� ���ؼ� ����

    extern int		number_ex(int from, int to, const char *file, int line); // from���� ���� to������ ���� �� ����
#define number(from, to) number_ex(from, to, __FILE__, __LINE__)

	float	fnumber(float from, float to);

    extern void		thecore_sleep(struct timeval * timeout);	// timeout��ŭ ���μ��� ����
    extern DWORD	thecore_random();				// ���� �Լ�

    extern float	get_float_time();
    extern DWORD	get_dword_time();

    extern char *	time_str(time_t ct);

#define CREATE(result, type, number)  do { \
	if (!((result) = (type *) calloc ((number), sizeof(type)))) { \
		sys_err("calloc failed [%d] %s", errno, strerror(errno)); \
		abort(); } } while(0)

#define RECREATE(result,type,number) do { \
	if (!((result) = (type *) realloc ((result), sizeof(type) * (number)))) { \
		sys_err("realloc failed [%d] %s", errno, strerror(errno)); \
		abort(); } } while(0)

    // Next �� Prev �� �ִ� ����Ʈ�� �߰�
#define INSERT_TO_TW_LIST(item, head, prev, next)   \
    if (!(head))                                    \
    {                                               \
	head         = item;                        \
	    (head)->prev = (head)->next = NULL;         \
    }                                               \
    else                                            \
    {                                               \
	(head)->prev = item;                        \
	    (item)->next = head;                    \
	    (item)->prev = NULL;                    \
	    head         = item;                    \
    }

#define REMOVE_FROM_TW_LIST(item, head, prev, next)	\
    if ((item) == (head))           			\
    {                               			\
	if (((head) = (item)->next))			\
	    (head)->prev = NULL;    			\
    }                    				\
    else                 				\
    {                    				\
	if ((item)->next)				\
	    (item)->next->prev = (item)->prev;		\
							\
	if ((item)->prev)				\
	    (item)->prev->next = (item)->next;		\
    }


#define INSERT_TO_LIST(item, head, next)            \
    (item)->next = (head);                      \
	(head) = (item);                            \

#define REMOVE_FROM_LIST(item, head, next)          \
	if ((item) == (head))                       \
	    head = (item)->next;                     \
	else                                        \
	{                                           \
	    temp = head;                            \
		\
		while (temp && (temp->next != (item)))  \
		    temp = temp->next;                  \
			\
			if (temp)                               \
			    temp->next = (item)->next;          \
	}                                           \

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                      \
                ((DWORD)(BYTE) (ch0       ) | ((DWORD)(BYTE) (ch1) <<  8) | \
                 ((DWORD)(BYTE) (ch2) << 16) | ((DWORD)(BYTE) (ch3) << 24))
#endif // defined(MAKEFOURCC)

#ifdef __cplusplus
}
#endif	// __cplusplus

// _countof for gcc/g++
#if !defined(_countof)
#if !defined(__cplusplus)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
extern "C++"
{
	    template <typename _CountofType, size_t _SizeOfArray>
			        char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) sizeof(*__countof_helper(_Array))
}
#endif
#endif

#ifdef __WIN32__
extern void gettimeofday(struct timeval* t, struct timezone* dummy);
#endif

#endif	// __INC_UTILS_H__
