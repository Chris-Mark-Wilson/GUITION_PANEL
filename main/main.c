#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

static const char *TAG = "BL_DISCOVERY";

/*
  Candidate GPIOs:
  - We try a broad but reasonable range.
  - If nothing works, we can widen it.
*/
static const int candidates[] = {
   23
};

static void bl_drive_gpio(int gpio, int level)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    // Some GPIO numbers may be invalid on a given chip/package; ignore errors.
    if (gpio_config(&io) != ESP_OK) return;
    gpio_set_level(gpio, level);
}

static void bl_pwm_gpio(int gpio, int percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    // LEDC timer
    ledc_timer_config_t t = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 20000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&t);

    // LEDC channel on the GPIO
    ledc_channel_config_t c = {
        .gpio_num = gpio,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    if (ledc_channel_config(&c) != ESP_OK) return;

    uint32_t max = (1 << 10) - 1;
    uint32_t duty = (max * (uint32_t)percent) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void all_off(void)
{
    // best-effort: drive all candidates low (safe after boot)
    for (int i = 0; i < (int)(sizeof(candidates)/sizeof(candidates[0])); i++) {
        bl_drive_gpio(candidates[i], 0);
    }
}

void app_main(void)
 

 
{

    ESP_LOGI(TAG, "Backlight discovery starting.");
    ESP_LOGI(TAG, "Watch the panel. When it lights up, note the GPIO in the log.");

    vTaskDelay(pdMS_TO_TICKS(500));
    all_off();

    // Phase 1: Digital enable sweep
    ESP_LOGI(TAG, "PHASE 1: Digital enable sweep (set GPIO high for 1.5s).");
    for (int i = 0; i < (int)(sizeof(candidates)/sizeof(candidates[0])); i++) {
        int gpio = candidates[i];
        ESP_LOGI(TAG, "DIGITAL HIGH: GPIO %d", gpio);
        bl_drive_gpio(gpio, 1);
        vTaskDelay(pdMS_TO_TICKS(1500));
        bl_drive_gpio(gpio, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // Phase 2: PWM sweep
    ESP_LOGI(TAG, "PHASE 2: PWM sweep (100%% duty @20kHz for 2s).");
    for (int i = 0; i < (int)(sizeof(candidates)/sizeof(candidates[0])); i++) {
        int gpio = candidates[i];
        ESP_LOGI(TAG, "PWM 100%%: GPIO %d", gpio);
        bl_pwm_gpio(gpio, 100);
        vTaskDelay(pdMS_TO_TICKS(2000));
        bl_pwm_gpio(gpio, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    ESP_LOGW(TAG, "Finished both sweeps. If nothing lit, we likely need a pinout (or BL is via expander).");
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
