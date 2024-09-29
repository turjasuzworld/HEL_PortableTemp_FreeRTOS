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
struct  _availableSSIDs     _scannedSsidList[_MAX_SSID_SCAN_SUPPORTED_] = {{._securityType = '1', ._ssidName = "TEST", ._ssidMACID = "ab:cd:ef:12:34:56"}};
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
                                                    struct _availableSSIDs* ptrToListedSsids[_MAX_SSID_SCAN_SUPPORTED_]) {
    esp8266StateMachines _pOnState = _E8266_SSID_LISTING;
    //memset(_espDataBuff, 5, __ENTIRE_LENGTH_OF(_espDataBuff));
    int timeout = 3; // Try 3 times
    int idx, ssidNumberCtr = 0;
    char*   mdmRply = NULL;
    char*   trackerPtr = NULL;
    do {
        rdFptr(&_espDataBuff,2048,NULL);
        wrFptr("ATE0\r\n",__ENTIRE_LENGTH_OF("ATE0\r\n"));
        sleep(2);
        rdFptr(NULL,0,NULL);
        //AT+CWLAP=[<ssid>,<mac>,<channel>,<scan_type>,<scan_time_min>,<scan_time_max>]
        //+CWLAP:(4,"TurjasuBLR_2.4G",-22,"14:33:75:df:e7:78",1)<\r><\n>
        rdFptr(&_espDataBuff,2048,NULL);
        wrFptr("AT+CWLAP\r\n",__ENTIRE_LENGTH_OF("AT+CWLAP\r\n"));
        sleep(5);
        //Parse to save the maximum number of SSID data. This can be rendered to the Display / UI
        trackerPtr = &_espDataBuff[0];
        for (ssidNumberCtr = 0; ssidNumberCtr < _MAX_SSID_SCAN_SUPPORTED_; ++ssidNumberCtr) {

            mdmRply = strstr(trackerPtr, "+CWLAP:");
            if(mdmRply) {
                //Parsing Security type
                mdmRply += 8;
                _scannedSsidList[ssidNumberCtr]._securityType[0] = *mdmRply;
                //Parsing SSID
                mdmRply += 3;
                idx = 0;
                while((*mdmRply != '"')&&(idx < __DFLT_SSID_LEN_MAX__)) {
                    _scannedSsidList[ssidNumberCtr]._ssidName[idx] = *mdmRply;
                    mdmRply++;
                    idx++;
                }
                // Parsing RSSI
                mdmRply += 2;
                idx = 0;
                while((*mdmRply != ',')&&(idx < 4)) {
                    _scannedSsidList[ssidNumberCtr]._ssidRSSI[idx] = *mdmRply;
                    mdmRply++;
                    idx++;
                }
                // Parsing MAC ID
                mdmRply += 2;
                idx = 0;
                while((*mdmRply != '"')&&(idx < 17)) {
                    _scannedSsidList[ssidNumberCtr]._ssidMACID[idx] = *mdmRply;
                    mdmRply++;
                    idx++;
                }
                // Parsing Channel Info (not used as of now
                mdmRply += 2;
                trackerPtr = mdmRply;
                if(ptrToListedSsids != NULL) {
                    ptrToListedSsids[ssidNumberCtr] = &_scannedSsidList[ssidNumberCtr];
                }
                _pOnState = _E8266_SSID_LISTED;
            } else {
                if(ssidNumberCtr == 0) {
                    _pOnState = _E8266_SSID_LIST_ERROR;
                }
                ssidNumberCtr = _MAX_SSID_SCAN_SUPPORTED_; // To exit the for loop
                timeout--;
//                sleep(1);
            }

        }

        rdFptr(NULL,0,NULL);
    } while ((mdmRply == NULL)&&(timeout>0));
    return _pOnState;
}

/*
 *
 */
esp8266StateMachines connectToSelected_AP(void (*wrFptr)(const void *, size_t ),
                                          void (*rdFptr)(void *, size_t , size_t* ),
                                          char* ssidName, char* ssidPassword, char* ssidMAC) {
    esp8266StateMachines _pOnState = _E8266_CWJAP_CONNECTING;
    char* _cwjapStr = "AT+CWJAP=\"";
    _cwjapStr = strcat(_cwjapStr, strcat(ssidName, "\""));
    do {
        rdFptr(&_espDataBuff,2048,NULL);
        wrFptr(_cwjapStr,__ENTIRE_LENGTH_OF(*_cwjapStr));
    } while (0);

    return  _pOnState;
}

