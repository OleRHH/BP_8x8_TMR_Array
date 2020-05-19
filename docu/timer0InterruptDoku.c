/***********************  TIMER 0 interrupt handler  ****************************/
/* Periodically measures the sensor Array values and draw them to the display.  */
/* Send commands to the stepper-motor.                                          */
/********************************************************************************/
void Timer0IntHandler(void)
{
    // clear the pending timer 0 interrupt
    timer0IntClear();

    // Draw the arrows and button states to the LC-Display. This function also
    // calculates the new arrow lines. This is the most time consuming part of
    // the program.
    drawDisplay(backColor);

    // Read touch screen informations. Returns true if new command is available.
    newCommandForMotor = readTouchscreen(motorCommand);

    // Send command to stepper-motor.
    // Commands for the motor could be: start, stop, left, right and so on.
    // It can originate from the touch screen or UART0.
    if(newCommandForMotor == true)
    {
        sendCommandToMotor(motorCommand, 9);
        newCommandForMotor = false;
    }

    // Start sensor-array ad-conversion. This starts the first of 16 ADC
    // read bursts. The other 15 bursts will be triggered in ADC0IntHandler().
    startAdcConversion();
}
