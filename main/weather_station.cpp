#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define I2C_PORT   I2C_NUM_0
#define SHTC3_ADDR 0x70
#define LCD_ADDR   0x3E

void lcd_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    i2c_master_start(h);
    i2c_master_write_byte(h, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(h, buf, 2, true);
    i2c_master_stop(h);
    i2c_master_cmd_begin(I2C_PORT, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_data(uint8_t data) {
    uint8_t buf[2] = {0x40, data};
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    i2c_master_start(h);
    i2c_master_write_byte(h, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(h, buf, 2, true);
    i2c_master_stop(h);
    i2c_master_cmd_begin(I2C_PORT, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
}

void lcd_print(const char* str) {
    while (*str) lcd_data(*str++);
}

void lcd_init() {
    lcd_cmd(0x38);
    lcd_cmd(0x39);
    lcd_cmd(0x14);
    lcd_cmd(0x70);
    lcd_cmd(0x5E);
    lcd_cmd(0x6A);
    vTaskDelay(pdMS_TO_TICKS(200))
;

    lcd_cmd(0x38);
    lcd_cmd(0x0C);
    lcd_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_cmd(0x06);
}

void shtc3_send(uint8_t b1, uint8_t b2) {
    uint8_t cmd[2] = {b1, b2};
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    i2c_master_start(h);
    i2c_master_write_byte(h, (SHTC3_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(h, cmd, 2, true);
    i2c_master_stop(h);
    i2c_master_cmd_begin(I2C_PORT, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
}

void shtc3_read3(uint8_t* buf) {
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    i2c_master_start(h);
    i2c_master_write_byte(h, (SHTC3_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(h, buf, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(h);
    i2c_master_cmd_begin(I2C_PORT, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
}

int to_humidity(uint8_t* buf) {
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (int)(100.0f * raw / 65535.0f);
}

int to_temperature(uint8_t* buf) {
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (int)(-45.0f + 175.0f * raw / 65535.0f);
}

extern "C" void app_main() {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)7;
    conf.scl_io_num = (gpio_num_t)8;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);

    lcd_init();

    char line[17];
    while (true) {
        // Read humidity
        shtc3_send(0x35, 0x17);
        vTaskDelay(pdMS_TO_TICKS(10));
        shtc3_send(0x5C, 0x24);
        vTaskDelay(pdMS_TO_TICKS(20));
        uint8_t hbuf[3];
        shtc3_read3(hbuf);
        shtc3_send(0xB0, 0x98);

        // Read temperature
        shtc3_send(0x35, 0x17);
        vTaskDelay(pdMS_TO_TICKS(10));
        shtc3_send(0x78, 0x66);
        vTaskDelay(pdMS_TO_TICKS(50));
        uint8_t tbuf[3];
        shtc3_read3(tbuf);
        shtc3_send(0xB0, 0x98);

        int temp = to_temperature(tbuf);
        int hum  = to_humidity(hbuf);

        printf("Temp: %dC  Hum: %d%%\n", temp, hum);

        lcd_cmd(0x80);
        snprintf(line, sizeof(line), "Temp: %dC       ", temp);
        lcd_print(line);
        lcd_cmd(0x80 | 0x40);
        snprintf(line, sizeof(line), "Hum : %d%%      ", hum);
        lcd_print(line);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

