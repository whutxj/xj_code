 
#ifndef Q_H_H
#define Q_H_H

/*------------------------------------------------------------
// ���нṹ�Ķ���
------------------------------------------------------------*/

typedef struct
{
    short int front;			// ����ͷָ��
    short int  rear;			// ����βָ��
	short int size;				//�ӳ���
	char  flag;					//���ݱ�� 0������ 1������
	unsigned char *elment;//Ԫ��
}QueueType;
 
/*------------------------------------------------------------
// ���еĻ�������
------------------------------------------------------------*/
 
int InitQueue(QueueType *Q,unsigned char *buf,int len);
void DestroyQueue(QueueType *Q);
int QueueEmpty(QueueType *Q);
int QueueFull(QueueType *Q);
int  QueueLength(QueueType *Q);
int EnQueue(QueueType *Q, unsigned char e);
int DeQueue(QueueType *Q, unsigned char *e);
 
#endif 
