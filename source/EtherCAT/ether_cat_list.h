/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ETHERCAT_LIST_H_
#define _ETHERCAT_LIST_H_

#include "fsl_common.h"
#include "ethercattype.h"

/*!
 * @addtogroup GenericList
 * @{
 */

/*!*********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public macro definitions
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
/*! @brief The list status */
typedef enum _list_status_ec
{
    EC_kLIST_Ok             = kStatus_Success,                   /*!< Success */
    EC_kLIST_DuplicateError = MAKE_STATUS(kStatusGroup_LIST, 1), /*!< Duplicate Error */
    EC_kLIST_Full           = MAKE_STATUS(kStatusGroup_LIST, 2), /*!< FULL */
    EC_kLIST_Empty          = MAKE_STATUS(kStatusGroup_LIST, 3), /*!< Empty */
    EC_kLIST_OrphanElement  = MAKE_STATUS(kStatusGroup_LIST, 4), /*!< Orphan Element */
    EC_kLIST_NotSupport     = MAKE_STATUS(kStatusGroup_LIST, 5), /*!< Not Support  */
} list_status_ec_e;

/*! @brief The list structure*/
typedef struct list_label_ec
{
    struct list_element_ec_tag *head; /*!< list head */
    struct list_element_ec_tag *tail; /*!< list tail */
    uint16_t size;                 /*!< list size */
    uint16_t max;                  /*!< list max number of elements */
} list_label_ec_t, *list_handle_ec_t;

/*! @brief The list element*/
typedef struct list_element_ec_tag
{
    struct list_element_ec_tag *next; /*!< next list element   */
    struct list_element_ec_tag *prev; /*!< previous list element */
    struct list_label_ec *list;       /*!< pointer to the list */

    uint8_t data[EC_BUFSIZE];
    uint16_t length;
} list_element_ec_t, *list_element_handle_ec_t;

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */
/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */
/*!
 * @brief Initialize the list.
 *
 * This function initialize the list.
 *
 * @param list - List handle to initialize.
 * @param max - Maximum number of elements in list. 0 for unlimited.
 */
void EC_LIST_Init(list_handle_ec_t list, uint32_t max);

/*!
 * @brief Gets the list that contains the given element.
 *
 *
 * @param element - Handle of the element.
 * @retval NULL if element is orphan, Handle of the list the element is inserted into.
 */
list_handle_ec_t EC_LIST_GetList(list_element_handle_ec_t element);

/*!
 * @brief Links element to the head of the list.
 *
 * @param list - Handle of the list.
 * @param element - Handle of the element.
 * @retval EC_kLIST_Full if list is full, EC_kLIST_Ok if insertion was successful.
 */
list_status_ec_e EC_LIST_AddHead(list_handle_ec_t list, list_element_handle_ec_t element);

/*!
 * @brief Links element to the tail of the list.
 *
 * @param list - Handle of the list.
 * @param element - Handle of the element.
 * @retval EC_kLIST_Full if list is full, EC_kLIST_Ok if insertion was successful.
 */
list_status_ec_e EC_LIST_AddTail(list_handle_ec_t list, list_element_handle_ec_t element);

/*!
 * @brief Unlinks element from the head of the list.
 *
 * @param list - Handle of the list.
 *
 * @retval NULL if list is empty, handle of removed element(pointer) if removal was successful.
 */
list_element_handle_ec_t EC_LIST_RemoveHead(void);

/*!
 * @brief Gets head element handle.
 *
 * @param list - Handle of the list.
 *
 * @retval NULL if list is empty, handle of removed element(pointer) if removal was successful.
 */
list_element_handle_ec_t EC_LIST_GetHead(list_handle_ec_t list);

/*!
 * @brief Gets next element handle for given element handle.
 *
 * @param element - Handle of the element.
 *
 * @retval NULL if list is empty, handle of removed element(pointer) if removal was successful.
 */
list_element_handle_ec_t EC_LIST_GetNext(list_element_handle_ec_t element);

/*!
 * @brief Gets previous element handle for given element handle.
 *
 * @param element - Handle of the element.
 *
 * @retval NULL if list is empty, handle of removed element(pointer) if removal was successful.
 */
list_element_handle_ec_t EC_LIST_GetPrev(list_element_handle_ec_t element);

/*!
 * @brief Unlinks an element from its list.
 *
 * @param element - Handle of the element.
 *
 * @retval EC_kLIST_OrphanElement if element is not part of any list.
 * @retval EC_kLIST_Ok if removal was successful.
 */
list_status_ec_e EC_LIST_RemoveElement(list_element_handle_ec_t element);

/*!
 * @brief Links an element in the previous position relative to a given member of a list.
 *
 * @param list - Handle of the list.
 * @param element - Handle of the element.
 * @param newElement - New element to insert before the given member.
 *
 * @retval EC_kLIST_OrphanElement if element is not part of any list.
 * @retval EC_kLIST_Ok if removal was successful.
 */
list_status_ec_e EC_LIST_AddPrevElement(list_element_handle_ec_t element, list_element_handle_ec_t newElement);

/*!
 * @brief Gets the current size of a list.
 *
 * @param list - Handle of the list.
 *
 * @retval Current size of the list.
 */
uint32_t EC_LIST_GetSize(list_handle_ec_t list);

/*!
 * @brief Gets the number of free places in the list.
 *
 * @param list - Handle of the list.
 *
 * @retval Available size of the list.
 */
uint32_t EC_LIST_GetAvailableSize(list_handle_ec_t list);

/* @} */

#if defined(__cplusplus)
}
#endif
/*! @}*/
#endif /*_ETHERCAT_LIST_H_*/
