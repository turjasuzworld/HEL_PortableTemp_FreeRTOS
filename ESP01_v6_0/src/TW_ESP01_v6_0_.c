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
struct  _wifiParams     wifiParamsRetrieved;
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
 * This api will read the IP address and ping timings and other details
 */
esp8266StateMachines retrieveConnectionDetails(struct _wifiParams *wifiparams_t,
                                               void (*wrFptr)(const void *, size_t ),
                                               void (*rdFptr)(void *, size_t , size_t* ),
                                               esp8266StateMachines _pOnStateEntry) {
    esp8266StateMachines _pOnState;
    uint_fast8_t timeout1, timeout2, idx;
    char*   mdmRply = NULL;
    char*   buffPtr = NULL;
    if(_pOnStateEntry != _E8266_SERVR_CONNECT_SUCCESS) {
        return _E8266_CIFSR_PRECONDITION_FAIL;
    }
    rdFptr(NULL,0,NULL);
    memset(_espDataBuff, 0, __ENTIRE_LENGTH_OF(_espDataBuff));
    rdFptr(&_espDataBuff,2048,NULL);
    wrFptr("AT+CIFSR\r\n",__ENTIRE_LENGTH_OF("AT+CIFSR\r\n"));
    timeout1 = 5;
    do {
        sleep(3);
        buffPtr = &_espDataBuff[0];
        mdmRply = strstr(buffPtr, "+CIFSR:APIP,\"");
        if(mdmRply) {
            mdmRply += strlen("+CIFSR:APIP,\"");
            idx=0;
            while(((*mdmRply) != '"')&&(idx<__DFLT_IPv4_LEN_MAX__)) {
                wifiparams_t->_APIP[idx] = *mdmRply;
                mdmRply++;
                idx++;
            }
        }

        mdmRply = strstr(buffPtr, ":APMAC,\"");
        if(mdmRply) {
            mdmRply += strlen(":APMAC,\"");
            idx=0;
            while(((*mdmRply) != '"')&&(idx<__DFLT_MAC_LEN_MAX__)) {
                wifiparams_t->_APMAC[idx] = *mdmRply;
                mdmRply++;
                idx++;
            }
        }

        mdmRply = strstr(buffPtr, ":STAIP,\"");
        if(mdmRply) {
            mdmRply += strlen(":STAIP,\"");
            idx=0;
            while(((*mdmRply) != '"')&&(idx<__DFLT_IPv4_LEN_MAX__)) {
                wifiparams_t->_STAIP[idx] = *mdmRply;
                mdmRply++;
                idx++;
            }
        }

        mdmRply = strstr(buffPtr, ":STAMAC,\"");
        if(mdmRply) {
            mdmRply += strlen(":STAMAC,\"");
            idx=0;
            while(((*mdmRply) != '"')&&(idx<__DFLT_MAC_LEN_MAX__)) {
                wifiparams_t->_STAMAC[idx] = *mdmRply;
                mdmRply++;
                idx++;
            }
            _pOnState =  _E8266_CIFSR_COMPLETE;
        }
        timeout1 = 0;
    } while (timeout1 > 0);

    // Check for Pinging

    rdFptr(NULL,0,NULL);
    memset(_espDataBuff, 0, __ENTIRE_LENGTH_OF(_espDataBuff));
    rdFptr(&_espDataBuff,2048,NULL);
    wrFptr("AT+PING=\"www.turjasuzworld.in\"\r\n",__ENTIRE_LENGTH_OF("AT+PING=\"www.turjasuzworld.in\"\r\n"));
    timeout1 = 5;
    do {
        sleep(5);
        buffPtr = &_espDataBuff[0];
        mdmRply = strstr(buffPtr, "OK");
        if(mdmRply) {
            _pOnState =  _E8266_PING_SUCCESS;
            mdmRply = strstr(buffPtr, "+");
            mdmRply++;
            idx=0;
            while(((*mdmRply) != '\r')&&(idx<4)) {
                wifiparams_t->_wifiInetPingMs[idx] = *mdmRply;
                mdmRply++;
                idx++;
            }
            timeout1 = 0;
        } else if (NULL != strstr(buffPtr, "ERROR")) {
            _pOnState =  _E8266_PING_FAIL;
            timeout1--;
        }

    } while (timeout1 > 0);

    return _pOnState;
}


/*
 *
 */
esp8266StateMachines connectToSelected_AP(void (*wrFptr)(const void *, size_t ),
                                          void (*rdFptr)(void *, size_t , size_t* ),
                                          char* ssidName, char* ssidPassword, char* ssidMAC) {
    esp8266StateMachines _pOnState = _E8266_CWJAP_CONNECTING;
    int timeout = 3;
    char*   mdmRply = NULL;
    char*   buffPtr = NULL;
    char _atCmdStr[100] = "AT+CWJAP=\"";
    char* _p_atCmdStr = _atCmdStr;
    _p_atCmdStr = strcat(_p_atCmdStr, (const char*)ssidName);
    _p_atCmdStr = strcat(_p_atCmdStr, "\",\"");
    _p_atCmdStr = strcat(_p_atCmdStr, (const char*)ssidPassword);
    _p_atCmdStr = strcat(_p_atCmdStr, "\"\r\n");
    do {
        memset(_espDataBuff, 0, __ENTIRE_LENGTH_OF(_espDataBuff));
        rdFptr(&_espDataBuff,2048,NULL);
        wrFptr(_p_atCmdStr,strlen((const char*)_p_atCmdStr));
        sleep(10);
        buffPtr = &_espDataBuff[0];
        mdmRply = strstr(buffPtr, "busy");
        if(mdmRply) {
            // Modem busy, wait do nothing
            sleep(5);
            timeout--;
        }
        else if(NULL != strstr(buffPtr, "\r\nOK\r\n")) {
            _pOnState =  _E8266_SERVR_CONNECT_SUCCESS;
            // Optional populate the IP address received AT+CIFSR\r\n//
#ifdef      _WIFI_CONN_DETAILS_NEEDED_
            _pOnState = retrieveConnectionDetails(&wifiParamsRetrieved, wrFptr, rdFptr, _pOnState);
            if((_pOnState == _E8266_CIFSR_COMPLETE)||(_pOnState == _E8266_PING_SUCCESS)) timeout = 0;
            // if(_pOnState == _E8266_PING_FAIL) timeout = 0; // ENABLE THIS LINE ONLY IF YOU WANT TO IGNORE PING FAILURES
#else
            timeout = 0;
#endif

        } else {
            // set the correct failure cause

            //decrement timeout
            timeout--;
        }
        rdFptr(NULL,0,NULL);
    } while ((_pOnState !=  _E8266_SERVR_CONNECT_SUCCESS)&&(timeout>0));

    return  _pOnState;
}

/* Establish TCP / UDP / SSL Connection using ESP01 Modem.
 * Can be made generic to other modules later
 *
 * // Single connection (AT+CIPMUX=0):
 * AT+CIPSTART=<"type">,<"remote host">,<remote port>[,<keep alive>][,<"local IP">]
 *
 * // Multiple Connections (AT+CIPMUX=1):
 * AT+CIPSTART=<link ID>,<"type">,<"remote host">,<remote port>[,<keep alive>][,<"local IP">]
 *
 */
esp8266StateMachines connectToServer(void (*wrFptr)(const void *, size_t ),
                                          void (*rdFptr)(void *, size_t , size_t* ),
                                          char* serverUrl,
                                          char* port,
                                          clientConnectionTypes connType, esp8266StateMachines entryState) {
    esp8266StateMachines _pOnState = entryState;
    if(!((entryState == _E8266_PING_SUCCESS)||(entryState == _E8266_CIFSR_COMPLETE))) {
        return _E8266_CIPSTART_ERROR;
    }
    if((connType != _Esp_TCP)&&(connType != _Esp_UDP)&&(connType != _Esp_TCPv6)&&(connType != _Esp_TCP_SSL)) {
        return _E8266_CIPSTART_CONNTYPE_ERROR;
    }
    if((NULL == serverUrl)||(NULL == port)) {
        return _E8266_CIPSTART_SRVR_PTR_OR_PORT_VAL_NULL;
    }
    int timeout = 3;
    char*   mdmRply = NULL;
    char*   buffPtr = NULL;
    //char _atCmdStr[100] = "AT+CIPSTART=\"TCP\",\"";
    char _atCmdStr[100] = "AT+CIPSTART=\"";
    char* _p_atCmdStr = _atCmdStr;
    switch (connType) {
        case _Esp_TCP:
            _p_atCmdStr = strcat(_p_atCmdStr, "TCP\",\"");
            break;
        case _Esp_TCP_SSL:
            _p_atCmdStr = strcat(_p_atCmdStr, "SSL\",\"");
            break;
        default:
            break;
    }
    _p_atCmdStr = strcat(_p_atCmdStr, (const char*)serverUrl);
    _p_atCmdStr = strcat(_p_atCmdStr, "\",");
    _p_atCmdStr = strcat(_p_atCmdStr, (const char*)port);
    _p_atCmdStr = strcat(_p_atCmdStr, "\r\n");
    do {
        memset(_espDataBuff, 0, __ENTIRE_LENGTH_OF(_espDataBuff));
        rdFptr(&_espDataBuff,2048,NULL);
//        wrFptr("AT+CIPMUX=1\r\n",strlen("AT+CIPMUX=1\r\n"));
//        sleep(5);
        wrFptr(_p_atCmdStr,strlen((const char*)_p_atCmdStr));
        sleep(10);
        buffPtr = &_espDataBuff[0];
        mdmRply = strstr(buffPtr, "CONNECT");
        if(mdmRply) {
            // succesfully connected to server
            _pOnState = _E8266_CIPSTART_OK;
            timeout = 0;
        }
        else if(NULL != strstr(buffPtr, "ALREADY")) {
            // Already connection exists.. continue with sending data
            _pOnState =  _E8266_CIPSTART_ALREADY_CONNCTD;
            timeout = 0;
        }
        else if(NULL != strstr(buffPtr, "DNS Fail")) {
            // set the correct failure cause for DNS Failure reported
            _pOnState =  _E8266_CIPSTART_DNS_ERROR;

            timeout = 0;
        }
        else if(NULL != strstr(buffPtr, "FAIL")) {
            // set the correct failure cause for DNS Failure reported
            _pOnState =  _E8266_CIPSTART_ERROR;

            timeout--;
        }
        else if(NULL != strstr(buffPtr, "CLOSED")) {
            // set the correct failure cause for DNS Failure reported
            _pOnState =  _E8266_CIPSTART_CLOSE_ERROR;
            timeout--;
        }
        else {
            // Other undefined error, may need to restart ESP01 module
            _pOnState =  _E8266_CIPSTART_ERROR;
            timeout = 0;
        }
        rdFptr(NULL,0,NULL);
    } while ((_pOnState !=  _E8266_CIPSTART_OK)&&(timeout>0));

    return  _pOnState;
}


