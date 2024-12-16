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
#include <ti/drivers/SPI.h>
#include <string.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Application Header */
#include "ESP01_v6_0/inc/TW_ESP01_v6_0_.h"

#define SPI_MSG_LENGTH  (1)
#define MASTER_MSG      ("Y")

unsigned char masterRxBuffer[SPI_MSG_LENGTH];
unsigned char masterTxBuffer[SPI_MSG_LENGTH];

bool            transferOK;
int32_t         status;
uint16_t        t = 0;
int_fast16_t Gstatus;
uint8_t buff_Read[2048]={0};
UART2_Handle uart;
UART2_Params uartParams;
bool replyDone = false;
struct _availableSSIDs* scannedSSID_details[_MAX_SSID_SCAN_SUPPORTED_];
//SPI INIT
SPI_Handle      masterSpi;
SPI_Params      spiParams;
SPI_Transaction transaction;

char    url[256] = "GET http://www.turjasuzworld.in/demo/api/setdevicetemp.php?did=TD1003-1&temp=000.00&day=19&mon=07&year=2020&hh=15&mm=52&ss=47&signal=45 HTTP/1.1\r\nHost: Turjasu\r\nConnection: keep-alive\r\n\r\n";
char*   ptrToUrl = NULL;

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

void esp01ResetFxn(void *arg0)
{
    GPIO_write(TW_ESP01_EN, 0);
    sleep(1);
    GPIO_write(TW_ESP01_EN, 1);
    sleep(1);
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


void writeToEspUart(const void *buffer, size_t size)//UART2_Handle handle, ,, size_t *bytesWritten
{
    UART2_write(uart, buffer, size,NULL);
}

void readFromEspUart(void *buffer, size_t size, size_t* bytesRead)//UART2_Handle handle, ,, size_t *bytesWritten
{
    UART2_read(uart, buffer, size, bytesRead);
}

float   readTempFromMAX6675(uint_fast8_t channel)
{
    static float  temp ;
    switch (channel) {
        case 1:
            GPIO_write(MAX6675_1, 0);
            break;
        case 2:
            GPIO_write(MAX6675_2, 0);
            break;
        case 3:
            GPIO_write(MAX6675_3, 0);
            break;
        case 4:
            GPIO_write(MAX6675_4, 0);
            break;
        case 5:
            GPIO_write(MAX6675_5, 0);
            break;
        default:
            break;
    }

    usleep(1000);
    transferOK = SPI_transfer(masterSpi, &transaction);
    if (transferOK) {
        t>>=3;
        temp = t*0.25;
    }
    else {
        // do some error handling here
    }

    /* Sleep for a bit before starting the next SPI transfer  */
    switch (channel) {
        case 1:
            GPIO_write(MAX6675_1, 1);
            break;
        case 2:
            GPIO_write(MAX6675_2, 1);
            break;
        case 3:
            GPIO_write(MAX6675_3, 1);
            break;
        case 4:
            GPIO_write(MAX6675_4, 1);
            break;
        case 5:
            GPIO_write(MAX6675_5, 1);
            break;
        default:
            break;
    }
    usleep(2000);
    return temp;
}

void *testThread(void *arg0)
{
    /* Init the UART    */

    UART_init();

    // Init the GPIO
    GPIO_init();
    // LEDS
    GPIO_setConfig(STS_LED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(FLT_LED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    // Chip Selects
    GPIO_setConfig(MAX6675_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    GPIO_setConfig(MAX6675_2, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    GPIO_setConfig(MAX6675_3, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    GPIO_setConfig(MAX6675_4, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    GPIO_setConfig(MAX6675_5, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);

    SPI_init();

    /* Open SPI as master (default) */
    SPI_Params_init(&spiParams);
    spiParams.frameFormat = SPI_POL0_PHA0;//SPI_POL1_PHA0;
    spiParams.bitRate = 1000000;
    spiParams.dataSize = 16;
    masterSpi = SPI_open(CONFIG_SPI_MASTER, &spiParams);
    if (masterSpi == NULL) {
        while (1);
    }
    /* Copy message to transmit buffer */
    strncpy((char *) masterTxBuffer, MASTER_MSG, SPI_MSG_LENGTH);
    memset((void *) masterRxBuffer, 1, SPI_MSG_LENGTH);
    transaction.count = SPI_MSG_LENGTH;
    transaction.txBuf = (void *) masterTxBuffer;
    transaction.rxBuf = &t;//(void *) masterRxBuffer;



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
    struct  _wifiParams* ptrToConnectionDetails;
    volatile float read_temp = 0;
    volatile int TC_Count = 0, var = 0;
    char* tmp = NULL;
    while(1) {

        switch (basicState) {

            case _E8266_SEND_OK_AND_CLOSED_NOT_RECVD:
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
                // Ensure that the AP selected actually exists
                for (var = 0; var < _MAX_SSID_SCAN_SUPPORTED_; ++var) {
                    if(NULL == strstr(&_scannedSsidList[var]._ssidName[0], "Hel Secure"))
                    {
                        if(var == (_MAX_SSID_SCAN_SUPPORTED_-1))
                            {
                                basicState = _E8266_PREFFERED_AP_NOT_EXIST;
                            }
                    }
                    else
                    {
                        var = _MAX_SSID_SCAN_SUPPORTED_;
                    }
                }

                break;

            case _E8266_PREFFERED_AP_NOT_EXIST:
                GPIO_toggle(FLT_LED);
                basicState = _E8266_PWR_UP; // check again if AP has come up ?
                break;
            case _E8266_MODULE_NOREPLY:

                break;
            case _E8266_SSID_LISTED:
                GPIO_write(FLT_LED, 0); // if SSID is found, turn off Fault LED
                basicState = connectToSelected_AP(&writeToEspUart,
                                                  &readFromEspUart,
                                                  "Hel Secure",
                                                  "helsite987", NULL);
                ptrToConnectionDetails = &wifiParamsRetrieved;
                break;

            case _E8266_CIFSR_COMPLETE:
            case _E8266_PING_SUCCESS:
            case _E8266_SEND_OK_RECVD: // For backward compatibility, _E8266_SEND_OK_AND_CLOSED_RECVD added to resolve
            case _E8266_SEND_OK_AND_CLOSED_RECVD:
                //Data sent succesfully, reset the fault LED if lighted before
                GPIO_write(FLT_LED, 0);
                // Fetch the RSSI before connecting to Server/ internet activity

                // Try to connect to Server using TCP/UDP
                basicState = connectToServer(&writeToEspUart,
                                             &readFromEspUart,
                                             "turjasuzworld.in",
                                             "80" ,
                                             _Esp_TCP,
                                             basicState);
                break;

            case _E8266_PING_FAIL:
                GPIO_write(FLT_LED, 1);
                break;

            case _E8266_SEND_FAIL:
            case _E8266_CIPSEND_CONN_SRVR_CLOSED:
            case _E8266_CIPSEND_ARROW_FAIL:

                GPIO_write(FLT_LED, 1);

                break;

            case _E8266_CIPSTART_OK:
                GPIO_write(STS_LED, 1);
                  // Append RSSI value

                    for (var = 0; var < _MAX_SSID_SCAN_SUPPORTED_; ++var) {
                        if(strstr(&_scannedSsidList[var]._ssidName[0], "Hel Secure"))
                        {
                              tmp = strstr(url, "&signal=");
                              if(tmp)
                              {
                                  tmp += strlen("&signal=");
                                  *tmp = _scannedSsidList[var]._ssidRSSI[1];
                                  tmp++;
                                  *tmp = _scannedSsidList[var]._ssidRSSI[2];
                              }
                              var = _MAX_SSID_SCAN_SUPPORTED_; // terminate the loop
                        }
                    }

//                for (TC_Count = 0; TC_Count < 5; ++TC_Count) {
                    read_temp = readTempFromMAX6675(TC_Count + 1);
                    ptrToUrl  = strstr(url, "did=TD1003-");
                    if(ptrToUrl)
                    {
                        ptrToUrl += strlen("did=TD1003-");
                        *ptrToUrl = 48 + (TC_Count + 1);
                    }
                    ptrToUrl  = strstr(url, "temp=");
                    if(ptrToUrl)
                    {
                        ptrToUrl += strlen("temp=");
                        *ptrToUrl = 48 + (read_temp*100)/10000;
                        ptrToUrl++;
                        *ptrToUrl = 48 + (((int)(read_temp*100))%10000)/1000;
                        ptrToUrl++;
                        *ptrToUrl = 48 + (((int)(read_temp*100))%1000)/100;
                        ptrToUrl++;
                        *ptrToUrl = '.';
                        ptrToUrl++;
                        *ptrToUrl = 48 + (((int)(read_temp*100))%100)/10;
                        ptrToUrl++;
                        *ptrToUrl = 48 + ((int)(read_temp*100))%10;


                // Try to connect to Server using TCP/UDP
                basicState = sendDataToConnectedSocket(&writeToEspUart,
                                                       &readFromEspUart,
                                                       &url[0],//"GET http://www.turjasuzworld.in/demo/api/setdevicetemp.php?did=TD1003-1&temp=000.00&day=19&mon=07&year=2020&hh=15&mm=52&ss=47&signal=45 HTTP/1.1\r\nHost: Turjasu\r\nConnection: keep-alive\r\n\r\n",
                                                       _Esp_TCP,
                                                       basicState);
                TC_Count++;
                if(TC_Count == 5) TC_Count = 0;
                    }
//                }

                GPIO_write(STS_LED, 0);
                break;

            default:
                break;
        }
    }
}
