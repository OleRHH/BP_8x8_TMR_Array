/***********************  ADC Interrupt handler  ********************************/
/* The hardware can convert up to 16 analog signals to digital when triggered.  */
/* When done with the conversion this function is called and another conversion */
/* is started until all sensor data was read. This function has to be triggered */
/* 16 times to convert all 16*16 = 256 analog values.                           */
/********************************************************************************/
void adcInterruptHandler(void)
{
    static uint16_t step = 0;

    // Interrupt is set when a AD-conversion is finished. It needs to be cleared.
    adcIntClear();

    // advance step count each time an AD-conversion is finished. The first one
    // was already started in Timer0Inthandler. There are 16 steps in total.
    step++;

    // set the analog multiplexer addresses or the next 16 sensor values.
    setMultiplexer(step);

    // store the just captured analog values into buffers to be later processed.
    storeArraySensorData(step - 1);

    // trigger the next AD-conversion (16 in total)
    if(step <= 15)
    {
        startADConversion();
    }

    // after 16 conversions: process analog data and reset values for the next
    // AD-conversion (which gets started during the next Timer0 Interrupt).
    else
    {
        // reset step counter for next use
        step = 0;
        // reset digital multiplexer inputs
        setMultiplexer(step);
    }
}
