/***********************  UART0 Interrupt handler  ******************************/
/* Communication via RS232 interface with the PC. The PC sends commands. The    */
/* MCU sends Sensor-Array data. More options like sending Stepper-Motor         */
/* telemetery could also be implemented.                                        */
/* 'char UART0receive[8]' contains commands from PC. They are always 8 bytes.   */
/* The type of command depends on the value of the first byte. This is checked  */
/* in the switch case.                                                          */
/********************************************************************************/
void UART0InterruptHandler(void)
{
    // Read the interrupt status of the UART0 peripherie.
    uint32_t interruptStatus = UARTGetIntStatus();

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UART0ClearInterrupt(interruptStatus);

    // Message (8 bytes) from PC received (via UDMA receive)
    if( interruptStatus & DMA_RX_INTERRUPT)
    {
        // pointer to a char string that is defined as UART0receive[8] in uartDMA.c
        char * UART0receive = getUART0RxData();

        // setup DMA for next receive
        prepareNextReceiveDMA();


        // byte 0 contains the type of command from PC:
        switch (UART0receive[0])
        {
        // send sensor-array data via RS-232 to a PC (matlab)
        case '0':
            setup->reqSensorData = true;
            break;

        // set maximum arrow length
        case '1':
            setup->maxArrowLength = getMaxArrowLengthUART();
            writeInfo(MAX_ARROW_LENGTH);
            saveSettingsToEEPROM(&settings, sizeof(Settings));
            break;

        // toggle arrow setting scaling: relative <-> absolute
        case '2':
            setup->relative = setup->relative ? false:true;
            writeInfo(SCALING);
            saveSettingsToEEPROM(&settings, sizeof(Settings));
            break;

        // toggle hardware averaging setting: on <-> off
        case '3':
            setup->adcAVG = setADCHardwareAveraging((UART0receive[1] == '1'));
            writeInfo(HARDW_AVG);
            saveSettingsToEEPROM(&settings, sizeof(Settings));
            break;

        // commands for the stepper-motor are being past forwarded to UART2.
        case '4':
            sendRawCommandToMotor(UART0receive);
            break;

        // todo: additional commands from UART here, if desired.
        case '5':
            break;
        }
    }
}
