/*
 * uartDMA.c
 *
 *  Created on: 10.05.2020
 */
/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <inc/hw_memmap.h>          // needed for UART0_BASE
#include "inc/hw_uart.h"            // needed for UART_O_DR
#include <driverlib/pin_map.h>      // GPIOPinConfigure
#include <driverlib/interrupt.h>
#include <driverlib/udma.h>

#include <uartDMA.h>

/********************************************************************************/
// The control table used by the uDMA controller. This table must be aligned
// to a 1024 byte boundary.
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];

// The transmit buffer used for the UART transfer.
static char UART0receive[8];


/********************************************************************************/
void UART0ClearInterrupt(unsigned int interruptStatus)
{
    UARTIntClear(UART0_BASE, interruptStatus);
}


/********************************************************************************/
unsigned int UARTGetIntStatus(void)
{
    return UARTIntStatus(UART0_BASE, 1);
}


/********************************************************************************/
char * getUART0RxData(void)
{
    return UART0receive;
}


/********************************************************************************/
bool getRelativeAbsoluteSetting(void)
{
    bool setting = false;

    if(UART0receive[1] == '1')
    {
        setting = true;
    }

    return setting;
}


/********************************************************************************/
uint16_t getMaxArrowLength(void)
{
    return ( UART0receive[4]<<24 | UART0receive[5]<<16
           | UART0receive[6]<<8 | UART0receive[7] );
}


/********************************************************************************/
void prepareNextReceiveDMA(void)
{
    // prepare uDMA for the next UART0receive (8 bytes)
    uDMAChannelTransferSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
                           UDMA_MODE_BASIC,
                               (void *)(UART0_BASE + UART_O_DR),
                               UART0receive, sizeof(UART0receive));
    uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
}


/********************************************************************************/
void sendUARTDMA(char * resultsForUARTSend)
{
    // Set up the transfer parameters for the uDMA UART TX channel.  This will
    // configure the transfer source and destination and the transfer size.
    // Basic mode is used because the peripheral is making the uDMA transfer
    // request.  The source is the DiffResults array and the destination is the UART
    // data register.
    uDMAChannelTransferSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC, resultsForUARTSend,
                               (void *)(UART0_BASE + UART_O_DR),
                               256);
    // The uDMA TX channel must be enabled to send a data burst.
    // It starts immediately because the Tx FIFO is empty (or should be)
    uDMAChannelEnable(UDMA_CHANNEL_UART0TX);
}


/********************************************************************************/
void sendCommandToMotor(char * data, uint16_t size)
{
    // the data structure to the stepper-motor is always 9 bytes.
    // last byte is a checksum. See datasheet Trinamic TMCM-1141 for details.

    uint16_t i;
    uint16_t checksum = 1;
    static char stepperMotorCommand[9] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    for(i = 1; i < 8; i++)
    {
        stepperMotorCommand[i] = data[i];
        checksum += data[i];
    }
    stepperMotorCommand[8] = checksum;

    // send command to stepper-motor via RS-485 (UART2)
    for(i = 0; i < 9; i++)
    {
        UARTCharPutNonBlocking(UART2_BASE, stepperMotorCommand[i]);
    }
}


/***********************  UART2 Interrupt handler  ******************************/
/* receive telemetry data from stepper-motor via RS485 */
void UART2IntHandler(void)
{
    uint32_t positionData;

    uint32_t UIstatus = UARTIntStatus(UART2_BASE, true);    // Get the interrupt status.
    UARTIntClear(UART2_BASE, UIstatus);

    if( UIstatus & UART_INT_RX)
    {
        positionData = receiveDataFromMotor();
    }
}


/********************************************************************************/
uint32_t receiveDataFromMotor(void)
{
    int i = 0;
    uint32_t positionData;
    char UART0receive[10];

    while(UARTCharsAvail(UART2_BASE) && i < 10)
    {
        UART0receive[i++] = UARTCharGetNonBlocking(UART2_BASE);
    }

    // UART0receive position data from stepper-motor (absolute)
    if(UART0receive[2] == 100 && UART0receive[3] == 6)
    {
        positionData = UART0receive[4];
        positionData <<= 8;
        positionData |= UART0receive[5];
        positionData <<= 8;
        positionData |= UART0receive[6];
        positionData <<= 8;
        positionData |= UART0receive[7];
    }
    else
    {
        positionData = 0;
    }
    return positionData;
}


/********************************************************************************/
void configureUartUDMA(void)
{
    // Enable the uDMA controller at the system level. Wait until it is ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)));

    // Enable the uDMA controller error interrupt. This interrupt will occur
    // if there is a bus error during a transfer.
    //    IntEnable(INT_UDMAERR);

    // Enable the uDMA controller.
    uDMAEnable();

    // Point at the control table to use for channel control structures.
    uDMAControlBaseSet(pui8ControlTable);

    // uDMA Rx:
    uDMAChannelAssign(UDMA_CH8_UART0RX);
    // Put the attributes in a known state for the uDMA UART0RX channel.  These
    // should already be disabled by default.
    uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0RX, UDMA_ATTR_ALL);

    // Configure the control parameters for the primary control structure for
    // the UART RX channel. The transfer data size is 8 bits, the
    // source address does not increment since it will be reading from a
    // register. The destination address increment is byte 8-bit bytes.  The
    // arbitration size is set to 4 to match the RX FIFO trigger threshold.
    // The uDMA controller will use a 4 byte burst transfer if possible.  This
    // will be somewhat more efficient that single byte transfers.
    uDMAChannelControlSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
                            UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 |
                            UDMA_ARB_4);

    // Set up the transfer parameters for the UART RX primary control
    // structure.  The mode is set to basic, the transfer source is the
    // UART data register, and the destination is the receive buffer. The
    // transfer size is set to match the size of the receive buffer.
    uDMAChannelTransferSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
                           UDMA_MODE_BASIC, (void *)(UART0_BASE + UART_O_DR),
                           (void *)UART0receive, sizeof(UART0receive));

    // uDMA Tx:
    uDMAChannelAssign(UDMA_CH9_UART0TX);

    // Put the attributes in a known state for the uDMA UART0TX channel. These
    // should already be disabled by default.
    uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0TX, UDMA_ATTR_ALL);


    // Set the USEBURST attribute for the uDMA UART TX channel.  This will
    // force the controller to always use a burst when transferring data from
    // the TX buffer to the UART.  This is somewhat more efficient bus usage
    // than the default which allows single or burst transfers.
    uDMAChannelAttributeEnable(UDMA_CHANNEL_UART0TX, UDMA_ATTR_USEBURST);

    // Configure the control parameters for the UART TX.  The uDMA UART TX
    // channel is used to transfer a block of data from a buffer to the UART.
    // The data size is 8 bits.  The source address increment is 8-bit bytes
    // since the data is coming from a buffer.  The destination increment is
    // none since the data is to be written to the UART data register.  The
    // arbitration size is set to 4, which matches the UART TX FIFO trigger
    // threshold.
    uDMAChannelControlSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                              UDMA_SIZE_8 | UDMA_SRC_INC_8 |
                              UDMA_DST_INC_NONE |
                              UDMA_ARB_8);

    // Now both the uDMA UART TX and RX channels are primed to start a
    // transfer.  As soon as the channels are enabled, the peripheral will
    // issue a transfer request and the data transfers will begin. Also it
    // will receive bytes
    uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
}


/********************************************************************************/
/* UART0 is used to transmit Array Data (256 byes) via RS232 and to receive     */
/* control commands.                                                            */
/********************************************************************************/
void ConfigureUART0(uint32_t SysClock)
{
    // Enable the GPIO Peripheral used by the UART and wait until it is ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)));

    // Enable the UART peripheral and wait until it is ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)));

    // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART communication parameters.
    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200,
                            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE);

    // TX trigger thresholds to 7 (14 bytes) and RX trigger thresholds to 4 (8 bytes).This will
    // be used by the uDMA controller to signal when more data should be transferred.
    UARTFIFOEnable(UART0_BASE);
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX7_8, UART_FIFO_RX4_8);

    // Enable the uDMA interface for both TX and RX channels.
    UARTDMAEnable(UART0_BASE, UART_DMA_RX | UART_DMA_TX);

    // Enable the UART DMA TX/RX interrupts.
    UARTIntRegister(UART0_BASE, UART0InterruptHandler);
//  UARTIntEnable(UART0_BASE, UART_INT_RX);
    UARTIntEnable(UART0_BASE, UART_INT_DMARX);

    // Enable the UART peripheral interrupts.
    IntEnable(INT_UART0);
}



/********************************************************************************/
/* UART2 is used for communication with the stepper motor via RS485             */
/********************************************************************************/
void ConfigureUART2(uint32_t SysClock)
{
    // Enable the GPIO Peripheral used by the UART
    // Rx: PA6  Tx: PA7
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    //Enable UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    // Configure GPIO Pins for UART mode
    GPIOPinConfigure(GPIO_PA6_U2RX);
    GPIOPinConfigure(GPIO_PA7_U2TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE,GPIO_PIN_6 | GPIO_PIN_7);


    UARTConfigSetExpClk(UART2_BASE, SysClock, 9600, //115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    UARTIntRegister(UART2_BASE, UART2IntHandler);
    UARTFIFOEnable(UART2_BASE);
    UARTFIFOLevelSet(UART2_BASE, UART_FIFO_TX1_8, UART_FIFO_RX4_8);
    IntEnable(INT_UART2);
    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT); // | UART_INT_TX);
}





