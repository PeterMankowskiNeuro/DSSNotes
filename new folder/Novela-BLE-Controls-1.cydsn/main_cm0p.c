/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.20
*
* Description:
*   This is source code for the PSoC 6 MCU with BLE Find Me code example.
*
* Note:
*
* Owners:
*   peter@novelaneuro.com

********************************************************************************
* Copyright 2019, Novela Neuro.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

int main(void)
{   
    __enable_irq(); /* Enable global interrupts. */
    
    Cy_BLE_Start(0);
    
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR);
   
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    for(;;)
    {
        Cy_BLE_ProcessEvents();        
    }
}

/* [] END OF FILE */
