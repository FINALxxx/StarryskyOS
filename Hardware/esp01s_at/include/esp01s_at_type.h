#ifndef __ESP01S_AT_TYPE_H__
#define __ESP01S_AT_TYPE_H__

#include <stdint.h>

/**
 * @brief ESP01S AT模块错误码
 * 
 * @param ESP01S_AT_ERR_OK 成功
 * @param ESP01S_AT_ERR_INVALID_PARAM 无效参数
 * @param ESP01S_AT_ERR_TIMEOUT 超时
 */
typedef enum esp01s_at_err_t {
    ESP01S_AT_ERR_OK = 0,
    ESP01S_AT_ERR_INVALID_PARAM = 1,
    ESP01S_AT_ERR_TIMEOUT = 2,
} esp01s_at_err_t;

/**
 * @brief ESP01S工作模式
 * 
 * @param ESP01S_MODE_STA 客户端模式
 * @param ESP01S_MODE_AP 服务器模式
 * @param ESP01S_MODE_AP_STA 客户端服务器模式
 */
typedef enum esp01s_mode_t {
    ESP01S_MODE_STA = 1,
    ESP01S_MODE_AP = 2,
    ESP01S_MODE_AP_STA = 3,
} esp01s_mode_t;

/**
 * @brief ESP01S连接模式
 * 
 * @param ESP01S_CONNECTION_SIGNAL 单信号连接
 * @param ESP01S_CONNECTION_MULTI 多信号连接
 */ 
typedef enum esp01s_connection_mode_t {
    ESP01S_CONNECTION_SIGNAL = 0,
    ESP01S_CONNECTION_MULTI = 1,
} esp01s_connection_mode_t;

/**
 * @brief ESP01S WiFi配置结构体
 * 
 * @param ssid SSID
 * @param password 密码
 */
typedef struct esp01s_wifi_t {
    char ssid[32];
    char password[64];
} esp01s_wifi_t;

/**
 * @brief ESP01S TCP配置结构体
 * 
 * @param host 主机名
 * @param port 端口号
 */
typedef struct esp01s_tcp_t {
    char ip[32];
    char port[8];
} esp01s_tcp_t;


#endif