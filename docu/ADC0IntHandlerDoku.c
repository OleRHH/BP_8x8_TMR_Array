/***********************  ADC Interrupt handler  ********************************/
/* captures the analog sensor array signals without busy waiting.               */
/* The digitized signals are being processed at the end.                        */
/********************************************************************************/
void ADC1IntHandler(void)
{
    static uint16_t step = 0;

    // Interrupt is set when a AD-conversion is finished. It needs to be cleared.
    adcIntClear();


    // advance step count each time an AD-conversion is finished. The first one
    // was already started in Timer0Inthandler. There are 16 steps in total.
    step++;

    // Port L is used to address the analog multiplexers on the TMR sensor array
    // GPIO_PIN_4 is inverted after half the measures are done. This is because
    // of the sensor-array hardware layout.
    if(step == 8)
    {
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
    }
    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);

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
        // process arrow length and store results to be later drawn on LCD.
        maximumAnalogValue = computeArrows(relative, maxArrowLength);
        // reset step counter for next use
        step = 0;
        // reset digital multiplexer inputs
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_PIN_4);
    }
}
