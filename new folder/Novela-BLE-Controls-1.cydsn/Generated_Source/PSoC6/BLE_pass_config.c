/***************************************************************************//**
* \file CY_BLE_pass_config.c
* \version 2.0
* 
* \brief
*  This file contains the source code of initialization of the config structure for
*  the Phone Alert Status Profile of the BLE Component.
*
********************************************************************************
* \copyright
* Copyright 2017-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#include "ble/cy_ble_pass.h"

#if(CY_BLE_MODE_PROFILE && defined(CY_BLE_PASS))

#ifdef CY_BLE_PASS_SERVER
static const cy_stc_ble_passs_t cy_ble_passs =
{
    0x0025u,    /* Handle of the PASS service */
    {
        
        /* Alert Status characteristic */
        {
            0x002Au, /* Handle of the Alert Status characteristic */ 
            
            /* Array of Descriptors handles */
            {
                0x002Bu, /* Handle of the Client Characteristic Configuration descriptor */ 
            }, 
        }, 
        
        /* Ringer Setting characteristic */
        {
            0x002Du, /* Handle of the Ringer Setting characteristic */ 
            
            /* Array of Descriptors handles */
            {
                0x002Eu, /* Handle of the Client Characteristic Configuration descriptor */ 
            }, 
        }, 
        
        /* Ringer Control Point characteristic */
        {
            0x0030u, /* Handle of the Ringer Control point characteristic */ 
            
            /* Array of Descriptors handles */
            {
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE, 
            }, 
        }, 
    },
};
#endif /* CY_BLE_PASS_SERVER */

/**
* \addtogroup group_globals
* @{
*/

/** The configuration structure for the Phone Alert Status Service. */
cy_stc_ble_pass_config_t cy_ble_passConfig =
{
    /* Service GATT DB handles structure */
    #ifdef CY_BLE_PASS_SERVER
    .passs = &cy_ble_passs,
    #else
    .passs = NULL,
    #endif /* CY_BLE_PASS_SERVER */

    /* An application layer event callback function to receive service events from the BLE Component. */
    .callbackFunc = NULL,
};

/** @} group_globals */

#endif /* (CY_BLE_MODE_PROFILE && defined(CY_BLE_PASS)) */

/* [] END OF FILE */
