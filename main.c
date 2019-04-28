//******************************************************************************
// Authors              - Shivam Desai, Joseph Torres
// Application Name     - Lab 2
// Application Overview - The purpose of this lab is to investigate two major 
//                        serial communication protocols frequently used with micro-
//                        controllers in embedded systems: the Serial Peripheral 
//                        Interface (SPI) and the Inter-Integrated Circuit (I2C).
//                        Both aforementioned protocols are used to permit 
//                        communication between the CC3200 Launchpad’s processor 
//                        and an 128 x 128 OLED and, in the case of I2C, the 
//                        Launchpad’s on-board accelerometer as well. The first 
//                        part of the lab uses predefined test functions that utilize 
//                        the SPI interface to communicate with the OLED and display 
//                        test designs. In the second, a ball is first drawn on the
//                        OLED using a predefined circle drawing function and the 
//                        SPI interface. Then, data proportional to angles made by 
//                        tilting the board with respect to the x and y axes are 
//                        obtained from the accelerometer over I2C and used to set 
//                        the direction and magnitude with which subsequent positions 
//                        of the ball should be drawn.
//******************************************************************************

// Standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "interrupt.h"
#include "test.h"


#include "Adafruit_GFX.h"
#include "pin_mux_config.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "i2c_if.h"


#define APPLICATION_VERSION     "1.1.1"

//*****************************************************************************
//
// Application Master/Slave mode selector macro
//
// MASTER_MODE = 1 : Application in master mode
// MASTER_MODE = 0 : Application in slave mode
//
//*****************************************************************************
#define MASTER_MODE      1

#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100
#define SUCCESS                 0
#define MASTER_MSG       "This is CC3200 SPI Master Application\n\r"
#define SLAVE_MSG        "This is CC3200 SPI Slave Application\n\r"
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (0 != iRetVal) \
                                     return  iRetVal;}


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static unsigned char g_ucTxBuff[TR_BUFF_SIZE];


#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************
int
DisplayBuffer(unsigned char *pucDataBuf, unsigned char ucLen)
{
    unsigned char ucBufIndx = 0;


            int num = pucDataBuf[ucBufIndx];
            /*
            if (num > 4 && num < 70){
                printf("Right: %d\n", num);
            } else if (num >= 190 && num < 255){
                printf("Left: %d\n", num);
            } else {
                printf("Flat\n");
            }
            */
            return num;
}

int
ProcessReadRegCommand(char *pcInpString)
{
    unsigned char ucDevAddr, ucRegOffset, ucRdLen;
    unsigned char aucRdDataBuf[256];
    char *pcErrPtr;
    ucDevAddr = (unsigned char)strtoul("0x18"+2, &pcErrPtr, 16);
    ucRegOffset = (unsigned char)strtoul(pcInpString+2, &pcErrPtr, 16);
    ucRdLen = (unsigned char)strtoul("1", &pcErrPtr, 10);
    RET_IF_ERR(I2C_IF_Write(ucDevAddr,&ucRegOffset,1,0));
    RET_IF_ERR(I2C_IF_Read(ucDevAddr, &aucRdDataBuf[0], ucRdLen));

    return DisplayBuffer(aucRdDataBuf, ucRdLen);
}

//*****************************************************************************
// MasterMainPart1() implements the main program of Part I, Lab 2. Functions for  
// printing the characters in the font array, printing "Hello world!", and the 
// remaining functions in test.c as speficied in the Lab are run one bye one 
// in an infinite loop. 
//*****************************************************************************
void MasterMainPart1() {
    memcpy(g_ucTxBuff,MASTER_MSG,sizeof(MASTER_MSG));

    MAP_SPIReset(GSPI_BASE);
    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));
    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

    //
    // Send the string to slave. Chip Select(CS) needs to be
    // asserted at start of transfer and deasserted at the end.
    //

    Adafruit_Init();

    goTo(0, 0);

    while (1){
    fillScreen(BLACK);

    int i = 0;
    int x = 0;
    int y = 0;
    while (i < 1200) {
            drawChar(x, y, font[i], WHITE, BLACK, 1);
            x += 6;
            i++;
            if (i%260 == 0){
                x = 0;
                y = 0;
                fillScreen(BLACK);
            }
            else if (i%20 == 0){
                x = 0;
                y += 10;
            }
     }

    fillScreen(BLACK);
    Outstr("Hello world!");
    delay(200);

    lcdTestPattern();
    delay(100);

    lcdTestPattern2();
    delay(100);

    testlines(RED);
    delay(50);

    testfastlines(RED, GREEN);
    delay(50);

    testdrawrects(BLUE);
    delay(50);

    testfillrects(BLUE, GREEN);
    delay(50);

    testfillcircles(2, WHITE);
    delay(50);

    testroundrects();
    delay(50);

    testtriangles();
    delay(50);

    }
}
//*****************************************************************************
// MasterMainPart2() implements the main program of Part II, Lab 2. A circle   
// representing a ball is drawn to the screen. As the board is tilted, the ball 
// changes position in the direction of the tilt and with a speed proportional 
// to the angle of the tilt. 
//*****************************************************************************
void MasterMainPart2()
{

    memcpy(g_ucTxBuff,MASTER_MSG,sizeof(MASTER_MSG));

    MAP_SPIReset(GSPI_BASE);
    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    MAP_SPIEnable(GSPI_BASE);
    Adafruit_Init();

    goTo(0, 0);

    fillScreen(BLACK);

    int x = 64;
    int y = 64;
    int maxSpeed = 10;

    while (1){

        drawCircle(x, y, 5, BLUE);
        drawCircle(x, y, 5, BLACK);

        int x_read = ProcessReadRegCommand("0x3");


        if (x_read > 6 && x_read < 65){
            int diff = 65 - x_read;

            if (diff < 10){
                y-=15;
            } else if (diff < 20){
                y-=12;
            } else if (diff < 30){
                y-=9;
            } else if (diff < 40){
                y-=6;
            } else {
                y-=3;
            }

        } else if (x_read > 194 && x_read < 255){
            int diff = x_read - 194;

            if (diff < 10){
                            y+=15;
                        } else if (diff < 20){
                            y+=12;
                        } else if (diff < 30){
                            y+=9;
                        } else if (diff < 40){
                            y+=6;
                        } else {
                            y+=3;
                        }
        }

        int y_read = ProcessReadRegCommand("0x5");


        if (y_read >= 0 && y_read <= 60){
            int diff = 60 - y_read;

                        if (diff < 10){
                            x-=15;
                        } else if (diff < 20){
                            x-=12;
                        } else if (diff < 30){
                            x-=9;
                        } else if (diff < 40){
                            x-=6;
                        } else {
                            x-=3;
                        }
        } else if (y_read < 253 && y_read >= 190){
            int diff = y_read - 190;

                        if (diff < 10){
                                        x+=15;
                                    } else if (diff < 20){
                                        x+=12;
                                    } else if (diff < 30){
                                        x+=9;
                                    } else if (diff < 40){
                                        x+=6;
                                    } else {
                                        x+=3;
                                    }
        }


        if (x + 5 > 128){
            x = 128 - 5;
        }

        if (y + 5 > 128){
            y = 128 - 5;
        }

        if (x - 5 < 0){
            x = 5;
        }

        if (y - 5 < 0){
            y = 5;
        }

    }

}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
// Main function for running EITHER Part I or Part II of Lab 2 
//*****************************************************************************
void main()
{
    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Muxing UART and SPI lines.
    //
    PinMuxConfig();

    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);


    I2C_IF_Open(I2C_MASTER_MODE_FST);
    //
    // Reset the peripheral
    //
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // To run part 2, comment next line out...
    MasterMainPart1();

    // And uncomment this line....
    //MasterMainPart2(); 

    // Program should not reach here 
    while(1)
    {

    }

}

