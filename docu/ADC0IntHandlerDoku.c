/***********************  ADC Interrupt handler  ********************************/
/* capture the analog sensor array signals without busy waiting until           */
/* ad-conversion is complete.                                                   */
/********************************************************************************/
void ADC0IntHandler(void)
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

    // store the previously captured analog values into local buffers
    ReadArray(step-1);

    // trigger the next AD-conversion (16 in total)
    if(step <= 15)
    {
        startAdcConversion();
    }

    // after 16 conversions process analog data and reset values for the next
    // AD-conversion (starts with next Timer0 Interrupt).
    else
    {
        maximumAnalogValue = computeArrows(relative, maxArrowLength);
        step = 0;
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_PIN_4);
    }
}
