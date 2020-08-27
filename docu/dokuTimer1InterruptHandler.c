/***********************  TIMER 1 interrupt handler  ****************************/
/* Timer 1 gets started in timer0InterruptHandler(). It is set to 90 ms.        */
/* Because Timer 0 is set to 100 ms, this handler always gets called 10 ms      */
/* before the next timer0InterruptHandler call.                                 */
/* This Timer 1 handler starts a new ADC process which takes 8 ms to convert    */
/* all sensor-array signals with hardware averaging setting enabled. This       */
/* means the adc process is always finished shortly before the next             */
/* timer0InterruptHandler draws the new arrows.                                 */
/********************************************************************************/
void timer1InterruptHandler(void)
{
    // clear the pending interrupt
    timer1IntClear();

    // Start sensorData-array ad-conversion. This starts the first of 16 ADC
    // read bursts. The other 15 bursts will be triggered in adcInterruptHandler().
    startADConversion();
}
