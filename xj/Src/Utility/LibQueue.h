 
#ifndef Q_H_H
#define Q_H_H

/*------------------------------------------------------------
// 队列结构的定义
------------------------------------------------------------*/

typedef struct
{
    short int front;			// 队列头指针
    short int  rear;			// 队列尾指针
	short int size;				//队长度
	char  flag;					//数据标记 0无数据 1有数据
	unsigned char *elment;//元素
}QueueType;
 
/*------------------------------------------------------------
// 队列的基本操作
------------------------------------------------------------*/
 
int InitQueue(QueueType *Q,unsigned char *buf,int len);
void DestroyQueue(QueueType *Q);
int QueueEmpty(QueueType *Q);
int QueueFull(QueueType *Q);
int  QueueLength(QueueType *Q);
int EnQueue(QueueType *Q, unsigned char e);
int DeQueue(QueueType *Q, unsigned char *e);
 
#endif 
