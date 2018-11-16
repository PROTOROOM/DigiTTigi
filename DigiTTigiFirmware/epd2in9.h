/**
  ******************************************************************************
  * @file    edp2in9.h
  * @author  Waveshare Team
  * @version V1.0.0
  * @date    23-January-2018
  * @brief   This file describes initialisation of 2.9 and 2.9b e-Papers
  *
  ******************************************************************************
  */
int EPD_Init_2in9() 
{
    EPD_Reset();
    EPD_Send_3(0x01, 39, 1, 0);//DRIVER_OUTPUT_CONTROL:LO(EPD_HEIGHT-1),HI(EPD_HEIGHT-1), GD = 0; SM = 0; TB = 0;
    EPD_Send_3(0x0C, 0xD7, 0xD6, 0x9D);//BOOSTER_SOFT_START_CONTROL
    EPD_Send_1(0x2C, 0xA8);//WRITE_VCOM_REGISTER:VCOM 7C
    EPD_Send_1(0x3A, 0x1A);//SET_DUMMY_LINE_PERIOD: 4 dummy lines per gate
    EPD_Send_1(0x3B, 0x08);//SET_GATE_TIME: 2us per line
    EPD_Send_1(0x11, 0x03);//DATA_ENTRY_MODE_SETTING: X increment; Y increment
 
    EPD_Send_2(0x44, 0, 15);//SET_RAM_X_ADDRESS_START_END_POSITION: LO(x >> 3), HI ((w-1) >> 3)
    EPD_Send_4(0x45, 0, 0, 45, 1);//SET_RAM_Y_ADDRESS_START_END_POSITION: LO(y), HI(y), LO(h - 1),HI(h - 1)

    EPD_Send_1(0x4E, 0);//SET_RAM_X_ADDRESS_COUNTER: LO(x >> 3)
    EPD_Send_2(0x4F, 0, 0);//SET_RAM_Y_ADDRESS_COUNTER: LO(y), HI(y)

    EPD_lut(0x32, 30, &lut_full_mono[0]);

    EPD_SendCommand(0x24);//WRITE_RAM
    delay(2);
    return 0;
}

int EPD_Init_2in9b() 
{
    EPD_Reset();
    EPD_Send_4(0x01, 0x07, 0x00, 0x08, 0x00);//POWER_SETTING
    EPD_Send_3(0x06, 0x17, 0x17, 0x17);//BOOSTER_SOFT_START
    EPD_SendCommand(0x04);//POWER_ON
    EPD_WaitUntilIdle();
    
    EPD_Send_1(0x00, 0x8F);//PANEL_SETTING
    EPD_Send_1(0x50, 0x77);//VCOM_AND_DATA_INTERVAL_SETTING;
    EPD_Send_1(0x30, 0x39);//PLL_CONTROL
    EPD_Send_3(0x61, 0x80, 0x01, 0x28);//TCON_RESOLUTION
    EPD_Send_1(0x82, 0x0A);//VCM_DC_SETTING_REGISTER

    EPD_SendCommand(0x10);//DATA_START_TRANSMISSION_1  
    delay(2);
    return 0;
}
