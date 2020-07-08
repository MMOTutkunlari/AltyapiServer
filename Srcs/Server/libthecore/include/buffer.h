/*
 *    Filename: buffer.h
 * Description: Buffer ó�� ���
 *
 *      Author: ������ (aka. ��, Cronan), �ۿ��� (aka. myevan, ���ڷ�)
 */
#ifndef __INC_LIBTHECORE_BUFFER_H__
#define __INC_LIBTHECORE_BUFFER_H__

#define SAFE_BUFFER_DELETE(buf)		{ if(buf != NULL) { buffer_delete(buf); buf = NULL; } }

    typedef struct buffer	BUFFER;
    typedef struct buffer *	LPBUFFER;

    struct buffer
    {
	struct buffer * next;

	char *          write_point;
	int             write_point_pos;

	const char *    read_point;
	int             length;

	char *          mem_data;
	int             mem_size;

	long            flag;
    };

	extern LPBUFFER	buffer_new(int size);				// �� ���� ����
    extern void		buffer_delete(LPBUFFER buffer);					// ���� ����
    extern void		buffer_reset(LPBUFFER buffer);					// ���� ���̵��� �ʱ�ȭ

    extern DWORD	buffer_size(LPBUFFER buffer);					// ���ۿ� ���� ����
    extern int		buffer_has_space(LPBUFFER buffer);				// �� �� �ִ� ���̸� ����

    extern void		buffer_write (LPBUFFER& buffer, const void* src, int length);	// ���ۿ� ����.
    extern void		buffer_read(LPBUFFER buffer, void * buf, int bytes);		// ���ۿ��� �д´�.
    extern BYTE		buffer_get_byte(LPBUFFER buffer);
    extern WORD		buffer_get_word(LPBUFFER buffer);
    extern DWORD	buffer_get_dword(LPBUFFER buffer);

    // buffer_proceed �Լ��� buffer_peek���� �б�� �����͸� ���� �޾Ƽ� �� �ʿ䰡
    // ���� �� ó���� ������ �󸶳� ó���� �����ٰ� �뺸�ؾ� �� �� ����. 
    // (buffer_read, buffer_get_* �ø����� ��쿡�� �˾Ƽ� ó�������� peek���� ó������
    //  ���� �׷��� �� ���� �����Ƿ�)
    extern const void *	buffer_read_peek(LPBUFFER buffer);				// �д� ��ġ�� ����
    extern void		buffer_read_proceed(LPBUFFER buffer, int length);		// length��ŭ�� ó���� ����

    // ���������� write_peek���� ���� ��ġ�� ���� ���� �󸶳� �質 �뺸�� ��
    // buffer_write_proceed�� ����Ѵ�.
    extern void *	buffer_write_peek(LPBUFFER buffer);				// ���� ��ġ�� ����
    extern void		buffer_write_proceed(LPBUFFER buffer, int length);		// length�� ���� ��Ų��.

    extern void		buffer_adjust_size(LPBUFFER & buffer, int add_size);		// add_size��ŭ �߰��� ũ�⸦ Ȯ��
#endif
