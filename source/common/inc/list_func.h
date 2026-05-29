/**
  ******************************************************************************
  * @file    list_func.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   移植LINUX双向链表基本操作
  ******************************************************************************
  */
#ifndef __COM_LIST_FUNC_H
#define __COM_LIST_FUNC_H

//#include "stm32f4xx.h"
#include "FreeRTOS.h"

/*list head 结构体定义*/
#pragma pack(1)
struct list_head {
    struct list_head *prev, *next;
};
#pragma pack()

/*链表头定义及初始化宏*/
#define LIST_HEAD_INIT(name)    { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do{ \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
}while(0)

/**
  * @brief  插入节点
  * @param  None
  * @retval None
  */
static portINLINE void __list_add(struct list_head *new,
                                struct list_head *prev,
                                struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
  * @brief  双向链表头插入节点
  * @param  None
  * @retval None
  */

static portINLINE void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}


/**
  * @brief  双向链表尾插入节点
  * @param  None
  * @retval None
  */

static portINLINE void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

/**
  * @brief  删除链表节点
  * @param  None
  * @retval None
  */
static portINLINE void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

static portINLINE void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->prev = (void *)0;
    entry->next = (void *)0;
}

/**
  * @brief  通过结构体的实例成员变量ptr, 获取结构体的体入口地址
  * @param  None
  * @retval None
  */
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
  * @brief  遍历列表
  * @param  None
  * @retval None
  */
#define list_for_each_safe(pos, n, head) \
    for(pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#endif /*__COM_LIST_FUNC_H*/
