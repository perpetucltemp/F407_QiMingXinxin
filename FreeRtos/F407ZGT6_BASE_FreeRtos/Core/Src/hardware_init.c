#include "main.h"
#include "hardware_init.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
/**
 * @brief 初始化硬件设备
 * 
 * 该函数负责执行硬件设备的初始化操作，包括但不限于外设配置、
 * 寄存器设置、时钟初始化等必要的硬件准备工作。
 * 
 * @param void 无参数
 * 
 * @return uint8_t 硬件初始化结果状态
 *         - 0: 初始化成功
 *         - 非0: 初始化失败，具体错误码待定义
 */
uint8_t hardware_init(void) {

  uint8_t ret = 0; 

  ret = MPU6050_Init(&hi2c2);

  return ret;
}
