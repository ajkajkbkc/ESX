/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "ether_cat_list.h"

static list_handle_ec_t g_list_handle;

static list_status_ec_e EC_LIST_Error_Check(list_handle_ec_t list, list_element_handle_ec_t newElement)
{
    list_status_ec_e listStatus      = EC_kLIST_Ok;
    list_element_handle_ec_t element = list->head;

    if ((list->max != 0U) && (list->max == list->size))
    {
        listStatus = EC_kLIST_Full; /*List is full*/
    }
    else
    {
        while (element != NULL) /*Scan list*/
        {
            /* Determine if element is duplicated */
            if (element == newElement)
            {
                listStatus = EC_kLIST_DuplicateError;
                break;
            }
            element = element->next;
        }
    }

    return listStatus;
}

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */
/*! *********************************************************************************
 * \brief     Initialises the list descriptor.
 *
 * \param[in] list - LIST_ handle to init.
 *            max - Maximum number of elements in list. 0 for unlimited.
 *
 * \return void.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
void EC_LIST_Init(list_handle_ec_t list, uint32_t max)
{
    g_list_handle = list;
    list->head = NULL;
    list->tail = NULL;
    list->max  = (uint16_t)max;
    list->size = 0;
}

/*! *********************************************************************************
 * \brief     Gets the list that contains the given element.
 *
 * \param[in] element - Handle of the element.
 *
 * \return NULL if element is orphan.
 *         Handle of the list the element is inserted into.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_handle_ec_t EC_LIST_GetList(list_element_handle_ec_t element)
{
    return element->list;
}

/*! *********************************************************************************
 * \brief     Links element to the tail of the list.
 *
 * \param[in] list - ID of list to insert into.
 *            element - element to add
 *
 * \return EC_kLIST_Full if list is full.
 *         EC_kLIST_Ok if insertion was successful.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_status_ec_e EC_LIST_AddTail(list_handle_ec_t list, list_element_handle_ec_t element)
{
    uint32_t regPrimask      = DisableGlobalIRQ();
    list_status_ec_e listStatus = EC_kLIST_Ok;
#if 0
    listStatus = EC_LIST_Error_Check(list, element);
    if (listStatus == EC_kLIST_Ok) /* Avoiding list status error */
#endif
    {
        if (list->size == 0U)
        {
            list->head = element;
        }
        else
        {
            list->tail->next = element;
        }
        element->prev = list->tail;
        element->list = list;
        element->next = NULL;
        list->tail    = element;
        list->size++;
    }

    EnableGlobalIRQ(regPrimask);
    return listStatus;
}

/*! *********************************************************************************
 * \brief     Links element to the head of the list.
 *
 * \param[in] list - ID of list to insert into.
 *            element - element to add
 *
 * \return EC_kLIST_Full if list is full.
 *         EC_kLIST_Ok if insertion was successful.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_status_ec_e EC_LIST_AddHead(list_handle_ec_t list, list_element_handle_ec_t element)
{
    uint32_t regPrimask      = DisableGlobalIRQ();
    list_status_ec_e listStatus = EC_kLIST_Ok;

    listStatus = EC_LIST_Error_Check(list, element);
    if (listStatus == EC_kLIST_Ok) /* Avoiding list status error */
    {
        /* Links element to the head of the list */
        if (list->size == 0U)
        {
            list->tail = element;
        }
        else
        {
            list->head->prev = element;
        }
        element->prev = NULL;
        element->list = list;
        element->next = list->head;
        list->head    = element;
        list->size++;
    }

    EnableGlobalIRQ(regPrimask);
    return listStatus;
}

/*! *********************************************************************************
 * \brief     Unlinks element from the head of the list.
 *
 * \param[in] list - ID of list to remove from.
 *
 * \return NULL if list is empty.
 *         ID of removed element(pointer) if removal was successful.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_element_handle_ec_t EC_LIST_RemoveHead(void)
{
    list_element_handle_ec_t element;

    uint32_t regPrimask = DisableGlobalIRQ();

    if ((NULL == g_list_handle) || (g_list_handle->size == 0U))
    {
        element = NULL; /*LIST_ is empty*/
    }
    else
    {
        element = g_list_handle->head;
        g_list_handle->size--;
        if (g_list_handle->size == 0U)
        {
            g_list_handle->tail = NULL;
        }
        else
        {
            element->next->prev = NULL;
        }
        element->list = NULL;
        g_list_handle->head    = element->next; /*Is NULL if element is head*/
    }

    EnableGlobalIRQ(regPrimask);
    return element;
}

/*! *********************************************************************************
 * \brief     Gets head element ID.
 *
 * \param[in] list - ID of list.
 *
 * \return NULL if list is empty.
 *         ID of head element if list is not empty.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_element_handle_ec_t EC_LIST_GetHead(list_handle_ec_t list)
{
    return list->head;
}

/*! *********************************************************************************
 * \brief     Gets next element ID.
 *
 * \param[in] element - ID of the element.
 *
 * \return NULL if element is tail.
 *         ID of next element if exists.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_element_handle_ec_t EC_LIST_GetNext(list_element_handle_ec_t element)
{
    return element->next;
}

/*! *********************************************************************************
 * \brief     Gets previous element ID.
 *
 * \param[in] element - ID of the element.
 *
 * \return NULL if element is head.
 *         ID of previous element if exists.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_element_handle_ec_t EC_LIST_GetPrev(list_element_handle_ec_t element)
{
    return element->prev;
}

/*! *********************************************************************************
 * \brief     Unlinks an element from its list.
 *
 * \param[in] element - ID of the element to remove.
 *
 * \return EC_kLIST_OrphanElement if element is not part of any list.
 *         EC_kLIST_Ok if removal was successful.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_status_ec_e EC_LIST_RemoveElement(list_element_handle_ec_t element)
{
    list_status_ec_e listStatus = EC_kLIST_Ok;
    uint32_t regPrimask      = DisableGlobalIRQ();

    if (element->list == NULL)
    {
        listStatus = EC_kLIST_OrphanElement; /*Element was previusly removed or never added*/
    }
    else
    {
        if (element->prev == NULL) /*Element is head or solo*/
        {
            element->list->head = element->next; /*is null if solo*/
        }
        if (element->next == NULL) /*Element is tail or solo*/
        {
            element->list->tail = element->prev; /*is null if solo*/
        }
        if (element->prev != NULL) /*Element is not head*/
        {
            element->prev->next = element->next;
        }
        if (element->next != NULL) /*Element is not tail*/
        {
            element->next->prev = element->prev;
        }
        element->list->size--;
        element->list = NULL;
    }

    EnableGlobalIRQ(regPrimask);
    return listStatus;
}

/*! *********************************************************************************
 * \brief     Links an element in the previous position relative to a given member
 *            of a list.
 *
 * \param[in] element - ID of a member of a list.
 *            newElement - new element to insert before the given member.
 *
 * \return EC_kLIST_OrphanElement if element is not part of any list.
 *         EC_kLIST_Full if list is full.
 *         EC_kLIST_Ok if insertion was successful.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
list_status_ec_e EC_LIST_AddPrevElement(list_element_handle_ec_t element, list_element_handle_ec_t newElement)
{
    list_status_ec_e listStatus = EC_kLIST_Ok;
    uint32_t regPrimask      = DisableGlobalIRQ();

    if (element->list == NULL)
    {
        listStatus = EC_kLIST_OrphanElement; /*Element was previusly removed or never added*/
    }
    else
    {
        listStatus = EC_LIST_Error_Check(element->list, newElement);
        if (listStatus == EC_kLIST_Ok)
        {
            if (element->prev == NULL) /*Element is list head*/
            {
                element->list->head = newElement;
            }
            else
            {
                element->prev->next = newElement;
            }
            newElement->list = element->list;
            element->list->size++;
            newElement->next = element;
            newElement->prev = element->prev;
            element->prev = newElement;
        }
    }

    EnableGlobalIRQ(regPrimask);
    return listStatus;
}

/*! *********************************************************************************
 * \brief     Gets the current size of a list.
 *
 * \param[in] list - ID of the list.
 *
 * \return Current size of the list.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
uint32_t EC_LIST_GetSize(list_handle_ec_t list)
{
    return list->size;
}

/*! *********************************************************************************
 * \brief     Gets the number of free places in the list.
 *
 * \param[in] list - ID of the list.
 *
 * \return Available size of the list.
 *
 * \pre
 *
 * \post
 *
 * \remarks
 *
 ********************************************************************************** */
uint32_t EC_LIST_GetAvailableSize(list_handle_ec_t list)
{
    return ((uint32_t)list->max - (uint32_t)list->size); /*Gets the number of free places in the list*/
}
