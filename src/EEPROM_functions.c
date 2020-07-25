#include <EEPROM_functions.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_eeprom.h"
#include "inc/hw_flash.h"
#include "inc/hw_ints.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/flash.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
//#include "driverlib/eeprom.h"


/********************************************************************************/
//#define INIT


/********************************************************************************/
uint32_t EEPROMProgram1(uint32_t *, uint32_t, uint32_t);
void EEPROMRead1(uint32_t *, uint32_t, uint32_t);
void _EEPROMSectorMaskSet(uint32_t);
void _EEPROMSectorMaskClear(void);
void EEPROMInit(void);


/********************************************************************************/
Settings setting;


/********************************************************************************/
Settings * loadSettings(void)
{
    Settings * settings = &setting;

    settings->relative = true;
    settings->coloredArrows = true;
    settings->maxArrowLength = 10;
    settings->adcAVG = true;
    settings->backgroundColor = 0x00FF0FFF;
    settings->motorIncrement = 100;

    EEPROMProgram1((uint32_t *)settings, 0x00, sizeof(settings));

#ifdef INIT
        settings->relative = true;
        settings->coloredArrows = true;
        settings->maxArrowLength = 100;
        settings->adcAVG = true;
        settings->backgroundColor = 0x00FFFFFF;
        settings->motorIncrement = 100;
        EEPROMProgram1((uint32_t *)settings, 0x00, sizeof(settings));
#else
        EEPROMRead1((uint32_t *)settings, 0x00, sizeof(settings));
#endif

    return settings;
}


/********************************************************************************/
void saveSettings(Settings * settings)
{
    EEPROMProgram1((uint32_t *)settings, 0x00, sizeof(settings));
}



// Reads data from the EEPROM.
/********************************************************************************/
#define OFFSET_FROM_ADDR(x) (((x) >> 2) & 0x0F)
#define EEPROMBlockFromAddr(ui32Addr) ((ui32Addr) >> 6)

void EEPROMRead1(uint32_t *pui32Data, uint32_t ui32Address, uint32_t ui32Count)
{

    //
    // Set the block and offset appropriately to read the first word.
    //
    HWREG(EEPROM_EEBLOCK) = EEPROMBlockFromAddr(ui32Address);
    HWREG(EEPROM_EEOFFSET) = OFFSET_FROM_ADDR(ui32Address);

    //
    // Convert the byte count to a word count.
    //
    ui32Count /= 4;

    //
    // Read each word in turn.
    //
    while(ui32Count)
    {
        //
        // Read the next word through the autoincrementing register.
        //
        *pui32Data = HWREG(EEPROM_EERDWRINC);

        //
        // Move on to the next word.
        //
        pui32Data++;
        ui32Count--;

        //
        // Do we need to move to the next block?  This is the case if the
        // offset register has just wrapped back to 0.  Note that we only
        // write the block register if we have more data to read.  If this
        // register is written, the hardware expects a read or write operation
        // next.  If a mass erase is requested instead, the mass erase will
        // fail.
        //
        if(ui32Count && (HWREG(EEPROM_EEOFFSET) == 0))
        {
            HWREG(EEPROM_EEBLOCK) += 1;
        }
    }
}


// Writes data to the EEPROM.
/********************************************************************************/
uint32_t
EEPROMProgram1(uint32_t *pui32Data, uint32_t ui32Address, uint32_t ui32Count)
{
    uint32_t ui32Status;

    //
    // Check parameters in a debug build.
    //
    ASSERT(pui32Data);
    ASSERT(ui32Address < SIZE_FROM_EESIZE(HWREG(EEPROM_EESIZE)));
    ASSERT((ui32Address + ui32Count) <=
           SIZE_FROM_EESIZE(HWREG(EEPROM_EESIZE)));
    ASSERT((ui32Address & 3) == 0);
    ASSERT((ui32Count & 3) == 0);

    //
    // Make sure the EEPROM is idle before we start.
    //
    do
    {
        //
        // Read the status.
        //
        ui32Status = HWREG(EEPROM_EEDONE);
    }
    while(ui32Status & EEPROM_EEDONE_WORKING);

    //
    // Set the block and offset appropriately to program the first word.
    //
    HWREG(EEPROM_EEBLOCK) = EEPROMBlockFromAddr(ui32Address);
    HWREG(EEPROM_EEOFFSET) = OFFSET_FROM_ADDR(ui32Address);

    //
    // Convert the byte count to a word count.
    //
    ui32Count /= 4;

    //
    // Write each word in turn.
    //
    while(ui32Count)
    {
        //
        // This is a workaround for a silicon problem on Blizzard rev A.  We
        // need to do this before every word write to ensure that we don't
        // have problems in multi-word writes that span multiple flash sectors.
        //
        if(CLASS_IS_TM4C123 && REVISION_IS_A0)
        {
            _EEPROMSectorMaskSet(ui32Address);
        }

        //
        // Write the next word through the autoincrementing register.
        //
        HWREG(EEPROM_EERDWRINC) = *pui32Data;

        //
        // Wait a few cycles.  In some cases, the WRBUSY bit is not set
        // immediately and this prevents us from dropping through the polling
        // loop before the bit is set.
        //
        SysCtlDelay(10);

        //
        // Wait for the write to complete.
        //
        do
        {
            //
            // Read the status.
            //
            ui32Status = HWREG(EEPROM_EEDONE);
        }
        while(ui32Status & EEPROM_EEDONE_WORKING);

        //
        // Make sure we completed the write without errors.  Note that we
        // must check this per-word because write permission can be set per
        // block resulting in only a section of the write not being performed.
        //
        if(ui32Status & EEPROM_EEDONE_NOPERM)
        {
            //
            // An error was reported that would prevent the values from
            // being written correctly.
            //
            if(CLASS_IS_TM4C123 && REVISION_IS_A0)
            {
                _EEPROMSectorMaskClear();
            }
            return(ui32Status);
        }

        //
        // Move on to the next word.
        //
        pui32Data++;
        ui32Count--;

        //
        // Do we need to move to the next block?  This is the case if the
        // offset register has just wrapped back to 0.  Note that we only
        // write the block register if we have more data to read.  If this
        // register is written, the hardware expects a read or write operation
        // next.  If a mass erase is requested instead, the mass erase will
        // fail.
        //
        if(ui32Count && (HWREG(EEPROM_EEOFFSET) == 0))
        {
            HWREG(EEPROM_EEBLOCK) += 1;
        }
    }

    //
    // Clear the sector protection bits to prevent possible problems when
    // programming the main flash array later.
    //
    if(CLASS_IS_TM4C123 && REVISION_IS_A0)
    {
        _EEPROMSectorMaskClear();
    }

    //
    // Return the current status to the caller.
    //
    return(HWREG(EEPROM_EEDONE));
}


// This function implements a workaround for a bug in Blizzard rev A silicon.
// It ensures that only the 1KB flash sector containing a given EEPROM address
// is erased if an erase/copy operation is required as a result of a following
// EEPROM write.
/********************************************************************************/
void _EEPROMSectorMaskSet(uint32_t ui32Address)
{
    uint32_t ui32Mask;

    //
    // Determine which page contains the passed EEPROM address.  The 2KB EEPROM
    // is implemented in 16KB of flash with each 1KB sector of flash holding
    // values for 32 consecutive EEPROM words (or 128 bytes).
    //
    ui32Mask = ~(1 << (ui32Address >> 7));

    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 3;
    SysCtlDelay(10);
    HWREG(0x400AE2C0) = ui32Mask;
    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 0;
    SysCtlDelay(10);
}


// Clear the FSM sector erase mask to ensure that any following main array
// flash erase operations operate as expected.
/********************************************************************************/
void _EEPROMSectorMaskClear(void)
{
    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 3;
    SysCtlDelay(10);
    HWREG(0x400AE2C0) = 0;
    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 0;
    SysCtlDelay(10);
}


/********************************************************************************/
void EEPROMInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    uint32_t ui32Status;

    //
    // Insert a small delay (6 cycles + call overhead) to guard against the
    // possibility that this function is called immediately after the EEPROM
    // peripheral is enabled.  Without this delay, there is a slight chance
    // that the first EEPROM register read will fault if you are using a
    // compiler with a ridiculously good optimizer!
    //
    SysCtlDelay(2);

    //
    // Make sure the EEPROM has finished any ongoing processing.
    //
//    _EEPROMWaitForDone();

    //
    // Read the EESUPP register to see if any errors have been reported.
    //
//    ui32Status = HWREG(EEPROM_EESUPP);
//
//    //
//    // Did an error of some sort occur during initialization?
//    //
//    if(ui32Status & (EEPROM_EESUPP_PRETRY | EEPROM_EESUPP_ERETRY))
//    {
////        return(EEPROM_INIT_ERROR);
//    }
//
//    //
//    // Perform a second EEPROM reset.
//    //
//    SysCtlPeripheralReset(SYSCTL_PERIPH_EEPROM0);
//
//    //
//    // Wait for the EEPROM to complete its reset processing once again.
//    //
//    SysCtlDelay(2);
////    _EEPROMWaitForDone();
//
//    //
//    // Read EESUPP once again to determine if any error occurred.
//    //
//    ui32Status = HWREG(EEPROM_EESUPP);
//
//    //
//    // Was an error reported following the second reset?
//    //
//    if(ui32Status & (EEPROM_EESUPP_PRETRY | EEPROM_EESUPP_ERETRY))
//    {
////        return(EEPROM_INIT_ERROR);
//    }

    //
    // The EEPROM does not indicate that any error occurred.
    //
//    return(EEPROM_INIT_OK);
}
