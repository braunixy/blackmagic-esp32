#include <string.h>
#include "led.h"
#include "cli/cli.h"
#include "driver/uart.h"
#include <driver/usb_serial_jtag.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// On Seeed Studio XIAO ESP32C6:
// D6 is GPIO 16 (UART TX)
// D7 is GPIO 17 (UART RX)
#define CLI_UART_PORT_NUM UART_NUM_0
#define CLI_UART_TXD_PIN (16)
#define CLI_UART_RXD_PIN (17)
#define CLI_UART_BAUD_RATE (115200)
#define CLI_UART_TX_BUF_SIZE (UART_FIFO_LEN * 2)
#define CLI_UART_RX_BUF_SIZE (UART_FIFO_LEN * 4)

static Cli* cli_uart;
static uint8_t uart_tx_buffer[CLI_UART_TX_BUF_SIZE];
static size_t uart_tx_index = 0;

static void cli_uart_write(const uint8_t* data, size_t data_size, void* context);
static void cli_uart_flush(void* context);

static void cli_uart_rx_task(void* pvParameters);

void cli_uart_init() {
    cli_uart = cli_init();
    cli_set_write_cb(cli_uart, cli_uart_write);
    cli_set_flush_cb(cli_uart, cli_uart_flush);

    // 1. Install USB Serial/JTAG driver (for USB-C connection)
    usb_serial_jtag_driver_config_t jtag_config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    jtag_config.rx_buffer_size = CLI_UART_RX_BUF_SIZE;
    jtag_config.tx_buffer_size = CLI_UART_TX_BUF_SIZE;
    usb_serial_jtag_driver_install(&jtag_config);

    // 2. Install hardware UART driver on D6 (TX) / D7 (RX)
    uart_config_t uart_config = {
        .baud_rate = CLI_UART_BAUD_RATE,
        .stop_bits = UART_STOP_BITS_1,
        .parity = UART_PARITY_DISABLE,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    uart_driver_install(
        CLI_UART_PORT_NUM, CLI_UART_RX_BUF_SIZE, CLI_UART_TX_BUF_SIZE, 0, NULL, intr_alloc_flags);
    uart_set_pin(CLI_UART_PORT_NUM, CLI_UART_TXD_PIN, CLI_UART_RXD_PIN, -1, -1);
    uart_param_config(CLI_UART_PORT_NUM, &uart_config);

    xTaskCreatePinnedToCore(cli_uart_rx_task, "cli_rx", 4096, NULL, 5, NULL, tskNO_AFFINITY);

    cli_force_motd(cli_uart);
}

static void cli_uart_write(const uint8_t* data, size_t data_size, void* context) {
    for(size_t i = 0; i < data_size; i++) {
        uart_tx_buffer[uart_tx_index] = data[i];
        uart_tx_index++;

        if(uart_tx_index == CLI_UART_TX_BUF_SIZE) {
            cli_uart_flush(NULL);
        }
    }
}

static void cli_uart_flush(void* context) {
    if(uart_tx_index > 0) {
        uart_write_bytes(CLI_UART_PORT_NUM, uart_tx_buffer, uart_tx_index);
        uart_wait_tx_done(CLI_UART_PORT_NUM, 10 / portTICK_PERIOD_MS);

        if(usb_serial_jtag_is_driver_installed()) {
            usb_serial_jtag_write_bytes(uart_tx_buffer, uart_tx_index, 10 / portTICK_PERIOD_MS);
            usb_serial_jtag_wait_tx_done(10 / portTICK_PERIOD_MS);
        }
    }

    uart_tx_index = 0;
}

static void cli_uart_rx_task(void* pvParameters) {
    uint8_t data[CLI_UART_RX_BUF_SIZE];

    while(1) {
        if(usb_serial_jtag_is_driver_installed()) {
            int len = usb_serial_jtag_read_bytes(data, sizeof(data), 10 / portTICK_PERIOD_MS);
            for(int i = 0; i < len; i++) {
                cli_handle_char(cli_uart, data[i]);
            }
        }

        int len = uart_read_bytes(
            CLI_UART_PORT_NUM, data, CLI_UART_RX_BUF_SIZE, 10 / portTICK_PERIOD_MS);

        for(int i = 0; i < len; i++) {
            cli_handle_char(cli_uart, data[i]);
        }
    }
}
