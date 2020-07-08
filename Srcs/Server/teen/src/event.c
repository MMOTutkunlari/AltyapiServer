/*
 *    Filename: event.c
 * Description: �̺�Ʈ ���� (timed event)
 *
 *      Author: ������ (aka. ��, Cronan), �ۿ��� (aka. myevan, ���ڷ�)
 */
#include "structs.h"
#include "utils.h"
#include "priority_queue.h"
#include "log.h"


extern int global_pulse;	/* main.c */


static PRIORITY_QUEUE*	event_q = NULL;          /* �̺�Ʈ queue (����Ʈ) */

/* �̺�Ʈ queue �� �ʱ�ȭ �Ѵ� */
void event_init(void)
{/*{{{*/
    sys_log("Intializing Event Engine.");
    event_q = queue_init();
}
/*}}}*/

/* �̺�Ʈ�� �����ϰ� �����Ѵ� */
LPEVENT event_create_ex(EVENTFUNC(*func), void *event_obj, long when, const char *file, int line)
{/*{{{*/
    LPEVENT new_event;

    /* �ݵ�� ���� pulse �̻��� �ð��� ���� �Ŀ� �θ����� �Ѵ�. */
    if (when < 1) 
	when = 1;

    CREATE(new_event, EVENT, 1);

    strncpy(new_event->file, file, 63);
    new_event->line = line;
    new_event->func = func;
    new_event->info = event_obj;
    new_event->q_el = queue_enq(event_q, new_event, when + global_pulse);

    return (new_event);
}/*}}}*/

/* �ý������� ���� �̺�Ʈ�� �����Ѵ� */
void event_cancel(LPEVENT event)
{/*{{{*/
    if (!event) 
    {
	sys_err("event_cancel(): try to cancel NULL event");
	return;
    }

    queue_deq(event_q, event->q_el);

    free(event->info);
    free(event);
}/*}}}*/

void event_reset_time(LPEVENT event, long when)
{/*{{{*/
    event->q_el = queue_reque(event_q, event->q_el, when + global_pulse);
}
/*}}}*/

/* �̺�Ʈ�� ������ �ð��� ������ �̺�Ʈ���� �����Ѵ� */
void event_process(void)
{/*{{{*/
    EVENT	*the_event;
    long	new_time;
    char	last_file[128];
    int		last_line = -1, last_time = -1;

    // event_q �� �̺�Ʈ ť�� ����� �ð����� ������ pulse �� ������ �������� 
    // ���� �ʰ� �ȴ�.
    while ((long) global_pulse >= queue_key(event_q)) 
    {
	new_time = queue_key(event_q);

	if (!(the_event = (LPEVENT) queue_head(event_q))) 
	{
	    sys_err("event_process(): trying to call NULL event");
	    return;
	}

	if (strcmp(the_event->file, last_file) || last_line != the_event->line || last_time != new_time)
	{
	    strcpy(last_file, the_event->file);
	    last_line = the_event->line;
	    last_time = new_time;
//	    sys_log("EVENT: %s %d %d", the_event->file, the_event->line, new_time);
	}

	/*
	 * ���� ���� ���ο� �ð��̸� ���� ���� 0 ���� Ŭ ��� �̺�Ʈ�� �ٽ� �߰��Ѵ�. 
	 * ���� ���� 0 �̻����� �� ��� event �� �Ҵ�� �޸� ������ �������� �ʵ���
	 * �����Ѵ�.
	 */
	if ((new_time = (the_event->func) (the_event)) > 0)
	    the_event->q_el = queue_enq(event_q, the_event, new_time + global_pulse);
	else
	{
	    free(the_event->info);
	    free(the_event);
	}
    }
}/*}}}*/

/* �̺�Ʈ�� ���� �ð��� pulse ������ ������ �ش� */
long event_time(LPEVENT event)
{/*{{{*/
    long when;

    when = queue_elmt_key(event->q_el);
    return (when - global_pulse);
}
/*}}}*/

/* ��� �̺�Ʈ�� �����Ѵ� */
void event_destroy()
{/*{{{*/
    LPEVENT the_event;

    if (!event_q)
	return;

    while ((the_event = (LPEVENT) queue_head(event_q))) 
	free(the_event);

    queue_free(event_q);
    event_q = NULL;
}/*}}}*/
