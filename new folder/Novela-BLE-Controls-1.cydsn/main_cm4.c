/*******************************************************************************
* File Name: main_cm4.c
* Version: 1.20
* Description:
*   This is source code for the PSoC 6 MCU with BLE:
*           1) PA   control WRITE
*           1) MUX  control WRITE
*           1) OSC  control WRITE
*           1) MISC control WRITE
*******************************************************************************/
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <limits.h>
#include "semphr.h"
#include "timers.h"

#define LED_ON  0UL
#define LED_OFF 1UL
#define HEAP_SIZE_1 8*1024
#define COUNTER_BLOCK_TIME 0

/********************************************************************/
/* Default for four(4) interal registers                            */
/********************************************************************/
#define DEFAULT_PA      0x01 /* Gain:1                              */
#define DEFAULT_MUX     0x00 /* Ch.1                                */
#define DEFAULT_OSC     0x10 /* 100-5KHz                            */
#define DEFAULT_MUX_ON  0x01 /* MUX Disabled                        */
#define DEFAULT_MUX_OFF 0x00 /* MUX Enabled                         */
/********************************************************************/

/*********************************************************************
* Gobal Variables                                                    *
*   val                                                              *
*   PA                                                               *
*   MUX                                                              *
*   OSC                                                              *
*   MISC                                                             *
**********************************************************************/
uint32_t  val     = 0x00;             /*BLE-LED  written into GATT   */
uint32_t  valPA   = DEFAULT_PA;       /*PA       written into GATT   */  
uint32_t  valMUX  = DEFAULT_MUX;      /*MUX      written into GATT   */
uint32_t  valOSC  = DEFAULT_OSC;      /*OSC      written into GATT   */
uint32_t  valMISC = DEFAULT_MUX_OFF;  /*MISC     written into GATT   */
/*********************************************************************/

// This is used to lock and unlock the BLE Task
SemaphoreHandle_t bleSemaphore;

// This is used to lock and unlock the val printf
SemaphoreHandle_t bleSemaphoreval;

// This is used to create a counter handle
TimerHandle_t xtimer;

/********************************************************************
  Define a callback function that will be used by timer instance.  
  The callback function does nothing but printf
*********************************************************************/
 void vTimerCallback( TimerHandle_t xTimer )
 {
 const uint32_t ulMaxExpiryCountBeforeStopping = 10;
 uint32_t ulCount;

    /* The number of times this timer has expired is saved as the
    timer's ID.  Obtain the count. */
    ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );
    printf("%d ", ulCount);

    /* Increment the count, then test to see if the timer has expired */
    ulCount++;

    /* If the timer has expired 10 times then stop it from running. */
    if( ulCount >= ulMaxExpiryCountBeforeStopping ) {
        /* Do not use a block time if calling a timer API function
        from a timer callback function, as doing so could cause a
        deadlock! */
        const char *timerName = pcTimerGetName(xtimer);
        printf("\n(%s) expired at count: %d, \r\n", timerName , ulCount);
        
        /* Dump all parameters at count (10) */
        printf("\r\nval:[%x] PA:[%x] MUX:[%x] OSC:[%x] MISC:[%x]\r\n", val, valPA, valMUX, valOSC, valMISC); 
        
        vTimerSetTimerID( xTimer, ( void * ) 0 );
        xTimerReset( xTimer, 0 );
    }
    else {
       /* Store the incremented count back into the timer's ID field
       so it can be read back again the next time this software timer
       expires. */
       vTimerSetTimerID( xTimer, ( void * ) ulCount );
    
       /* increasing the counter period per each count (x2)
        * This is just for debugging to see printf() slowing down from 0->9.
        *
                                     2xthe last period                    */
       xTimerChangePeriod( xTimer, (100*ulCount), COUNTER_BLOCK_TIME);
    }
 }

/*******************************************************************************
* Function: CreateTimer_1
* Input:    void
* Return:   void
* Description:
*    This function is to create a timer configuration
*******************************************************************************/

void CreateTimer_1(void)
{
printf("Creating one RTOS timer \r\n");

xtimer = xTimerCreate
   ( /* Just a text name, not used by the RTOS
     kernel. */
     "Timer_1",
     /* The timer period in ticks, must be
     greater than 0. Default: 100ms */
     pdMS_TO_TICKS(100),
     /* Timer will auto-reload */
     pdTRUE,
     /* The ID is used to store a count of the
     number of times the timer has expired, which
     is initialised to 0. */
     ( void * ) 0,
     /* Timer calls the same callback when it expires. */
     vTimerCallback
   );
                
if( xtimer == NULL )
     {
         printf("The timer was not created\r\n");
     }
else{
 /* Start the timer.  No block time is specified, and
 even if one was it would be ignored because the RTOS
 scheduler has not yet been started. */
 if( xTimerStart( xtimer, 0 ) != pdPASS )
 {
     printf("The timer could not be set into the Active state\r\n");
 }
}
} 

/*******************************************************************************
* Function: writeDisplayEventHandler
* Input:    void
* Return:   void
* Description:
*    This function is the BLE write to the server function - printf
*******************************************************************************/

void writeDisplayEventHandler(void)
{
    printf("'Semaphored' HEX Value received for the GATT Client: %x \r\n", val);
    xSemaphoreGive(bleSemaphoreval);
} 

/*******************************************************************************
* Function: GPIOInit_1
* Input:    void
* Return:   void
* Description:
*    This function is the golbal GPIO Init
*******************************************************************************/

void GPIOInit_1(void)
{
    /* PA */
    Cy_GPIO_Pin_Init(PA0_0_PORT, PA0_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(PA1_0_PORT, PA1_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(PA2_0_PORT, PA2_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(PA3_0_PORT, PA3_0_NUM, 0UL);
    
    /* MUX */
    Cy_GPIO_Pin_Init(MUX0_0_PORT, MUX0_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(MUX1_0_PORT, MUX1_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(MUX2_0_PORT, MUX2_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(MUX3_0_PORT, MUX3_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(MUX_EN_0_PORT, MUX_EN_0_NUM, 0UL);
    
    /* OSC */
    Cy_GPIO_Pin_Init(OSC0_0_PORT, OSC0_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(OSC1_0_PORT, OSC1_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(OSC2_0_PORT, OSC2_0_NUM, 0UL);
    
    /* MISC */
    Cy_GPIO_Pin_Init(MISC0_0_PORT, MISC0_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(MISC1_0_PORT, MISC1_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(MISC2_0_PORT, MISC2_0_NUM, 0UL);
    Cy_GPIO_Pin_Init(MISC3_0_PORT, MISC3_0_NUM, 0UL);
    
    /* User SW1 */
    Cy_GPIO_Pin_Init(SW1_0_PORT, SW1_0_NUM, 0UL);       
}   
    // Cy_GPIO_Pin_Init(RED_0_PORT, RED_0_NUM, 0UL);
    // Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);


/*******************************************************************************
* Function: writeDisplayPA
* Input:    void
* Return:   void
* Description:
*    This function is the BLE write to PA GATT register
*******************************************************************************/
void writeDisplayPA(void)
{   
    printf("PA GATT Client: %x \r\n", valPA);
    
    switch (valPA)
	{
        case 0x00:        
            printf("Gain:0 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 0UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;
        
        case 0x01:        
            printf("Gain:1 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 1UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 0UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;

        case 0x02:        
            printf("Gain:2 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 1UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;
        
        case 0x03:        
            printf("Gain:4 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 1UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 1UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;

        case 0x04:        
            printf("Gain:8 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 0UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 1UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;
        
        case 0x05:        
            printf("Gain:16 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 1UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 0UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 1UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;

        case 0x06:        
            printf("Gain:32 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 1UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 1UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;
        
        case 0x07:        
            printf("Gain:64 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 1UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 1UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 1UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 0UL);
            break;
            
        case 0x08:        
            printf("Gain128 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 0UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 1UL);
            break;
        
        case 0x09:        
            printf("Gain:256 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 1UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 0UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 1UL);
            break;

        case 0x0A:        
            printf("Gain:512 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 1UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 1UL);
            break;
        
        case 0x0B:        
            printf("Gain:1024 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 1UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 1UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 0UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 1UL);
            break;

        case 0x0C:        
            printf("Gain:2048 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 0UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 0UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 1UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 1UL);
            break;
            
        default:
            printf("Gain:4096 \r\n");
            Cy_GPIO_Write(PA0_0_PORT, PA0_0_NUM, 1UL);
            Cy_GPIO_Write(PA1_0_PORT, PA1_0_NUM, 1UL);
            Cy_GPIO_Write(PA2_0_PORT, PA2_0_NUM, 1UL);
            Cy_GPIO_Write(PA3_0_PORT, PA3_0_NUM, 1UL);
            break;            
    }
}

/*******************************************************************************
* Function: writeDisplayMUX
* Input:    void
* Return:   void
* Description:
*    This function is the BLE write to MUX GATT register
*******************************************************************************/
void writeDisplayMUX(void)
{ 
    printf("MUX GATT Client: %x \r\n", valMUX);
    
switch (valMUX)
	{
        case 0x00:        
            printf("ON Switch:0 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL);          
            break;
        
        case 0x01:        
            printf("ON Switch:1 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL); 
            break;

        case 0x02:        
            printf("ON Switch:2 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL); 
            break;
        
        case 0x03:        
            printf("ON Switch:3 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL); 
            break;

        case 0x04:        
            printf("ON Switch:4 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL); 
            break;
        
        case 0x05:        
            printf("ON Switch:5 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL); 
            break;

        case 0x06:        
            printf("ON Switch:6 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL); 
            break;
        
        case 0x07:        
            printf("ON Switch:7 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 0UL); 
            break;
            
        case 0x08:        
            printf("ON Switch:8 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;
        
        case 0x09:        
            printf("ON Switch:9 \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;

        case 0x0A:        
            printf("ON Switch:A \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;
        
        case 0x0B:        
            printf("ON Switch:B \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 0UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;

        case 0x0C:        
            printf("ON Switch:C \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;
            
        case 0x0D:        
            printf("ON Switch:D \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 0UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;
            
        case 0x0E:        
            printf("ON Switch:E \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 0UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;
            
        default:
            printf("ON Switch:F \r\n");
            Cy_GPIO_Write(MUX0_0_PORT, MUX0_0_NUM, 1UL);
            Cy_GPIO_Write(MUX1_0_PORT, MUX1_0_NUM, 1UL);
            Cy_GPIO_Write(MUX2_0_PORT, MUX2_0_NUM, 1UL);            
            Cy_GPIO_Write(MUX3_0_PORT, MUX3_0_NUM, 1UL); 
            break;            
    }
}

/*******************************************************************************
* Function: writeDisplayOSC
* Input:    void
* Return:   void
* Description:
*    This function is the BLE write to OSC GATT register
*******************************************************************************/
void writeDisplayOSC(void)
{   
    printf("OSC Client: %x \r\n", valOSC);
    /* Empty for now */
    /* Missing Case switch */
}

/*******************************************************************************
* Function: writeDisplayMISC
* Input:    void
* Return:   void
* Description:
*    This function is the BLE write to MISC GATT register
*******************************************************************************/

void writeDisplayMISC(void)
{   
    printf("MISC GATT Client: %x \r\n", valMISC);
    /* Empty for now */
    /* Missing Case switch */
}

/*******************************************************************************
* Function: genericEventHandler
* Input:    CY_BLE Event Handler event and eventParameter
* Return:   void
* Description:
*    This function is the BLE event handler function. It's called by the BLE
*    stack when and event occures
*******************************************************************************/

void genericEventHandler(uint32_t event, void *eventParameter)
{
    cy_stc_ble_gatts_write_cmd_req_param_t *writeReqParameter;
    uint8 i;
    
    switch (event)
	{
        /* There are some events generated by the BLE component
        *  that are not required for this code example.
        *  Don't edit them as they might be relevant later on
        */
        
        /**********************************************************
        *                       General Events
        ***********************************************************/
		/* This event is received when the BLE stack is started */
        case CY_BLE_EVT_STACK_ON:       
            printf("CY_BLE_EVT_STACK_ON: \r\n"); 
            
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            PWM_BLINK_Start();
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);   
            Cy_TCPWM_TriggerReloadOrIndex(PWM_DIM_HW,PWM_DIM_CNT_NUM);
            Cy_TCPWM_PWM_Disable(PWM_DIM_HW,PWM_DIM_CNT_NUM); 
            printf("Start Advertising: \r\n");
            printf("RED_LED is blinking till paired: \r\n");
            break;
            
        /* This event is received when there is a timeout */
        case CY_BLE_EVT_TIMEOUT:
            printf("CY_BLE_EVT_TIMEOUT \r\n"); 
            break;
            
        /* This event indicates that some internal HW error has occurred */    
		case CY_BLE_EVT_HARDWARE_ERROR: 
            printf("CY_BLE_EVT_HARDWARE_ERROR \r\n");
			break;
            
        /*  This event will be triggered by host stack if BLE stack is busy or 
         *  not busy. Parameter corresponding to this event will be the state 
    	 *  of BLE stack.
         *  BLE stack busy = CYBLE_STACK_STATE_BUSY,
    	 *  BLE stack not busy = CYBLE_STACK_STATE_FREE 
         */
    	case CY_BLE_EVT_STACK_BUSY_STATUS:
            printf("CY_BLE_EVT_STACK_BUSY_STATUS: %x\r\n", *(uint8 *)eventParameter);
            break;
            
        /* This event indicates set device address command completed */
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
            printf("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE \r\n");
            break;
            
        /* This event indicates get device address command completed
           successfully */
        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
            printf("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE: ");
            for(i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
            {
                /* Public Bluetooth device address of size 6 bytes in little endian format. */
                printf("%2.2x", ((cy_stc_ble_bd_addrs_t *)((cy_stc_ble_events_param_generic_t *)eventParameter)->eventParams)->publicBdAddr[i-1]);
                
                /* Private Bluetooth device address of size 6 bytes in little endian format. */
                printf("%2.2x", ((cy_stc_ble_bd_addrs_t *)((cy_stc_ble_events_param_generic_t *)eventParameter)->eventParams)->privateBdAddr[i-1]);
            }
            printf("\r\n");          
            break;
         
        /* This event indicates set Tx Power command completed */
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
            printf("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
            break;
                        
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
       
        /* This event indicates peripheral device has started/stopped
           advertising */
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            printf("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP: ");
            break;
            
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device */
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            printf("CY_BLE_EVT_GAP_DEVICE_CONNECTED \r\n");
            break;
        
        /* This event is generated at the GAP Central and the peripheral end 
           after connection parameter update is requested from the host to 
           the controller */
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            printf("CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE \r\n");
            break;
                     
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
            
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device */
       case CY_BLE_EVT_GATT_CONNECT_IND:
            printf("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x \r\n", 
                        (*(cy_stc_ble_conn_handle_t *)eventParameter).attId, 
                        (*(cy_stc_ble_conn_handle_t *)eventParameter).bdHandle);
             printf("Active Connection Instance: %x \r\n", 
                        (*(cy_stc_ble_conn_handle_t *)eventParameter).attId);
            
            /* Stop blinking LED: Client->Server connection is 'ON' */
            Cy_TCPWM_TriggerReloadOrIndex(PWM_BLINK_HW,PWM_BLINK_CNT_NUM);
            Cy_TCPWM_PWM_Disable(PWM_BLINK_HW,PWM_BLINK_CNT_NUM);
            
            /* Start dimming LED with default compare */ 
            PWM_DIM_Start();        
            break;
            
        /* This event is generated at the GAP Peripheral end after 
           disconnection */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            printf("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
            break;
            
        /*********************************************************************************
         * This event is generated when the phone side send a write request
         * Probe which GATT service/characteristic is called
         *********************************************************************************/
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            
            printf("CY_BLE_EVT_GATTS_WRITE_REQ\r\n");
            writeReqParameter = (cy_stc_ble_gatts_write_cmd_req_param_t *)eventParameter; 
            
            if(CY_BLE_LED_GREEN_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {
                /*************************************************************************/
                /*        Read the handle and value
                 *
                 *        Attribute Value    
                 *           cy_stc_ble_gatt_value_t              value;
                 *
                 *        Attribute Handle of GATT DB
                 *           cy_ble_gatt_db_attr_handle_t         attrHandle;               
                *************************************************************************/
                if( xSemaphoreTake( bleSemaphoreval, (TickType_t ) 10 ) == pdTRUE)
                {
                    val = writeReqParameter->handleValPair.value.val[0];
                    if(val>100) val = 100;
                    
                    xSemaphoreGive(bleSemaphoreval);               
                }
                else
                {
                    /* We could not obtain the semaphore and can therefore not access
                    the shared resource safely. */   
                    printf("bleSemaphoreval error\r\n");
                }               
                
                Cy_TCPWM_PWM_SetCompare0(PWM_DIM_HW,PWM_DIM_CNT_NUM, val);
                
            /* printf function call */
            xSemaphoreTake( bleSemaphoreval, (TickType_t ) 10 );
            writeDisplayEventHandler();
            }
            
            //Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);
            /* printf function call */
            //xSemaphoreTake( bleSemaphoreval, (TickType_t ) 10 );
            //writeDisplayEventHandler();
            
            /*************************************************************************
             *        WRITE to the 'PA' Characteristic
             *
             *        Do nothing, just printf() for now                
             *************************************************************************/
            if(CY_BLE_LED_PA_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {
                valPA = writeReqParameter->handleValPair.value.val[0];
                writeDisplayPA(); /* Actual function for future use */    
            }
            
            /*************************************************************************
             *        WRITE to the 'MUX' Characteristic
             *
             *        Do nothing, just printf() for now                
             *************************************************************************/
            if(CY_BLE_LED_MUX_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {
                valMUX = writeReqParameter->handleValPair.value.val[0];
                writeDisplayMUX(); /* Actual function for future use */  
            }
            
            /*************************************************************************
             *        WRITE to the 'OSC' Characteristic
             *
             *        Do nothing, just printf() for now                
             *************************************************************************/
            if(CY_BLE_LED_OSC_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {
                valOSC = writeReqParameter->handleValPair.value.val[0];
                writeDisplayOSC(); /* Actual function for future use */  
            }
            
            /*************************************************************************
             *        WRITE to the 'MISC' Characteristic
             *
             *        Do nothing, just printf() for now                
             *************************************************************************/
            if(CY_BLE_LED_MISC_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {
                valMISC = writeReqParameter->handleValPair.value.val[0];
                writeDisplayMISC(); /* Actual function for future use */   
            }  
            
            /**************************************************************************
            * Since this is the GATT service with WRITE + response..the end of that call
            ***************************************************************************/
            Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);     
            break;
         
        /* This event is triggered when 'GATT MTU Exchange Request' 
           received from GATT client device */
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
            printf("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ \r\n");
            break;
        
        /* This event is triggered when a read received from GATT 
           client device */
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            printf("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ \r\n");
            break;

        /**********************************************************
        *                       Other Events
        ***********************************************************/
        default:
            printf("Other event: %lx \r\n", (unsigned long) event);
			break;
	}
}

/*****************************************************************************\
 * Function:    bleInterruptNotify
 * Input:       void (it is called inside of the ISR)
 * Returns:     void
 * Description: 
 *   This is called back in the BLE ISR when an event has occured and needs to
 *   be processed.  It will then set/give the sempahore to tell the BLE task to
 *   process events.
\*****************************************************************************/
void bleInterruptNotify()
{
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(bleSemaphore, &xHigherPriorityTaskWoken); 
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/*****************************************************************************\
 * Function:    bleTask
 * Input:       A FreeRTOS Task - void * that is unused
 * Returns:     void
 * Description: 
 *  This task starts the BLE stack... and processes events when the bleSempahore
 *  is set by the ISR.
\*****************************************************************************/
void bleTask(void *arg)
{
    (void)arg;
    
    printf("BLE Task Started\r\n");
    
    /******************************************************************
    * Create a counting semaphore that has a maximum count of 10 and an
    initial count of 0. 
    *******************************************************************/
    /*                                      max     ,intial           */
    bleSemaphore = xSemaphoreCreateCounting(UINT_MAX,0);
    
    /* Simple semaphore for the 'val' printf */
    bleSemaphoreval = xSemaphoreCreateMutex();
    
    /* The semaphore was created successfully. */
    if(bleSemaphore != NULL) {
        printf("'bleSemaphore' was created successfully \r\n");      
    }
    
    /* The val printf semaphore was created successfully. */
    if(bleSemaphoreval != NULL) {
        printf("'bleSemaphoreval' was created successfully \r\n");      
    }
    
    
    Cy_BLE_Start(genericEventHandler);
    
    while (Cy_BLE_GetState() != CY_BLE_STATE_ON)
    {
        Cy_BLE_ProcessEvents();    
    }
    
    Cy_BLE_RegisterAppHostCallback(bleInterruptNotify);
    
    for(;;)
    {
        xSemaphoreTake(bleSemaphore, portMAX_DELAY);
        Cy_BLE_ProcessEvents();   
    }   
}

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */  
    
    UART_1_Start();
    setvbuf( stdin, NULL, _IONBF, 0 );
    setvbuf( stdout, NULL, _IONBF, 0 );
    printf("System Started Succesfully.\r\n");
    
    /* Start two PWMs */
    PWM_DIM_Start();
    PWM_BLINK_Start();
    
    /**************************
    * GPIO Initializartion Call
    * PA, MUX, OSC, MISC, SW1
    ***************************/
    GPIOInit_1();
    
    /* Create one counter and call vTimerCallback */ 
    CreateTimer_1();
    
    xTaskCreate(bleTask,"bleTask",HEAP_SIZE_1,0,2,0);
    
    vTaskStartScheduler();
    
    for(;;) 
    {   
	}   
} 

/* [] END OF FILE */
