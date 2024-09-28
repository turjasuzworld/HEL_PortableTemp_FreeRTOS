/*
 * TW_ESP01_v6_0_.c
 *
 *  Created on: 22-Sep-2024
 *      Author: admin
 */

#include "../inc/TW_ESP01_v6_0_.h"

/*
 * AT Command Replies
 */

char   _espDataBuff[_ESP01_DATABUFF_MAX_] = {0};
uint16_t        _dataReadFromEsp01;
/*!
 *  @brief      Get the presence of ESP01 module to report to system. Does
 *              not initialize the module, but simple checks for the AT OK
 *
 *  @param      *initFptr        function pointer to the function performing
 *                               the initialization of the GPIO port (SDK)
 *
 *  @param      *enableEsp01RtnFptr        function pointer to the function performing
 *                               the making RST & EN pins High (SDK)
 *
 *  @return     The state of the ESP01 module. If present, just reports it
 */
esp8266StateMachines checkPresenceEsp01Module(void (*initFptr)(void *),
                                              void (*enableEsp01RtnFptr)(void *),
                                              void (*disableEsp01RtnFptr)(void *),
                                              void (*wrFptr)(const void *, size_t ),
                                              void (*rdFptr)(void *, size_t , size_t* ),
                                              void (*cancFptr)(void),
                                              void (*closeFptr)(void)) {
    esp8266StateMachines _pOnState = _E8266_MODULE_NOREPLY;
    volatile char*   mdmRply = NULL;
    char*            buffPtr = NULL;
    int timeout = 3; // Try 3 times
    //Make init for EN and RST GPIOs
    if (NULL == initFptr)  {
        _pOnState = _E8266_FPTR_NULL_ERR;
        return _pOnState;
    }
    initFptr(NULL);
    //Make EN & RST pin High
    if (NULL == enableEsp01RtnFptr)  {
        _pOnState = _E8266_FPTR_NULL_ERR;
        return _pOnState;
    }
    //Check reply and set _pOnState
    rdFptr(&_espDataBuff,2048,NULL);
    do {
        enableEsp01RtnFptr(NULL);
        //wait for sometime
        sleep(1);
        //Assuming Cbks are registered, Send AT\r\n
        wrFptr("AT\r\n",4);
        sleep(1);
        buffPtr = &_espDataBuff[100];
        mdmRply=strstr(buffPtr, "AT\r\r\n\r\nOK\r\n");
        if(mdmRply) {
            _pOnState = _E8266_MODULE_PRESENT;
        } else {
            timeout--;
            disableEsp01RtnFptr(NULL);
            sleep(1);
        }

    } while ((mdmRply == NULL)&&(timeout>0));

    if(cancFptr) {
        cancFptr();
    }
    if(closeFptr) {
        closeFptr();
    }
    rdFptr(NULL,0,NULL);
    return _pOnState;
}

/*!
 *  @brief      Use this api to establish a wifi connection to the available SSIDs.
 *              This will first list all the available SSIDs, and then connect to
 *              the selected SSID
 *
 *  @param      *initFptr        function pointer to the function performing
 *                               the initialization of the GPIO port (SDK)
 *
 *  @param      *enableEsp01RtnFptr        function pointer to the function performing
 *                               the making RST & EN pins High (SDK)
 *
 *  @return     The state of the ESP01 module. If present, just reports it
 */
esp8266StateMachines listApAndConnectToSelectedSSID(void (*wrFptr)(const void *, size_t ),
                                                    void (*rdFptr)(void *, size_t , size_t* ),
                                                    uint_fast8_t* replyDone) {
    esp8266StateMachines _pOnState = _E8266_SSID_LISTING;
    //memset(_espDataBuff, 5, __ENTIRE_LENGTH_OF(_espDataBuff));
    rdFptr(&_espDataBuff,2048,NULL);
    do {
        wrFptr("AT+CWLAP\r\n",__ENTIRE_LENGTH_OF("AT+CWLAP\r\n"));
        sleep(5);
    } while (0);
    __no_lock_analysis;

}

