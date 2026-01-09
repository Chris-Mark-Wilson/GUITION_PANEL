#include "driver/gpio.h"
#include "esp_err.h"

#define BL_EN_GPIO 23

esp_err_t backlight_init(void) {
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BL_EN_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    gpio_set_level(BL_EN_GPIO, 0);
    return ESP_OK;
}

void backlight_on(void)  { gpio_set_level(BL_EN_GPIO, 1); }
void backlight_off(void) { gpio_set_level(BL_EN_GPIO, 0); }
