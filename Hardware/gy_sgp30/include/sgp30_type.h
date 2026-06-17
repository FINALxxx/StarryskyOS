#ifndef __SGP30_TYPE_H__
#define __SGP30_TYPE_H__

#include <stdint.h>

// SGP30 I2C地址 (7位地址，不需要左移)
#define SGP30_I2C_ADDR     (0x58)

// SGP30命令定义
#define SGP30_CMD_GET_SERIAL_ID   (0x3682)
#define SGP30_CMD_GET_FEATURE_SET (0x202F)
#define SGP30_CMD_INIT_AIR_QUALITY (0x2003)
#define SGP30_CMD_MEASURE_AIR_QUALITY (0x2008)
#define SGP30_CMD_GET_BASELINE (0x2015)
#define SGP30_CMD_SET_BASELINE (0x201E)
#define SGP30_CMD_SET_HUMIDITY (0x2061)
#define SGP30_CMD_MEASURE_TEST (0x2032)
#define SGP30_CMD_GET_TVOC_INCEPTIVE_BASELINE (0x20B3)
#define SGP30_CMD_SET_TVOC_BASELINE (0x2077)

// SGP30测量数据结构
typedef struct {
    uint16_t co2_eq_ppm;    // CO2当量浓度 (ppm)
    uint16_t tvoc_ppb;      // TVOC浓度 (ppb)
} sgp30_air_quality_t;

// SGP30基线数据结构
typedef struct {
    uint16_t co2_eq_baseline;
    uint16_t tvoc_baseline;
} sgp30_baseline_t;

#endif