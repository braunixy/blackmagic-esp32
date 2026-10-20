#include "led.h"
#include <driver/ledc.h>
#include <esp_log.h>
#include <esp_err.h>

// On Seeed Studio XIAO ESP32C6, user LED is on GPIO 15 (Active LOW)
#define XIAO_LED_PIN (15)

#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define TAG "led"
#define LED_PWM_MAX_VAL 256U

static uint8_t cur_r = 0, cur_g = 0, cur_b = 0;

static void update_led_hw(void) {
    uint8_t max_val = cur_r;
    if(cur_g > max_val) max_val = cur_g;
    if(cur_b > max_val) max_val = cur_b;

    // Active low: max_val == 0 -> duty = 256 (100% HIGH = OFF)
    // max_val == 255 -> duty = 0 (0% = LOW = fully ON)
    uint32_t duty = LED_PWM_MAX_VAL - ((uint32_t)max_val * LED_PWM_MAX_VAL / 255);
    if(duty > LED_PWM_MAX_VAL) duty = LED_PWM_MAX_VAL;

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
}

void led_init() {
    ESP_LOGI(TAG, "init (XIAO ESP32C6 onboard LED on GPIO %d)", XIAO_LED_PIN);
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = XIAO_LED_PIN,
        .duty = LED_PWM_MAX_VAL, // Set duty to 100% (LED OFF)
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ESP_LOGI(TAG, "init done");
}

void led_set(uint8_t red, uint8_t green, uint8_t blue) {
    cur_r = red;
    cur_g = green;
    cur_b = blue;
    update_led_hw();
}

void led_set_red(uint8_t value) {
    cur_r = value;
    update_led_hw();
}

void led_set_green(uint8_t value) {
    cur_g = value;
    update_led_hw();
}

void led_set_blue(uint8_t value) {
    cur_b = value;
    update_led_hw();
}
