#ifndef __ESP01S_AT_H__
#define __ESP01S_AT_H__

#include <stdbool.h>
#include "esp01s_at_type.h"

/**
 * @brief 初始化ESP01S AT模块
 * 
 * @param void
 * @return esp01s_at_err_t
 */
esp01s_at_err_t esp01s_test(void);

/**
 * @brief 设置ESP01S工作模式
 * 
 * @param esp01s_mode_t ESP01S工作模式
 * @return esp01s_at_err_t
 */
esp01s_at_err_t esp01s_set_mode(esp01s_mode_t mode);

/**
 * @brief 设置ESP01S连接模式
 * 
 * @param esp01s_connection_mode_t ESP01S连接模式
 * @return esp01s_at_err_t
 */
esp01s_at_err_t esp01s_set_connection_mode(esp01s_connection_mode_t mode);

/**
 * @brief 连接ESP01S到WiFi网络
 * 
 * @param esp01s_wifi_t ESP01S WiFi配置结构体指针
 * @return esp01s_at_err_t
 */
esp01s_at_err_t esp01s_wifi_connect(esp01s_wifi_t *wifi);

/**
 * @brief 连接ESP01S到TCP服务器
 * 
 * @param esp01s_tcp_t ESP01S TCP配置结构体指针
 * @return esp01s_at_err_t
 */
esp01s_at_err_t esp01s_tcp_connect(esp01s_tcp_t *tcp);

/**
 * @brief 进入透传模式
 * 
 * @param void
 * @return esp01s_at_err_t
 */
esp01s_at_err_t esp01s_enter_transmit_mode(void);

/**
 * @brief 检查ESP01S AT模块错误码
 * 
 * @param esp01s_at_err_t ESP01S AT模块错误码
 */
void esp01s_err_check(esp01s_at_err_t err);
#endif
