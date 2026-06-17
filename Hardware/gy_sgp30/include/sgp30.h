#ifndef __SGP30_H__
#define __SGP30_H__

#include <stdbool.h>
#include "sgp30_type.h"

// 读取序列号，成功返回true，并在serial_id写入48位序列号
bool sgp30_read_serial_id(uint64_t *serial_id);

// 读取特征集版本，成功返回true，并在feature_set写入16位版本值
bool sgp30_read_feature_set(uint16_t *feature_set);

// 初始化空气质量测量
bool sgp30_init_air_quality(void);

// 测量空气质量 (CO2当量和TVOC)
bool sgp30_measure_air_quality(sgp30_air_quality_t *air_quality);

// 获取基线值
bool sgp30_get_baseline(sgp30_baseline_t *baseline);

// 设置基线值
bool sgp30_set_baseline(const sgp30_baseline_t *baseline);

// 设置湿度补偿 (相对湿度 * 256)
bool sgp30_set_humidity(uint16_t humidity);

// 执行自测试
bool sgp30_measure_test(uint16_t *test_result);

#endif