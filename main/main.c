#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"

// If you don't have these components in your project yet,
// comment the #includes and hard-code the addresses instead.
#include "esp_lcd_touch_gsl3680.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_touch_ft5x06.h"

static const char *TAG = "TP_PROBE";

// From pins_config.h
#define TP_I2C_SDA   7
#define TP_I2C_SCL   8
#define TP_RST       22
#define TP_INT       21

static void touch_reset_pulse(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << TP_RST,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));

    // Active-low reset, typical for these chips
    gpio_set_level(TP_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TP_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void touch_int_input(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << TP_INT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,  // safe, usually open-drain
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
}

static void probe_touch_addrs(i2c_master_bus_handle_t bus)
{
    esp_err_t r;

    r = i2c_master_probe(bus, ESP_LCD_TOUCH_IO_I2C_GSL3680_ADDRESS, 100);
    ESP_LOGI(TAG, "Probe GSL3680 0x%02X: %s",
             ESP_LCD_TOUCH_IO_I2C_GSL3680_ADDRESS, esp_err_to_name(r));

    r = i2c_master_probe(bus, ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS, 100);
    ESP_LOGI(TAG, "Probe GT911   0x%02X: %s",
             ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS, esp_err_to_name(r));

    r = i2c_master_probe(bus, ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP, 100);
    ESP_LOGI(TAG, "Probe GT911B  0x%02X: %s",
             ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP, esp_err_to_name(r));

    r = i2c_master_probe(bus, ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS, 100);
    ESP_LOGI(TAG, "Probe FT5x06  0x%02X: %s",
             ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS, esp_err_to_name(r));
}

void app_main(void)
{
    // Keep the i2c spam down
    esp_log_level_set("i2c.master", ESP_LOG_WARN);

    ESP_LOGI(TAG, "TP probe starting (SDA=%d SCL=%d RST=%d INT=%d)",
             TP_I2C_SDA, TP_I2C_SCL, TP_RST, TP_INT);

    touch_reset_pulse();
    touch_int_input();

    i2c_master_bus_handle_t bus = NULL;
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,              // try port 0 first
        .sda_io_num = TP_I2C_SDA,
        .scl_io_num = TP_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .flags.enable_internal_pullup = 0,  // external pull-ups on the board
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    probe_touch_addrs(bus);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
