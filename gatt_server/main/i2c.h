#ifndef I2C_H
#define I2C_H


#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"


#define I2C_MASTER_SCL_IO           22                        /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21                        /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

// #define MPU9250_PWR_MGMT_1_REG_ADDR         0x6B        /*!< Register addresses of the power managment register */
#define MPU9250_RESET_BIT                   7



#define BME280_SENSOR_ADDR           0x76  /*!< Change to 0x77 if SDO is connected to VDDIO */
#define BME280_CHIP_ID_REG           0xD0  /*!< Register address for chip ID */
#define BME280_RESET_REG             0xE0  /*!< Register address for soft reset */
#define BME280_CTRL_HUM_REG          0xF2  /*!< Register to control humidity oversampling */
#define BME280_CTRL_MEAS_REG         0xF4  /*!< Register to control temperature and pressure oversampling and mode */
#define BME280_CONFIG_REG            0xF5  /*!< Configuration register for IIR filter and standby time */
#define BME280_DATA_START_REG        0xF7  /*!< Starting register for pressure and temperature data */
#define BME280_HUM_DATA_START_REG    0xFD  /*!< Starting register for humidity data */
#define BME280_CONFIG_REG            0xF5  /*!< Configuration register for IIR filter and standby time */
#define BME280_CTRL_HUM_REG 0xF2
#define BME280_CTRL_MEAS_REG 0xF4
#define BME280_CONFIG_REG 0xF5

esp_err_t read_sensor_data(float* temperature, float* pressure, float* humidity);
esp_err_t i2c_set_freq(uint8_t f);
esp_err_t set_humidity_oversampling(uint8_t oversampling);
esp_err_t set_temp_press_oversampling(uint8_t temp_oversampling, uint8_t press_oversampling);
esp_err_t set_iir_filter(uint8_t filter_coefficient);

void i2c_start(void);

#endif // I2C_H