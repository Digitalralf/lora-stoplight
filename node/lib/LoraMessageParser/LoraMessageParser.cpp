#include "LoraMessageParser.h"


unpackedMessage_t ParseLoraMessage(uint8_t loraData)
{
    unpackedMessage_t unpackedMessage;

    const static uint8_t redBitMaskOn       = 0b00000001;
    const static uint8_t yellowBitMaskOn    = 0b00000010;
    const static uint8_t greenBitMaskOn     = 0b00000100;
    
    
    const static uint8_t AutomaticModeBitMaskOn     = 0b00000000;
    const static uint8_t ManualModeBitMaskOn        = 0b00001000;
    const static uint8_t FlickerModeBitMaskOn       = 0b00010000;
    const static uint8_t InvalidModeBitMaskOn       = 0b00011000;

    const static uint8_t offMessage = ManualModeBitMaskOn;

    if(loraData != offMessage)
    {
        unpackedMessage.state = ON;
        if((loraData & redBitMaskOn) == redBitMaskOn)
        {
            unpackedMessage.color.red = true;
        }
        if((loraData & greenBitMaskOn) == greenBitMaskOn)
        {
            unpackedMessage.color.green = true;
        }
        if((loraData & yellowBitMaskOn) == yellowBitMaskOn)
        {
            unpackedMessage.color.yellow = true;
        }

        if((loraData & InvalidModeBitMaskOn) != InvalidModeBitMaskOn)
        {
            if((loraData & AutomaticModeBitMaskOn) == AutomaticModeBitMaskOn)
            {
                unpackedMessage.mode = AUTOMATIC;
            }
            if((loraData & ManualModeBitMaskOn) == ManualModeBitMaskOn)
            {
                unpackedMessage.mode = MANUAL;
            }
            if((loraData & FlickerModeBitMaskOn) == FlickerModeBitMaskOn)
            {
                unpackedMessage.mode = FLICKER;
            }
        }
    }
    else
    {
        unpackedMessage.state = OFF;
        unpackedMessage.mode = MANUAL;
    }
    return unpackedMessage;
}