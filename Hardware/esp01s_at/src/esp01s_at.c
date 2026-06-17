#include "esp01s_at.h"
#include "hal_sys_uart.h"
#include "hal_hp_uart.h"

uint8_t response[128];

esp01s_at_err_t esp01s_test(){
    uint32_t timeout = 100;
    hal_hp_uart_putstr("AT+GMR\r\n");
    while(timeout--) {
        hal_hp_uart_recv_str(response);
        hal_sys_putstr(response);
        if (response[0] == 'O' && response[1] == 'K') {
            return ESP01S_AT_ERR_OK;
        }
    }
    return ESP01S_AT_ERR_TIMEOUT;
}

esp01s_at_err_t esp01s_set_mode(esp01s_mode_t mode){
    uint32_t timeout = 100;
    switch(mode){
        case ESP01S_MODE_STA:
            hal_hp_uart_putstr("AT+CWMODE=1\r\n");
            break;
        case ESP01S_MODE_AP:
            hal_hp_uart_putstr("AT+CWMODE=2\r\n");
            break;
        case ESP01S_MODE_AP_STA:
            hal_hp_uart_putstr("AT+CWMODE=3\r\n");
            break;
        default:
            return ESP01S_AT_ERR_INVALID_PARAM;
    }
    response[0] = '\0';
    hal_sys_putstr("set mode: ");
    while(timeout--) {
        hal_hp_uart_recv_str(response);
        hal_sys_putstr(response);
        if (response[0] == 'O' && response[1] == 'K') {
            return ESP01S_AT_ERR_OK;
        }
    }
    return ESP01S_AT_ERR_TIMEOUT;
}

esp01s_at_err_t esp01s_wifi_connect(esp01s_wifi_t *wifi){
    uint32_t timeout = 10000;
    hal_hp_uart_putstr("AT+CWJAP=\"");
    hal_hp_uart_putstr(wifi->ssid);
    hal_hp_uart_putstr("\",\"");
    hal_hp_uart_putstr(wifi->password);
    hal_hp_uart_putstr("\"");
    hal_hp_uart_putstr("\r\n");
    response[0] = '\0';
    hal_sys_putstr("connect: ");
    while(timeout--) {
        hal_hp_uart_recv_str(response);
        hal_sys_putstr(response);
        if (response[0] == 'O' && response[1] == 'K') {
            return ESP01S_AT_ERR_OK;
        }
    }
    return ESP01S_AT_ERR_TIMEOUT;
}

esp01s_at_err_t esp01s_set_connection_mode(esp01s_connection_mode_t mode){
    uint32_t timeout = 100;
    switch(mode){
        case ESP01S_CONNECTION_SIGNAL:
            hal_hp_uart_putstr("AT+CIPMUX=0\r\n");
            break;
        case ESP01S_CONNECTION_MULTI:
            hal_hp_uart_putstr("AT+CIPMUX=1\r\n");
            break;
        default:
            return ESP01S_AT_ERR_INVALID_PARAM;
    }
    response[0] = '\0';
    hal_sys_putstr("set connection mode: ");
    while(timeout--) {
        hal_hp_uart_recv_str(response);
        hal_sys_putstr(response);
        if (response[0] == 'O' && response[1] == 'K') {
            return ESP01S_AT_ERR_OK;
        }
    }
    return ESP01S_AT_ERR_TIMEOUT;
}

esp01s_at_err_t esp01s_tcp_connect(esp01s_tcp_t *tcp){
    uint32_t timeout = 10000;
    hal_hp_uart_putstr("AT+CIPSTART=\"TCP\",\"");
    hal_hp_uart_putstr(tcp->ip);
    hal_hp_uart_putstr("\",");
    hal_hp_uart_putstr(tcp->port);
    hal_hp_uart_putstr("\r\n");
    response[0] = '\0';
    hal_sys_putstr("connect tcp server: ");
    while(timeout--) {
        hal_hp_uart_recv_str(response);
        hal_sys_putstr(response);
        if (response[0] == 'O' && response[1] == 'K') {
            return ESP01S_AT_ERR_OK;
        }
    }
    return ESP01S_AT_ERR_TIMEOUT;
}

esp01s_at_err_t esp01s_enter_transmit_mode(){
    hal_sys_putstr("enter transmit mode: ");
    uint32_t timeout = 100;
    hal_hp_uart_putstr("AT+CIPMODE=1\r\n");
    response[0] = '\0';
    while(timeout--) {
        hal_hp_uart_recv_str(response);
        hal_sys_putstr(response);
        if (response[0] == 'O' && response[1] == 'K') {
            return ESP01S_AT_ERR_OK;
        }
    }
    timeout = 100;
    hal_hp_uart_putstr("AT+CIPSEND\r\n");
    response[0] = '\0';
    while(timeout--) {
        hal_hp_uart_recv_str(response);
        hal_sys_putstr(response);
        if (response[0] == 'O' && response[1] == 'K') {
            return ESP01S_AT_ERR_OK;
        }
    }
    return ESP01S_AT_ERR_TIMEOUT;
}

void esp01s_err_check(esp01s_at_err_t err){
    switch(err){
        case ESP01S_AT_ERR_OK:
            hal_sys_putstr("success\n");
            break;
        case ESP01S_AT_ERR_INVALID_PARAM:
            hal_sys_putstr("invalid param\n");
            break;
        case ESP01S_AT_ERR_TIMEOUT:
            hal_sys_putstr("timeout\n");
            break;
        default:
            hal_sys_putstr("unknown error\n");
            break;
    }
}