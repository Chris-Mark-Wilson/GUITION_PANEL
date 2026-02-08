idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_driver_gpio
        esp_driver_i2c
        # esp_lcd_touch_gsl3680
        # esp_lcd_touch_gt911
        esp_lcd_touch_ft5x06
)
