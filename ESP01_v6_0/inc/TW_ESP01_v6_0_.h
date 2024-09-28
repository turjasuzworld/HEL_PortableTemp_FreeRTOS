/*
 * TW_ESP01_.h
 *
 *  Created on: 22-Sep-2024
 *      Author: admin
 */

#ifndef TW_ESP01__H_
#define TW_ESP01__H_
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>



#define     __DFLT_SSID_LEN_MAX__       30
#define     __DFLT_PSWD_LEN_MAX__       30
#define     __DFLT_IPv4_LEN_MAX__       16
#define     _ESP01_DATABUFF_MAX_        2048

#define     __ENTIRE_LENGTH_OF(x)       sizeof(x)

typedef     enum    {
        _Esp_Full_Init,
        _Esp_Connect_WIFI,
        _Esp_disConnect_WIFI,
        _Esp_GET_Request,
        _Esp_UDP_GET_Request,
        _Esp_UDP_Close_Request,
        _Esp__repeated_send_Request,
        _Esp_SET_Request,
        _Esp_POST_Request,
        _Esp_CreateandConnect_TCP_Client,
}espOperCommand;


typedef enum    { //POWER ON -> UNECHO SHRT RESPNSE -> SET NTWRK TIME SYNC -> CHK NTWRK REG -> CHECK NTWRK PWR -> CHK MODULE SUPPLY VOLTAGE
                          //*   --> CHK GPRS -->EXIT
        _E8266_NON_UDP_MODE = -3,
        _E8266_MINUS2,
        _E8266_MINUS1,
        _SUCCESS,
        _FAIL,
        _E8266_INIT_SUCCESS,
        _E8266_HW_FLT,
        _E8266_HW_RETRY_TIMEOUT,
        _UNKNOWN,
        _E8266_FPTR_NULL_ERR,
        _E8266_MODULE_PRESENT,
        _E8266_MODULE_NOREPLY,
        _E8266_PWR_UP,
        _E8266_PWR_RESTART,
        _E8266_PWR_RESTART_DELAY,
        _E8266_PWR_UP_SUCCESS,              //11
        _E8266_PWR_UP_FAILURE,
        _E8266_PWR_DN,
        _E8266_PWR_OFF,
        _E8266_UNKNOWN_FAILURE,
        _E8266_RST_SUCCESS,
        _E8266_RST_FAIL,
        _E8266_AT_RESPNSE_FAIL,
        _E8266_AT_RESPNSE_SUCCESS,
        _E8266_DeECHO_FAIL,
        _E8266_DeECHO_SUCCESS,
        _E8266_STN_MODE_FAIL,
        _E8266_STN_MODE_SUCCESS,
        _E8266_SoFTAP_MODE_FAIL,
        _E8266_SoFTAP_MODE_SUCCESS,
        _E8266_MIX_MODE_FAIL,
        _E8266_MIX_MODE_SUCCESS,
//        _E8266_SoFTAP_MODE_FAIL,
//        _E8266_SoFTAP_MODE_SUCCESS,
        _E8266_WIFI_CONCTD_SUCCESS,
        _E8266_WIFI_RCV_IP_SUCCESS,
        _E8266_SERVR_CONNECT_SUCCESS,
        _E8266_SERVR_CONNECT_TIMEOUT,
        _E8266_SERVR_DISCONNCT,
        _E8266_CWJAP_UNKNWN_FAIL,
        _E8266_CWJAP_CON_TIMOUT_1,
        _E8266_CWJAP_WRNG_PSWRD_2,
        _E8266_CWJAP_AP_NT_FND_3,
        _E8266_CWJAP_CON_GENERAL_FAIL_4,
        _E8266_CIFSR_FAIL,
        _E8266_PING_SUCCESS,                //39        // THIS ALSO ENSURES INTERNET IS PRESENT = PINGING WWW.GOOGLE.COM
        _E8266_PING_FAIL,                               // THIS ALSO ENSURES THAT INTERNET IS NOT PRESENT
        _E8266_PING_TIMEOUT,
        _E8266_SSID_LISTING,
        _E8266_SSID_LISTED,
        _E8266_SSID_LIST_ERROR,
        _E8266_RSSI_RCV_SUCCESS,
        _E8266_RSSI_RCV_ERROR,
        _E8266_RSSI_RCV_TIMEOUT,
        _E8266_CIPSTART_OK,                 //45
        _E8266_CIPSTART_UDP_OK,
        _E8266_CIPCLOSE_UDP,
        _E8266_CIPCLOSE_UDP_SUCCESS,
        _E8266_CIPCLOSE_UDP_FAIL,
        _E8266_CIPSTART_ERROR,
        _E8266_CIPSTART_DNS_ERROR,
        _E8266_CIPSTART_TIMEOUT,
        _E8266_CIPSTART_SRVR_PTR_NULL,
        _E8266_CIPSTART_ALREADY_CONNCTD,
        _E8266_CIPSEND_ARROW_SUCCESS,
        _E8266_CIPSEND_ARROW_FAIL,  // ALSO INCLUDES TIMEOUT
        _E8266_CIPSEND_ARROW_TIMEOUT,
        _E8266_CIPCLOSE_FAIL,
        _E8266_CIPCLOSED_SUCCESS,
        _E8266_SEND_OK_RECVD,               //60
        _E8266_SEND_FAIL,
        _E8266_SEND_TIMEOUT,
        _E8266_MODULE_EXIT,
        _E8266_SM_CMD_ERROR,    // State MAchine Command could not be processed as the present state is wrong or not allowed
        _E8266_TEST_ENUM,
        _E8266_TCP_PORT_VAL_INVALID,
        _E8266_TCP_CONNECT_SRVR_SUCCESS,
        _E8266_TCP_CONNECT_SRVR_INVALID_STATE,
        _E8266_TCP_CREATECLIENT_FAILED,

} esp8266StateMachines;

struct      _wifiParams{
            char _wifiSSID[__DFLT_SSID_LEN_MAX__];
            char _wifiPSWD[__DFLT_PSWD_LEN_MAX__];
            char _wifiIPAddr[__DFLT_IPv4_LEN_MAX__];
            char _wifiInetPingMs[4];
            char _wifiRssi[3];
};

/*
 * Variables
 */
extern      char   _espDataBuff[_ESP01_DATABUFF_MAX_];
extern      uint16_t        _dataReadFromEsp01;
extern      bool replyDone;
/*
 *  Functions
 */
extern                      esp8266StateMachines checkPresenceEsp01Module(void (*initFptr)(void *),
                                                                          void (*enableEsp01RtnFptr)(void *),
                                                                          void (*disableEsp01RtnFptr)(void *),
                                                                          void (*wrFptr)(const void *, size_t ),
                                                                          void (*rdFptr)(void *, size_t , size_t* ),
                                                                          void (*cancFptr)(void),
                                                                          void (*closeFptr)(void));

extern                      esp8266StateMachines listApAndConnectToSelectedSSID(void (*wrFptr)(const void *, size_t ),
                                                                                void (*rdFptr)(void *, size_t , size_t* ),
                                                                                uint_fast8_t* replyDone);
#endif /* TW_ESP01__H_ */
