/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/UART2.h>
#include <string.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Application Header */
#include "ESP01_v6_0/inc/TW_ESP01_v6_0_.h"

//const char  echoPrompt[] = "AT\r\n";


    int_fast16_t Gstatus;
    uint8_t buff_Read[2048]={0};
    UART2_Handle uart;
    UART2_Params uartParams;
    bool replyDone = false;
    struct _availableSSIDs* scannedSSID_details[_MAX_SSID_SCAN_SUPPORTED_];

void    readCbKFn(UART2_Handle handle, void *buf, size_t count,
                  void *userArg, int_fast16_t status) {
    //usleep(1000);
 //   memcpy(buff_Read, buf, count);
    UART2_readCancel(uart);
//    if(NULL != strstr(buf,"\r\n")) {

//    }
//    replyDone = false;

//    else {
//        buf++;
//        UART2_read(uart, (void*)buff_Read,1,NULL);
//    }

    static int16_t count_ =0;
            count_ = count;
    static int_fast16_t status_ = 0;
            status_ = status;
}

void    writeCbKFn(UART2_Handle handle, void *buf, size_t count,
                   void *userArg, int_fast16_t status) {
    //usleep(10);
//
    //while(1);

    //UART_write(handle, buf, count);

}

void esp01GpioInitFxn(void *arg0)
{
    GPIO_init();
    GPIO_setConfig(TW_ESP01_EN, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(TW_ESP01_RST, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_write(TW_ESP01_EN , 0);
    GPIO_write(TW_ESP01_RST, 0);
}

void esp01GpioCtrlFxn(void *arg0)
{
    GPIO_write(TW_ESP01_EN, 1);
    GPIO_write(TW_ESP01_RST, 1);
}

void esp01GpioDisableFxn(void *arg0)
{
    GPIO_write(TW_ESP01_EN, 0);
    GPIO_write(TW_ESP01_RST, 0);
}

void uart2ReadCancel(void) {
    UART2_readCancel(uart);
}

void uart2WriteCancel(void) {
    UART2_writeCancel(uart);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    volatile char        input[20];
    int idx = 0;
    //



    /* Call driver init functions */
    GPIO_init();


    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);


}

void writeToEspUart(const void *buffer, size_t size)//UART2_Handle handle, ,, size_t *bytesWritten
{
    UART2_write(uart, buffer, size,NULL);
}

void readFromEspUart(void *buffer, size_t size, size_t* bytesRead)//UART2_Handle handle, ,, size_t *bytesWritten
{
    UART2_read(uart, buffer, size, bytesRead);
}

void *testThread(void *arg0)
{
    /* Init the UART    */

    UART_init();

     /* Create a UART with data processing off. */
    UART2_Params_init(&uartParams);
    uartParams.readReturnMode = UART2_ReadReturnMode_PARTIAL;
    uartParams.baudRate = 115200;
    uartParams.readMode = UART2_Mode_CALLBACK;
    uartParams.writeMode = UART2_Mode_CALLBACK;
    uartParams.readCallback = &readCbKFn;
    uartParams.writeCallback = &writeCbKFn;

    uart = UART2_open(CONFIG_UART2_0, &uartParams);

    if (uart == NULL) {
         /* UART_open() failed */
         while (1);
     }
    static esp8266StateMachines basicState = _E8266_PWR_UP;
    while(1) {

        switch (basicState) {
            case _E8266_PWR_UP: //Check ESP01 module presence
                basicState = checkPresenceEsp01Module(&esp01GpioInitFxn,
                                                      &esp01GpioCtrlFxn,
                                                      &esp01GpioDisableFxn,
                                                      &writeToEspUart,
                                                      &readFromEspUart,
                                                      NULL,
                                                      NULL);
                break;
            case _E8266_MODULE_PRESENT:
                basicState = listApAndConnectToSelectedSSID(&writeToEspUart,
                                                            &readFromEspUart,
                                                            &scannedSSID_details);

                break;
            case _E8266_MODULE_NOREPLY:

                break;
            case _E8266_SSID_LISTED:
                basicState = connectToSelected_AP(&writeToEspUart,
                                                  &readFromEspUart,
                                                  "TurjasuBLR_2.4G",
                                                  "Stk#41912", NULL);
                break;
            default:
                break;
        }

    }
}
