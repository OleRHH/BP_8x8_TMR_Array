/***********************  Touch Interrupt handler  ******************************/
/* A GPIO falling edge interrupt occurs when the touch screen is pressed.       */
/********************************************************************************/
void touchInterruptHandler(void)
{
    touchInterruptClear();

    // Read touch screen status.
    uint16_t item = touchGetMenuItem();

    // check what (or if) a button was pressed and act accordingly
    switch(item)
    {
    case LEFT_BUTTON:
        sendCommandToMotor(LEFT_BUTTON);
        break;

    case RIGHT_BUTTON:
        sendCommandToMotor(RIGHT_BUTTON);
        break;

    case STOP_BUTTON:
        sendCommandToMotor(STOP_BUTTON);
        break;

    case CHANGE_SETTING_BUTTON:
        drawSettingsMenu();
        settings.settingNo = touchGetSettingNum(settings.settingNo);
        setup = setLCDLayout(&settings);
        drawDisplayLayout();
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;

    // draws a window where the user can choose from 16 arrow length sizes
    // ranging from 10 to 160. Selection via touch.
    case ARROW_LENGTH_BUTTON:
        drawArrowLengthMenu();
        setup->maxArrowLength = touchGetArrowLength(setup->maxArrowLength);
        drawDisplayLayout();
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;

    // toggle arrow setting scaling: relative <-> absolute
    case SCALING_BUTTON:
        setup->relative = setup->relative ? false:true;
        writeInfo(SCALING);
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;

    // toggle setting hardware averaging: on <-> off
    case HARDW_AVG_BUTTON:
        setup->adcAVG = setADCHardwareAveraging(setup->adcAVG);
        writeInfo(HARDW_AVG);
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;
    }
}
