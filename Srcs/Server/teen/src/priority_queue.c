/*
 *    Filename: queue.c
 * Description: ť ó��
 *
 *      Author: ������ (aka. ��, Cronan), �ۿ��� (aka. myevan, ���ڷ�)
 */
#include "structs.h"
#include "utils.h"
#include "priority_queue.h"

extern int global_pulse;

/* ť�� �ʱ�ȭ �Ѵ� */
PRIORITY_QUEUE *queue_init(void)
{/*{{{*/
    PRIORITY_QUEUE *q;

    CREATE(q, PRIORITY_QUEUE, 1);

    return (q);
}
/*}}}*/

/* ť�� enqueue �Ѵ�. */
PRIORITY_Q_ELEMENT *queue_enq(PRIORITY_QUEUE *q, void *data, int key)
{/*{{{*/
    PRIORITY_Q_ELEMENT	*qe, *i;
    int			bucket;

    /* fast hasing */
    bucket	= key % NUM_PRIORITY_QUEUES;

    CREATE(qe, PRIORITY_Q_ELEMENT, 1);

    qe->data	= data;
    qe->key	= key;
    qe->prev	= NULL;
    qe->next	= NULL;

    if (!q->head[bucket]) 
    {
	// ť�� �����.
	q->head[bucket] = qe;
	q->tail[bucket] = qe;
    }
    else
    {
	for (i = q->tail[bucket]; i; i = i->prev) 
	{
	    if (i->key < key) 
	    { 
		// ���� ���� ã�Ҵ�.
		if (i == q->tail[bucket])
		    q->tail[bucket]	= qe;
		else 
		{
		    qe->next		= i->next;
		    i->next->prev	= qe;
		}

		qe->prev = i;
		i->next = qe;
		break;
	    }
	}

	if (i == NULL) 
	{ 
	    // ���� ���� ����Ʈ�� ó�� �̴�.
	    qe->next		= q->head[bucket];
	    q->head[bucket]	= qe;
	    qe->next->prev	= qe;
	}
    }

    return (qe);
}/*}}}*/

/* ť�� dequeue �Ѵ�. */
void queue_deq(PRIORITY_QUEUE *q, PRIORITY_Q_ELEMENT *qe)
{/*{{{*/
    int i;

    if (qe == NULL) 
    {
	//core_dump();
	return;
    }

    i = qe->key % NUM_PRIORITY_QUEUES;

    if (qe->prev == NULL)
	q->head[i] = qe->next;
    else
	qe->prev->next = qe->next;

    if (qe->next == NULL)
	q->tail[i] = qe->prev;
    else
	qe->next->prev = qe->prev;

    free(qe);
}
/*}}}*/

PRIORITY_Q_ELEMENT *queue_reque(PRIORITY_QUEUE *q, PRIORITY_Q_ELEMENT *qe, int new_key)
{/*{{{*/
    void	*data;

    data = qe->data;

    queue_deq(q, qe);
    return (queue_enq(q, data, new_key));
}
/*}}}*/

/*
 * ť�� ��带 dequeue �� �����Ѵ�.
 */
void *queue_head(PRIORITY_QUEUE *q)
{/*{{{*/
    void *data;
    int i;

    i = (global_pulse % NUM_PRIORITY_QUEUES);

    if (!q->head[i])
	return (NULL);

    data = q->head[i]->data;
    queue_deq(q, q->head[i]);

    return (data);
}
/*}}}*/

/*
 * ���ڰ� ���� ���� (ť�� ���) key �� �����Ѵ�.
 * ť�� NULL �� ��� unsinged number �� �ְ���� �����Ѵ�.
 */
int queue_key(PRIORITY_QUEUE *q)
{/*{{{*/
    int i;

    i = global_pulse % NUM_PRIORITY_QUEUES;

    if (q->head[i])
	return q->head[i]->key;
    else
	return LONG_MAX;
}
/*}}}*/

/* key �� �����Ѵ�. */
int queue_elmt_key(PRIORITY_Q_ELEMENT *qe)
{/*{{{*/
    return qe->key;
}
/*}}}*/

/* ť�� ������ �����Ѵ�. */
int queue_count(PRIORITY_QUEUE *q)
{/*{{{*/
    int i, count = 0;
    PRIORITY_Q_ELEMENT *qe, *next_qe;

    for (i = 0; i < NUM_PRIORITY_QUEUES; i++)
    {
	for (qe = q->head[i]; qe; qe = next_qe)
	{
	    next_qe = qe->next;
	    count++;
	}
    }
    
    return (count);
}
/*}}}*/

/* ť�� free �Ѵ�. */
void queue_free(PRIORITY_QUEUE *q)
{/*{{{*/
    int i;
    PRIORITY_Q_ELEMENT *qe, *next_qe;

    for (i = 0; i < NUM_PRIORITY_QUEUES; i++)
    {
	for (qe = q->head[i]; qe; qe = next_qe) 
	{
	    next_qe = qe->next;
	    free(qe);
	}
    }

    free(q);
}/*}}}*/
