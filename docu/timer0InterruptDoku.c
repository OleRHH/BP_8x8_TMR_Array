/***********************  TIMER 0 interrupt handler  ****************************/
/* Periodically measures the sensor Array values and draw them to the display.  */
/* Sends commands to the stepper-motor and calls setup-menu                     */
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

    // Reads touch screen status. Returns command information and (if so)
    // the command itself as a pointer.
    commandFromTouch = readTouchscreen(command);

    // Commands for the motor could be: start, stop, left, right and so on.
    // It can originate from the touch screen or UART0.
    switch(commandFromTouch)
    {
        noNewCommand:           break;
        enterSettings:          settings(command); break;
        newCommandForMotor:     sendCommandToMotor(command, 9); break;
    }

    // Start sensor-array ad-conversion. This starts the first of 16 ADC
    // read bursts. The other 15 bursts will be triggered in ADC1IntHandler().
    startADConversion();
}
