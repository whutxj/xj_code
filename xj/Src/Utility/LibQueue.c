
#include "LibQueue.h"
#include "stdio.h"
#include <string.h>
/*------------------------------------------------------------
操作目的：   初始化队列
初始条件：   无
操作结果：   构造一个空的队列
函数参数：
        buf 存放队列元素的缓冲区 len
返回值：
        1 成功 0 失败
------------------------------------------------------------*/
int InitQueue(QueueType *Q,unsigned char *buf,int len)
{
	if(buf == NULL)
	{
		return 0;
	}

	Q->front = 0;
	Q->rear = 0;
	Q->elment = buf;
	Q->size = len;
	Q->flag = 0;
	return 1;
}

/*------------------------------------------------------------
操作目的：   清空一个队列
初始条件：   无
操作结果：   
函数参数：
			Q 需要清空的队列对象
返回值：
			无
------------------------------------------------------------*/
void DestroyQueue(QueueType *Q)
{
	Q->front = 0;
	Q->rear = 0;
	memset(Q->elment,0,Q->size);
	Q->size = 0;
	Q->flag = 0;
}
/*------------------------------------------------------------
操作目的：   判断队列是否为空
初始条件：   无
操作结果：   
函数参数：
			Q 需要判断的队列对象
返回值：
			1 ：空 ；0 ：非空
------------------------------------------------------------*/
int QueueEmpty(QueueType *Q)
{
	if((Q->front == Q->rear) && (Q->flag == 0))
	{
		return 1;
	}

	return 0;
}

/*------------------------------------------------------------
操作目的：   判断队列是否为满
初始条件：   无
操作结果：   
函数参数：
			Q 需要判断的队列对象
返回值：
			1 ：满 ；0 ：没有满
------------------------------------------------------------*/
int QueueFull(QueueType *Q)
{
	if((Q->front == Q->rear) && (Q->flag == 1))
	{
		return 1;
	}

	return 0;
}
/*------------------------------------------------------------
操作目的：   队列数据个数
初始条件：   无
操作结果：   
函数参数：
			Q 需要判断的队列对象
返回值：
			1 ：空 ；0 ：非空
------------------------------------------------------------*/
int  QueueLength(QueueType *Q)
{
	int num =0;

	if(Q->flag == 0)
	{
		num =  Q->rear - Q->front;
	}
	else
	{
		if(Q->front < Q->rear)
		{
			num =  Q->rear - Q->front;
		}
        else if(Q->front == Q->rear)
        {
            num = Q->size;
        }
		else
		{
			num = (Q->size - Q->front) + Q->rear;
		}
	}
	return num;
}

/*------------------------------------------------------------
操作目的：   入列
初始条件：   无
操作结果：   
函数参数：
			Q 需要操作的队列对象
返回值：
			1 ：成功 ；0 ：失败
------------------------------------------------------------*/
int EnQueue(QueueType *Q, unsigned char e)
{
	if(Q->size == 0)
	{
		return 0;
	}

	if((Q->front != Q->rear) || (Q->flag ==0))
	{//非空
		Q->elment[Q->rear++] = e;
		if(Q->rear == Q->size)
		{
			Q->rear = 0;
		}
		if(Q->front == Q->rear)
		{
			Q->flag = 1;
		}
		return 1;
	}
	else
	{
		return 0;
	}

}
/*------------------------------------------------------------
操作目的：   出列
初始条件：   无
操作结果：   
函数参数：
			Q 需要操作的队列对象
返回值：
			1 ：成功 ；0 ：失败
------------------------------------------------------------*/
int DeQueue(QueueType *Q, unsigned char *e)
{
	if(Q->size == 0)
	{
		return 0;
	}

	if((Q->front != Q->rear) || (Q->flag ==1))
	{
		*e = Q->elment[Q->front++];

		if(Q->front == Q->size)
		{
			Q->front = 0;
		}
		if(Q->front != Q->rear)
		{
			Q->flag = 0;
		}

		return 1;
	}

	return 0;
}

