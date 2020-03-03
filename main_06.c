#include <main.h>


/*********************  # global variables #   ******************/
uint32_t SysClock;


int main(void)
{
	SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
	                | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

	//Initialize the UART, GPIO, ADC and Timer
	ConfigureUART(SysClock);
	ConfigureGPIO();
	ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD();
    COLOR background;

    write_screen_color(background);

    IntMasterEnable();

	while(1);
}
