/***********************  TIMER 0 interrupt handler  ****************************/
/* Timer 0 is set to call this handler every 100 ms. This handler calls the     */
/* drawDisplay() function to refresh the LCD content. It also starts Timer 1    */
/* which is programmed to call the timer1InterruptHandler() 85 ms later.        */
/********************************************************************************/
void timer0InterruptHandler(void)
{
    // clear the pending interrupt
    timer0IntClear();

    // start timer1 to delayed start a new ad-conversion process
    startTimer1();

    // send sensor data to pc if requested.
    if(setup->reqSensorData == true)
    {
        setup->reqSensorData = false;
        sendUARTDMA(sensorData->resultsForUARTSend);
    }
    // compute the new arrow length from last measurement before drawing LCD
    computeArrows(setup->relative, setup->maxArrowLength);

    // Draw the arrows and button states to the LC-Display. This function also
    // calculates the new arrow lines. This is the most time consuming part of
    // the project.
    drawDisplay((void *)&sensorData->arrows);
}
