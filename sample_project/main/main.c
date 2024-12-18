/* i2c - Simple example

   Simple I2C example that shows how to initialize I2C
   as well as reading and writing from and to registers for a sensor connected over I2C.

   The sensor used in this example is a MPU9250 inertial measurement unit.

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "i2c-simple-example";

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



uint16_t dig_T1;
int16_t dig_T2, dig_T3;
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
uint8_t dig_H1, dig_H3;
int16_t dig_H2;
int16_t dig_H4, dig_H5;
int8_t dig_H6;




/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
static esp_err_t mpu9250_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, BME280_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t mpu9250_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, BME280_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}




static esp_err_t bme280_configure_sensor(void)
{
    // Set humidity oversampling to x1
    ESP_ERROR_CHECK(mpu9250_register_write_byte(BME280_CTRL_HUM_REG, 0x01));

    // Set temperature and pressure oversampling to x1 and mode to normal
    ESP_ERROR_CHECK(mpu9250_register_write_byte(BME280_CTRL_MEAS_REG, 0x27));

    // Set IIR filter to off and standby time to 0.5ms
    ESP_ERROR_CHECK(mpu9250_register_write_byte(BME280_CONFIG_REG, 0x00));

    ESP_LOGI(TAG, "BME280 configured successfully");
    return ESP_OK;
}

static esp_err_t bme280_read_calibration_data(void)
{
    uint8_t buffer[26];

    // Read 26 bytes from 0x88 to 0xA1 for temperature and pressure calibration
    ESP_ERROR_CHECK(mpu9250_register_read(0x88, buffer, 26));

    dig_T1 = (buffer[1] << 8) | buffer[0];
    dig_T2 = (buffer[3] << 8) | buffer[2];
    dig_T3 = (buffer[5] << 8) | buffer[4];

    dig_P1 = (buffer[7] << 8) | buffer[6];
    dig_P2 = (buffer[9] << 8) | buffer[8];
    dig_P3 = (buffer[11] << 8) | buffer[10];
    dig_P4 = (buffer[13] << 8) | buffer[12];
    dig_P5 = (buffer[15] << 8) | buffer[14];
    dig_P6 = (buffer[17] << 8) | buffer[16];
    dig_P7 = (buffer[19] << 8) | buffer[18];
    dig_P8 = (buffer[21] << 8) | buffer[20];
    dig_P9 = (buffer[23] << 8) | buffer[22];

    dig_H1 = buffer[25];

    // Read humidity calibration from 0xE1 to 0xE7
    uint8_t hum_buffer[7];
    ESP_ERROR_CHECK(mpu9250_register_read(0xE1, hum_buffer, 7));

    dig_H2 = (hum_buffer[1] << 8) | hum_buffer[0];
    dig_H3 = hum_buffer[2];
    dig_H4 = (hum_buffer[3] << 4) | (hum_buffer[4] & 0x0F);
    dig_H5 = (hum_buffer[5] << 4) | (hum_buffer[4] >> 4);
    dig_H6 = hum_buffer[6];

    ESP_LOGI(TAG, "Calibration data loaded successfully");
    return ESP_OK;
}


int32_t t_fine; // Global variable needed for pressure and humidity compensation

int32_t compensate_temperature(int32_t adc_T) {
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
             ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8; // Scaled to hundredths of °C
    return T;                   // Return in hundredths of °C
}

uint32_t compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // Avoid division by zero
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (uint32_t)p; // Pressure in Pascals (Pa)
}

uint32_t compensate_humidity(int32_t adc_H) {
    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) -
                    (((int32_t)dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >> 15) *
                 (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) *
                      (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +
                       ((int32_t)32768))) >> 10) +
                    ((int32_t)2097152)) * ((int32_t)dig_H2) +
                   8192) >>
                  14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               ((int32_t)dig_H1)) >>
                              4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (uint32_t)(v_x1_u32r >> 12); // Humidity in %RH (scaled by 1024)
}

void read_sensor_data() {
    uint8_t data[8];
    ESP_ERROR_CHECK(mpu9250_register_read(BME280_DATA_START_REG, data, 8));

    // Extract raw values
    uint32_t pressure_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    uint32_t temperature_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    uint32_t humidity_raw = (data[6] << 8) | data[7];

    // Convert to signed 32-bit integers
    int32_t temp_raw = (int32_t)temperature_raw;
    int32_t press_raw = (int32_t)pressure_raw;
    int32_t hum_raw = (int32_t)humidity_raw;

    // Compensate and convert to human-readable values
    int32_t temperature = compensate_temperature(temp_raw); // Hundredths of °C
    uint32_t pressure = compensate_pressure(press_raw);     // Pascals (Pa)
    uint32_t humidity = compensate_humidity(hum_raw);       // %RH in Q22.10 format

    // Print results
    ESP_LOGI(TAG, "Temperature: %.2f°C", temperature / 100.0);
    ESP_LOGI(TAG, "Pressure: %.2f hPa", pressure / 256.0 / 100.0);
    ESP_LOGI(TAG, "Humidity: %.2f%%", humidity / 1024.0);
}


void app_main(void)
{
    uint8_t data[2];
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    ESP_ERROR_CHECK(bme280_configure_sensor());
    ESP_ERROR_CHECK(bme280_read_calibration_data());

    read_sensor_data();

    /* Read the MPU9250 WHO_AM_I register, on power up the register should have the value 0x71 */
    // ESP_ERROR_CHECK(mpu9250_register_read(BME280_CHIP_ID_REG, data, 1));
    // ESP_LOGI(TAG, "WHO_AM_I = %X", data[0]);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    /* Demonstrate writing by reseting the MPU9250 */
    // ESP_ERROR_CHECK(mpu9250_register_write_byte(MPU9250_PWR_MGMT_1_REG_ADDR, 1 << MPU9250_RESET_BIT));
    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG, "I2C de-initialized successfully");
}
