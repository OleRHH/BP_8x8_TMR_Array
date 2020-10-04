void main(void)
{
    [...]

    while(1)
    {
        wait = true;
        toggleLED(1);
        // start first ADC sequence
        startADConversion();

        // 'wait' gets set back in ADC interupt handler
        // after all measurements are complete
        while(wait == true);

        onLED(2);
        computeArrows(setup->relative, setup->maxArrowLength);
        offLED(2);
    }
}

void adcInterruptHandler(void)
{
    onLED(2);

    [...]
    [siehe Abschnitt ADC Interrupt]
    [...]

    offLED(2);
}
