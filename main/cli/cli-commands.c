#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <esp_system.h>
#include "cli.h"
#include "led.h"
#include "nvs.h"
#include "helpers.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "general.h"
#include "exception.h"
#include "target.h"
#include "target_internal.h"

void cli_device_info(Cli* cli, mstring_t* args);
void cli_factory_reset(Cli* cli, mstring_t* args);
void cli_gpio_get(Cli* cli, mstring_t* args);
void cli_gpio_set(Cli* cli, mstring_t* args);
void cli_led(Cli* cli, mstring_t* args);
void cli_help(Cli* cli, mstring_t* args);
void cli_ping(Cli* cli, mstring_t* args);
void cli_sw_reboot(Cli* cli, mstring_t* args);
void cli_wifi_scan(Cli* cli, mstring_t* args);
void cli_wifi_ap_clients(Cli* cli, mstring_t* args);
void cli_wifi_ip(Cli* cli, mstring_t* args);
void cli_wifi_sta_info(Cli* cli, mstring_t* args);

void cli_config_get(Cli* cli, mstring_t* args);
void cli_config_set_wifi_mode(Cli* cli, mstring_t* args);
void cli_config_set_ap_pass(Cli* cli, mstring_t* args);
void cli_config_set_ap_ssid(Cli* cli, mstring_t* args);
void cli_config_set_sta_pass(Cli* cli, mstring_t* args);
void cli_config_set_sta_ssid(Cli* cli, mstring_t* args);
void cli_config_set_hostname(Cli* cli, mstring_t* args);

void cli_nvs_dump(Cli* cli, mstring_t* args);

void cli_swd_scan(Cli* cli, mstring_t* args);
void cli_swd_reset(Cli* cli, mstring_t* args);
void cli_targets(Cli* cli, mstring_t* args);
void cli_gdb_info(Cli* cli, mstring_t* args);

void cli_attach(Cli* cli, mstring_t* args);
void cli_regs(Cli* cli, mstring_t* args);
void cli_fault(Cli* cli, mstring_t* args);
void cli_halt(Cli* cli, mstring_t* args);
void cli_resume(Cli* cli, mstring_t* args);
void cli_step(Cli* cli, mstring_t* args);
void cli_md(Cli* cli, mstring_t* args);
void cli_flash(Cli* cli, mstring_t* args);
void cli_x(Cli* cli, mstring_t* args);

const CliItem cli_items[] = {
    {
        .name = "!",
        .desc = "alias for device_info",
        .callback = cli_device_info,
    },
    {
        .name = "?",
        .desc = "alias for help",
        .callback = cli_help,
    },
    {
        .name = "config_get",
        .desc = "show current config",
        .callback = cli_config_get,
    },
    {
        .name = "config_set_wifi_mode",
        .desc =
            "set Wi-Fi mode, AP (own access point) or STA (join another network), requires a reboot to apply",
        .callback = cli_config_set_wifi_mode,
    },
    {
        .name = "config_set_ap_pass",
        .desc = "set AP mode password, requires a reboot to apply",
        .callback = cli_config_set_ap_pass,
    },
    {
        .name = "config_set_ap_ssid",
        .desc = "set AP mode SSID, requires a reboot to apply",
        .callback = cli_config_set_ap_ssid,
    },
    {
        .name = "config_set_sta_pass",
        .desc = "set STA mode password, requires a reboot to apply",
        .callback = cli_config_set_sta_pass,
    },
    {
        .name = "config_set_sta_ssid",
        .desc = "set STA mode SSID, requires a reboot to apply",
        .callback = cli_config_set_sta_ssid,
    },
    {
        .name = "config_set_hostname",
        .desc = "set MDNS host name, requires a reboot to apply",
        .callback = cli_config_set_hostname,
    },
    {
        .name = "device_info",
        .desc = "show device info (mac, fw version, chip info, etc)",
        .callback = cli_device_info,
    },
    {
        .name = "factory_reset",
        .desc = "reset config (clears NVS storage)",
        .callback = cli_factory_reset,
    },
    {
        .name = "gpio_get",
        .desc = "get gpio level",
        .callback = cli_gpio_get,
    },
    {
        .name = "gpio_set",
        .desc = "set gpio level",
        .callback = cli_gpio_set,
    },
    {
        .name = "help",
        .desc = "show this help",
        .callback = cli_help,
    },
    {
        .name = "led",
        .desc = "set led color",
        .callback = cli_led,
    },
    {
        .name = "nvs_dump",
        .desc = "show all NVS contents",
        .callback = cli_nvs_dump,
    },
    {
        .name = "ping",
        .desc = "answers pong",
        .callback = cli_ping,
    },
    {
        .name = "reboot",
        .desc = "reboot device",
        .callback = cli_sw_reboot,
    },
    {
        .name = "wifi_ap_clients",
        .desc = "list AP mode clients",
        .callback = cli_wifi_ap_clients,
    },
    {
        .name = "wifi_ip",
        .desc = "show current ip, mask and gateway",
        .callback = cli_wifi_ip,
    },
    {
        .name = "wifi_sta_info",
        .desc = "show STA mode network info",
        .callback = cli_wifi_sta_info,
    },
    {
        .name = "wifi_scan",
        .desc = "scan all available Wi-Fi networks (takes some time)",
        .callback = cli_wifi_scan,
    },
    {
        .name = "swd_scan",
        .desc = "scan SWD bus for target devices (D0=SWCLK, D1=SWDIO)",
        .callback = cli_swd_scan,
    },
    {
        .name = "scan",
        .desc = "alias for swd_scan",
        .callback = cli_swd_scan,
    },
    {
        .name = "swd_reset",
        .desc = "pulse target reset pin NRST (D2) low",
        .callback = cli_swd_reset,
    },
    {
        .name = "reset",
        .desc = "alias for swd_reset",
        .callback = cli_swd_reset,
    },
    {
        .name = "targets",
        .desc = "list detected targets in memory",
        .callback = cli_targets,
    },
    {
        .name = "gdb_info",
        .desc = "show GDB server connection details",
        .callback = cli_gdb_info,
    },
    {
        .name = "gdb",
        .desc = "alias for gdb_info",
        .callback = cli_gdb_info,
    },
    {
        .name = "attach",
        .desc = "attach to target 1 and halt it",
        .callback = cli_attach,
    },
    {
        .name = "regs",
        .desc = "read core registers (PC, LR, SP, xPSR, R0-R12) & decode stack",
        .callback = cli_regs,
    },
    {
        .name = "fault",
        .desc = "read Cortex-M fault and status registers (DHCSR, ICSR, CFSR, HFSR, RCC_CSR)",
        .callback = cli_fault,
    },
    {
        .name = "halt",
        .desc = "halt target execution",
        .callback = cli_halt,
    },
    {
        .name = "resume",
        .desc = "resume target execution",
        .callback = cli_resume,
    },
    {
        .name = "step",
        .desc = "single-step target by one instruction",
        .callback = cli_step,
    },
    {
        .name = "md",
        .desc = "memory dump: md <hex_addr> [count]",
        .callback = cli_md,
    },
    {
        .name = "flash",
        .desc = "read first words of Flash (0x08000000) & decode vector table",
        .callback = cli_flash,
    },
    {
        .name = "vectors",
        .desc = "alias for flash",
        .callback = cli_flash,
    },
    {
        .name = "x",
        .desc = "GDB examine memory: x[/<count><format><size>] <address> (e.g. x/1wx 0x40010000)",
        .callback = cli_x,
    },
};

size_t cli_items_count = COUNT_OF(cli_items);

void cli_help(Cli* cli, mstring_t* args) {
    size_t max_len = 0;
    for(size_t i = 0; i < cli_items_count; i++) {
        if(strlen(cli_items[i].name) > max_len) {
            max_len = strlen(cli_items[i].name);
        }
    }

    max_len += 1;

    for(size_t i = 0; i < cli_items_count; i++) {
        cli_write_str(cli, cli_items[i].name);

        if(cli_items[i].desc != NULL && strlen(cli_items[i].desc)) {
            for(size_t s = 0; s < (max_len - strlen(cli_items[i].name)); s++) {
                cli_write_str(cli, " ");
            }
            cli_write_str(cli, "- ");
            cli_write_str(cli, cli_items[i].desc);
        }

        if((i + 1) < cli_items_count) {
            cli_write_eol(cli);
        }
    }
}

void cli_sw_reboot(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "SW REBOOT");
    cli_write_eol(cli);
    cli_flush(cli);
    esp_restart();
    cli_write_str(cli, "OK");
}

void cli_factory_reset(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "Erasing NVS");
    cli_write_eol(cli);
    cli_flush(cli);
    nvs_erase();
    cli_write_str(cli, "OK");
}

void cli_ping(Cli* cli, mstring_t* args) {
    cli_write_str(cli, "pong");
}

void cli_led_print_usage(Cli* cli) {
    cli_write_str(cli, "Usage: led <red> <green> <blue>");
}

void cli_led(Cli* cli, mstring_t* args) {
    int red;
    int green;
    int blue;
    do {
        if(!cli_args_read_int_and_trim(args, &red)) {
            cli_led_print_usage(cli);
            break;
        }

        if(!cli_args_read_int_and_trim(args, &green)) {
            cli_led_print_usage(cli);
            break;
        }

        if(!cli_args_read_int_and_trim(args, &blue)) {
            cli_led_print_usage(cli);
            break;
        }

        if(red > 255 || red < 0 || green > 255 || green < 0 || blue > 255 || blue < 0) {
            cli_led_print_usage(cli);
            break;
        }

        led_set((uint8_t)red, (uint8_t)green, (uint8_t)blue);
        cli_write_str(cli, "OK");
    } while(false);
}

static void cli_target_display_cb(int i, target_s* t, void* context) {
    Cli* cli = (Cli*)context;
    const char* core = target_core_name(t);
    cli_printf(cli, " [%d] %s (%s) Designer: 0x%03x Part: 0x%03x Attached: %s\r\n",
               i,
               target_driver_name(t),
               core ? core : "unknown",
               target_designer(t),
               target_part_id(t),
               target_attached(t) ? "yes" : "no");
}

void cli_swd_scan(Cli* cli, mstring_t* args) {
    (void)args;
    cli_write_str(cli, "Scanning SWD bus (D0=SWCLK, D1=SWDIO)...\r\n");
    uint32_t devs = 0;
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        devs = adiv5_swdp_scan(0);
    }
    if(e.type != 0) {
        cli_printf(cli, "Scan Exception: %s\r\n", e.msg ? e.msg : "unknown error");
    }
    platform_target_clk_output_enable(false);
    if(devs == 0) {
        platform_nrst_set_val(false);
        cli_write_str(cli, "No SWD devices found!\r\n");
        cli_write_str(cli, "Wiring check for STM32 / ARM:\r\n");
        cli_write_str(cli, "  XIAO D0 (GPIO 0) -> Target SWCLK (STM32 Pin 76 / PA14)\r\n");
        cli_write_str(cli, "  XIAO D1 (GPIO 1) -> Target SWDIO (STM32 Pin 72 / PA13)\r\n");
        cli_write_str(cli, "  XIAO D2 (GPIO 2) -> Target NRST  (STM32 Pin 14 / NRST)\r\n");
        cli_write_str(cli, "  XIAO GND         -> Target GND\r\n");
        cli_write_str(cli, "  Target VDD       -> 3.3V power supply\r\n");
        cli_write_str(cli, "  Target BOOT0     -> GND\r\n");
        return;
    }
    cli_printf(cli, "Success! Found %u target device(s):\r\n", (unsigned int)devs);
    target_foreach(cli_target_display_cb, cli);
}

void cli_swd_reset(Cli* cli, mstring_t* args) {
    (void)args;
    cli_write_str(cli, "Pulsing target reset NRST (D2 / GPIO 2) low for 50ms...\r\n");
    platform_nrst_set_val(true);
    platform_delay(50);
    platform_nrst_set_val(false);
    cli_write_str(cli, "Target reset complete.\r\n");
}

void cli_targets(Cli* cli, mstring_t* args) {
    (void)args;
    cli_write_str(cli, "Targets in memory:\r\n");
    int count = target_foreach(cli_target_display_cb, cli);
    if(count == 0) {
        cli_write_str(cli, "No targets currently listed. Run 'swd_scan' first.\r\n");
    }
}

void cli_gdb_info(Cli* cli, mstring_t* args) {
    (void)args;
    cli_write_str(cli, "Black Magic Probe GDB Server Info:\r\n");
    cli_write_str(cli, "  Server Port: 2345 (TCP)\r\n");
    cli_write_str(cli, "  mDNS Name:   blackmagic.local:2345\r\n");
    cli_write_str(cli, "  Connect in arm-none-eabi-gdb:\r\n");
    cli_write_str(cli, "    target extended-remote blackmagic.local:2345\r\n");
    cli_write_str(cli, "    monitor swdp_scan\r\n");
    cli_write_str(cli, "    attach 1\r\n");
    cli_write_str(cli, "    load\r\n");
    cli_write_str(cli, "    run\r\n");
}

static void cli_tc_destroy(target_controller_s *tc, target_s *t) {
    (void)tc;
    (void)t;
}

static target_controller_s cli_tc = {
    .destroy_callback = cli_tc_destroy,
};

static target_s* cli_get_target(Cli* cli) {
    if(!target_list) {
        cli_write_str(cli, "No target found in memory. Scanning SWD first...\r\n");
        volatile exception_s e;
        TRY_CATCH(e, EXCEPTION_ALL) {
            adiv5_swdp_scan(0);
        }
        if(!target_list) {
            cli_write_str(cli, "Error: No SWD target found!\r\n");
            return NULL;
        }
    }
    if(!target_attached(target_list)) {
        cli_write_str(cli, "Attaching to target 1...\r\n");
        volatile exception_s e;
        TRY_CATCH(e, EXCEPTION_ALL) {
            target_attach(target_list, &cli_tc);
        }
        if(!target_attached(target_list)) {
            cli_write_str(cli, "Error: Failed to attach to target.\r\n");
            return NULL;
        }
    }
    return target_list;
}

void cli_attach(Cli* cli, mstring_t* args) {
    target_s* t = cli_get_target(cli);
    if(!t) return;
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_halt_request(t);
    }
    cli_printf(cli, "Attached to %s (%s). Target halted.\r\n",
               target_driver_name(t),
               target_core_name(t) ? target_core_name(t) : "");
    cli_regs(cli, args);
}

void cli_regs(Cli* cli, mstring_t* args) {
    (void)args;
    target_s* t = cli_get_target(cli);
    if(!t) return;

    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_halt_request(t);
        target_addr_t watch = 0;
        for(int i = 0; i < 10; i++) {
            if(target_halt_poll(t, &watch) != TARGET_HALT_RUNNING) break;
            platform_delay(10);
        }
    }

    uint32_t regs[24] = {0};
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_regs_read(t, regs);
    }
    if(e.type != 0) {
        cli_printf(cli, "Exception reading registers: %s\r\n", e.msg ? e.msg : "unknown");
        return;
    }

    uint32_t r0   = regs[0];
    uint32_t r1   = regs[1];
    uint32_t r2   = regs[2];
    uint32_t r3   = regs[3];
    uint32_t r4   = regs[4];
    uint32_t r5   = regs[5];
    uint32_t r6   = regs[6];
    uint32_t r7   = regs[7];
    uint32_t r8   = regs[8];
    uint32_t r9   = regs[9];
    uint32_t r10  = regs[10];
    uint32_t r11  = regs[11];
    uint32_t r12  = regs[12];
    uint32_t sp   = regs[13];
    uint32_t lr   = regs[14];
    uint32_t pc   = regs[15];
    uint32_t xpsr = regs[16];
    uint32_t msp  = regs[17];
    uint32_t psp  = regs[18];

    cli_printf(cli, "=================== TARGET REGISTERS ===================\r\n");
    cli_printf(cli, " R0:  0x%08" PRIx32 "   R4:  0x%08" PRIx32 "   R8:  0x%08" PRIx32 "   R12: 0x%08" PRIx32 "\r\n", r0, r4, r8, r12);
    cli_printf(cli, " R1:  0x%08" PRIx32 "   R5:  0x%08" PRIx32 "   R9:  0x%08" PRIx32 "   SP:  0x%08" PRIx32 "\r\n", r1, r5, r9, sp);
    cli_printf(cli, " R2:  0x%08" PRIx32 "   R6:  0x%08" PRIx32 "   R10: 0x%08" PRIx32 "   LR:  0x%08" PRIx32 "\r\n", r2, r6, r10, lr);
    cli_printf(cli, " R3:  0x%08" PRIx32 "   R7:  0x%08" PRIx32 "   R11: 0x%08" PRIx32 "   PC:  0x%08" PRIx32 "\r\n", r3, r7, r11, pc);
    cli_printf(cli, " xPSR: 0x%08" PRIx32 "  MSP: 0x%08" PRIx32 "   PSP: 0x%08" PRIx32 "\r\n", xpsr, msp, psp);
    cli_printf(cli, "--------------------------------------------------------\r\n");

    if(pc >= 0x08000000 && pc <= 0x0802FFFF) {
        cli_printf(cli, " PC Location: 0x%08" PRIx32 " (Flash memory - user application)\r\n", pc);
    } else if(pc >= 0x20000000 && pc <= 0x20004FFF) {
        cli_printf(cli, " PC Location: 0x%08" PRIx32 " (SRAM - executing from RAM)\r\n", pc);
    } else if(pc >= 0x1FF00000 && pc <= 0x1FF01FFF) {
        cli_printf(cli, " PC Location: 0x%08" PRIx32 " (System ROM Bootloader - check BOOT0!)\r\n", pc);
    } else if(pc == 0x00000000 || pc == 0xFFFFFFFF) {
        cli_printf(cli, " PC Location: 0x%08" PRIx32 " (INVALID! Target unprogrammed or corrupted)\r\n", pc);
    } else {
        cli_printf(cli, " PC Location: 0x%08" PRIx32 "\r\n", pc);
    }

    uint32_t vect = xpsr & 0x1FF;
    if(vect == 0) {
        cli_printf(cli, " Execution Mode: Thread Mode (normal application code)\r\n");
    } else if(vect == 2) {
        cli_printf(cli, " Execution Mode: NMI (Non-Maskable Interrupt)\r\n");
    } else if(vect == 3) {
        cli_printf(cli, " Execution Mode: HARDFAULT! (CPU crashed into HardFault_Handler)\r\n");
    } else if(vect == 11) {
        cli_printf(cli, " Execution Mode: SVCall Handler\r\n");
    } else if(vect == 14) {
        cli_printf(cli, " Execution Mode: PendSV Handler (RTOS scheduler)\r\n");
    } else if(vect == 15) {
        cli_printf(cli, " Execution Mode: SysTick Handler\r\n");
    } else if(vect >= 16) {
        cli_printf(cli, " Execution Mode: Peripheral IRQ #%" PRIu32 " Handler\r\n", vect - 16);
    }

    if((lr & 0xFFFFFF00) == 0xFFFFFF00) {
        cli_printf(cli, " Handler Return (LR): 0x%08" PRIx32 "\r\n", lr);
        uint32_t active_sp = (lr & 0x4) ? psp : msp;
        cli_printf(cli, " Stack Frame before fault (at SP 0x%08" PRIx32 "):\r\n", active_sp);
        TRY_CATCH(e, EXCEPTION_ALL) {
            uint32_t stacked_r0   = target_mem_read32(t, active_sp);
            uint32_t stacked_r1   = target_mem_read32(t, active_sp + 4);
            uint32_t stacked_r2   = target_mem_read32(t, active_sp + 8);
            uint32_t stacked_r3   = target_mem_read32(t, active_sp + 12);
            uint32_t stacked_r12  = target_mem_read32(t, active_sp + 16);
            uint32_t stacked_lr   = target_mem_read32(t, active_sp + 20);
            uint32_t stacked_pc   = target_mem_read32(t, active_sp + 24);
            uint32_t stacked_xpsr = target_mem_read32(t, active_sp + 28);
            cli_printf(cli, "   >>> FAULTING INSTRUCTION (PC): 0x%08" PRIx32 " <<<\r\n", stacked_pc);
            cli_printf(cli, "   >>> CALLER ADDRESS      (LR): 0x%08" PRIx32 " <<<\r\n", stacked_lr);
            cli_printf(cli, "   Stacked R0:  0x%08" PRIx32 "  R1: 0x%08" PRIx32 "\r\n", stacked_r0, stacked_r1);
            cli_printf(cli, "   Stacked R2:  0x%08" PRIx32 "  R3: 0x%08" PRIx32 "\r\n", stacked_r2, stacked_r3);
            cli_printf(cli, "   Stacked R12: 0x%08" PRIx32 "  xPSR: 0x%08" PRIx32 "\r\n", stacked_r12, stacked_xpsr);
        }
    }
}

void cli_fault(Cli* cli, mstring_t* args) {
    (void)args;
    target_s* t = cli_get_target(cli);
    if(!t) return;

    cli_printf(cli, "================ SYSTEM / FAULT STATUS ================\r\n");
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        uint32_t dhcsr   = target_mem_read32(t, 0xE000EDF0);
        uint32_t icsr    = target_mem_read32(t, 0xE000ED04);
        uint32_t shcsr   = target_mem_read32(t, 0xE000ED24);
        uint32_t cfsr    = target_mem_read32(t, 0xE000ED28);
        uint32_t hfsr    = target_mem_read32(t, 0xE000ED2C);
        uint32_t dfsr    = target_mem_read32(t, 0xE000ED30);
        uint32_t rcc_csr = target_mem_read32(t, 0x40021050);

        cli_printf(cli, " DHCSR:   0x%08" PRIx32 " (Halted: %s, Sleep: %s, Lockup: %s)\r\n",
                   dhcsr,
                   (dhcsr & (1 << 17)) ? "YES" : "NO",
                   (dhcsr & (1 << 18)) ? "YES" : "NO",
                   (dhcsr & (1 << 19)) ? "YES (LOCKUP!)" : "NO");
        cli_printf(cli, " ICSR:    0x%08" PRIx32 " (VECTACTIVE: %" PRIu32 ", VECTPENDING: %" PRIu32 ")\r\n",
                   icsr, icsr & 0x1FF, (icsr >> 12) & 0x1FF);
        cli_printf(cli, " SHCSR:   0x%08" PRIx32 "\r\n", shcsr);
        cli_printf(cli, " CFSR:    0x%08" PRIx32 "\r\n", cfsr);
        cli_printf(cli, " HFSR:    0x%08" PRIx32 " %s\r\n", hfsr, (hfsr & (1 << 30)) ? "[FORCED HardFault]" : "");
        cli_printf(cli, " DFSR:    0x%08" PRIx32 "\r\n", dfsr);
        cli_printf(cli, " RCC_CSR: 0x%08" PRIx32 " (Reset flags:\r\n", rcc_csr);
        if(rcc_csr & (1 << 31)) cli_printf(cli, "   - Low-power reset (LPWRRSTF)\r\n");
        if(rcc_csr & (1 << 30)) cli_printf(cli, "   - Window Watchdog reset (WWDGRSTF)\r\n");
        if(rcc_csr & (1 << 29)) cli_printf(cli, "   - Independent Watchdog reset (IWDGRSTF)\r\n");
        if(rcc_csr & (1 << 28)) cli_printf(cli, "   - Software reset (SFTRSTF)\r\n");
        if(rcc_csr & (1 << 27)) cli_printf(cli, "   - Power-on reset (PORRSTF)\r\n");
        if(rcc_csr & (1 << 26)) cli_printf(cli, "   - NRST pin reset (PINRSTF)\r\n");
        cli_printf(cli, " )\r\n");
    }
    if(e.type != 0) {
        cli_printf(cli, "Exception reading fault status: %s\r\n", e.msg ? e.msg : "unknown");
    }
}

void cli_halt(Cli* cli, mstring_t* args) {
    (void)args;
    target_s* t = cli_get_target(cli);
    if(!t) return;
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_halt_request(t);
    }
    cli_write_str(cli, "Target halt request sent.\r\n");
    uint32_t regs[24] = {0};
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_regs_read(t, regs);
    }
    cli_printf(cli, "Target halted at PC: 0x%08" PRIx32 "\r\n", regs[15]);
}

void cli_resume(Cli* cli, mstring_t* args) {
    (void)args;
    target_s* t = cli_get_target(cli);
    if(!t) return;
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_halt_resume(t, false);
    }
    cli_write_str(cli, "Target resumed.\r\n");
}

void cli_step(Cli* cli, mstring_t* args) {
    (void)args;
    target_s* t = cli_get_target(cli);
    if(!t) return;
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_halt_resume(t, true);
    }
    uint32_t regs[24] = {0};
    TRY_CATCH(e, EXCEPTION_ALL) {
        target_regs_read(t, regs);
    }
    cli_printf(cli, "Stepped to PC: 0x%08" PRIx32 "\r\n", regs[15]);
}

void cli_md(Cli* cli, mstring_t* args) {
    target_s* t = cli_get_target(cli);
    if(!t) return;

    mstring_t* arg1 = mstring_alloc();
    mstring_t* arg2 = mstring_alloc();
    size_t ws = mstring_search_char(args, ' ', 0);
    if(ws == STRING_FAILURE) {
        mstring_set(arg1, mstring_get_cstr(args));
    } else {
        mstring_set_n(arg1, args, 0, ws);
        mstring_set_n(arg2, args, ws, mstring_size(args));
        mstring_strim(arg2, " \t\r\n");
    }
    mstring_strim(arg1, " \t\r\n");

    if(mstring_size(arg1) == 0) {
        cli_write_str(cli, "Usage: md <hex_addr> [count]\r\nExample: md 0x20000000 8\r\n");
        mstring_free(arg1);
        mstring_free(arg2);
        return;
    }

    uint32_t addr = strtoul(mstring_get_cstr(arg1), NULL, 0);
    uint32_t count = 4;
    if(mstring_size(arg2) > 0) {
        count = strtoul(mstring_get_cstr(arg2), NULL, 0);
        if(count > 64) count = 64;
        if(count == 0) count = 4;
    }
    mstring_free(arg1);
    mstring_free(arg2);

    cli_printf(cli, "Memory at 0x%08" PRIx32 " (%" PRIu32 " words):\r\n", addr, count);
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        for(uint32_t i = 0; i < count; i += 4) {
            cli_printf(cli, "  0x%08" PRIx32 ": ", addr + i * 4);
            for(uint32_t j = 0; j < 4 && (i + j) < count; ++j) {
                uint32_t val = target_mem_read32(t, addr + (i + j) * 4);
                cli_printf(cli, "0x%08" PRIx32 "  ", val);
            }
            cli_write_eol(cli);
        }
    }
    if(e.type != 0) {
        cli_printf(cli, "Exception during memory read: %s\r\n", e.msg ? e.msg : "unknown");
    }
}

void cli_flash(Cli* cli, mstring_t* args) {
    (void)args;
    target_s* t = cli_get_target(cli);
    if(!t) return;

    cli_printf(cli, "================ FLASH VECTOR TABLE (0x08000000) ================\r\n");
    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        uint32_t sp        = target_mem_read32(t, 0x08000000);
        uint32_t reset     = target_mem_read32(t, 0x08000004);
        uint32_t nmi       = target_mem_read32(t, 0x08000008);
        uint32_t hardfault = target_mem_read32(t, 0x0800000C);
        uint32_t rsvd1     = target_mem_read32(t, 0x08000010);
        uint32_t rsvd2     = target_mem_read32(t, 0x08000014);
        uint32_t rsvd3     = target_mem_read32(t, 0x08000018);
        uint32_t rsvd4     = target_mem_read32(t, 0x0800001C);
        uint32_t svcall    = target_mem_read32(t, 0x0800002C);
        uint32_t pendsv    = target_mem_read32(t, 0x08000038);
        uint32_t systick   = target_mem_read32(t, 0x0800003C);

        cli_printf(cli, " [0x08000000] Initial SP (MSP):  0x%08" PRIx32 "\r\n", sp);
        cli_printf(cli, " [0x08000004] Reset Handler:     0x%08" PRIx32 "\r\n", reset);
        cli_printf(cli, " [0x08000008] NMI Handler:       0x%08" PRIx32 "\r\n", nmi);
        cli_printf(cli, " [0x0800000C] HardFault Handler: 0x%08" PRIx32 "\r\n", hardfault);
        cli_printf(cli, " [0x08000010] Reserved/Checksum: 0x%08" PRIx32 " 0x%08" PRIx32 " 0x%08" PRIx32 " 0x%08" PRIx32 "\r\n",
                   rsvd1, rsvd2, rsvd3, rsvd4);
        cli_printf(cli, " [0x0800002C] SVCall Handler:    0x%08" PRIx32 "\r\n", svcall);
        cli_printf(cli, " [0x08000038] PendSV Handler:    0x%08" PRIx32 "\r\n", pendsv);
        cli_printf(cli, " [0x0800003C] SysTick Handler:   0x%08" PRIx32 "\r\n", systick);

        cli_printf(cli, "------------------------------------------------------------------\r\n");
        if(sp == 0xFFFFFFFF && reset == 0xFFFFFFFF) {
            cli_printf(cli, ">>> FLASH STATUS: BLANK / ERASED (all 0xFFFFFFFF)!\r\n");
            cli_printf(cli, "    No firmware is currently flashed on the STM32.\r\n");
        } else if(sp == 0x00000000 && reset == 0x00000000) {
            cli_printf(cli, ">>> FLASH STATUS: ALL ZERO (0x00000000)!\r\n");
            cli_printf(cli, "    Check Readout Protection (RDP level 1) or Option Bytes.\r\n");
        } else {
            cli_printf(cli, ">>> FLASH STATUS: Firmware present! Reset vector -> 0x%08" PRIx32 "\r\n", reset & ~1);
            if((sp & 0xFFFE0000) != 0x20000000) {
                cli_printf(cli, "    NOTE: Initial SP (0x%08" PRIx32 ") is outside standard SRAM range (0x20000000)!\r\n", sp);
            }
        }
    }
    if(e.type != 0) {
        cli_printf(cli, "Exception reading flash: %s\r\n", e.msg ? e.msg : "unknown");
    }
}

static void print_formatted_val(Cli* cli, uint64_t val, char format, char size) {
    switch(format) {
    case 'x':
    default:
        if(size == 'b') {
            cli_printf(cli, "0x%02x", (uint8_t)val);
        } else if(size == 'h') {
            cli_printf(cli, "0x%04x", (uint16_t)val);
        } else if(size == 'g') {
            cli_printf(cli, "0x%016" PRIx64, val);
        } else {
            cli_printf(cli, "0x%08" PRIx32, (uint32_t)val);
        }
        break;
    case 'd':
        if(size == 'b') {
            cli_printf(cli, "%d", (int8_t)val);
        } else if(size == 'h') {
            cli_printf(cli, "%d", (int16_t)val);
        } else if(size == 'g') {
            cli_printf(cli, "%" PRId64, (int64_t)val);
        } else {
            cli_printf(cli, "%" PRId32, (int32_t)val);
        }
        break;
    case 'u':
        if(size == 'b') {
            cli_printf(cli, "%u", (uint8_t)val);
        } else if(size == 'h') {
            cli_printf(cli, "%u", (uint16_t)val);
        } else if(size == 'g') {
            cli_printf(cli, "%" PRIu64, val);
        } else {
            cli_printf(cli, "%" PRIu32, (uint32_t)val);
        }
        break;
    case 't':
        if(size == 'b') {
            for(int b = 7; b >= 0; b--) cli_printf(cli, "%c", (val & (1 << b)) ? '1' : '0');
        } else if(size == 'h') {
            for(int b = 15; b >= 0; b--) cli_printf(cli, "%c", (val & (1 << b)) ? '1' : '0');
        } else {
            for(int b = 31; b >= 0; b--) cli_printf(cli, "%c", (val & (1ULL << b)) ? '1' : '0');
        }
        break;
    case 'c':
        if(val >= 32 && val <= 126) {
            cli_printf(cli, "'%c'", (char)val);
        } else {
            cli_printf(cli, "'\\x%02x'", (uint8_t)val);
        }
        break;
    case 'i':
        if(size == 'h') {
            cli_printf(cli, "0x%04x", (uint16_t)val);
        } else {
            cli_printf(cli, "0x%08" PRIx32, (uint32_t)val);
        }
        break;
    }
}

void cli_x(Cli* cli, mstring_t* args) {
    target_s* t = cli_get_target(cli);
    if(!t) return;

    const char* str = mstring_get_cstr(args);
    while(*str == ' ' || *str == '\t') str++;

    volatile uint32_t count = 1;
    volatile char format = 'x';
    volatile char size = 'w';

    if(*str == '/') {
        str++;
        if(*str >= '0' && *str <= '9') {
            count = strtoul(str, (char**)&str, 10);
            if(count == 0) count = 1;
            if(count > 256) count = 256;
        }
        while(*str && *str != ' ' && *str != '\t') {
            char c = *str++;
            if(c == 'b' || c == 'h' || c == 'w' || c == 'g') {
                size = c;
            } else if(c == 'x' || c == 'd' || c == 'u' || c == 'o' || c == 't' || c == 'c' || c == 's' || c == 'i') {
                format = c;
            }
        }
    }

    while(*str == ' ' || *str == '\t') str++;

    if(*str == '\0') {
        cli_write_str(cli, "Usage: x[/<count><format><size>] <address>\r\n");
        cli_write_str(cli, "Examples:\r\n");
        cli_write_str(cli, "  x/1wx 0x40010000\r\n");
        cli_write_str(cli, "  x/4xw 0x08000000\r\n");
        cli_write_str(cli, "  x/16xb 0x20000000\r\n");
        cli_write_str(cli, "  x/s 0x08001000\r\n");
        cli_write_str(cli, "  x 0x08000000\r\n");
        return;
    }

    volatile uint32_t addr = strtoul(str, NULL, 0);

    if(format == 's') {
        cli_printf(cli, "0x%08" PRIx32 ": \"", addr);
        volatile exception_s e;
        TRY_CATCH(e, EXCEPTION_ALL) {
            for(uint32_t i = 0; i < 256; i++) {
                uint8_t ch = target_mem_read8(t, addr + i);
                if(ch == 0) break;
                if(ch >= 32 && ch <= 126) {
                    cli_printf(cli, "%c", ch);
                } else if(ch == '\n') {
                    cli_printf(cli, "\\n");
                } else if(ch == '\r') {
                    cli_printf(cli, "\\r");
                } else if(ch == '\t') {
                    cli_printf(cli, "\\t");
                } else {
                    cli_printf(cli, "\\x%02x", ch);
                }
            }
            cli_printf(cli, "\"\r\n");
        }
        if(e.type != 0) {
            cli_printf(cli, "\"\r\nException during string read: %s\r\n", e.msg ? e.msg : "unknown");
        }
        return;
    }

    volatile uint32_t step = 4;
    volatile uint32_t items_per_line = 4;
    if(size == 'b') {
        step = 1;
        items_per_line = 16;
    } else if(size == 'h') {
        step = 2;
        items_per_line = 8;
    } else if(size == 'g') {
        step = 8;
        items_per_line = 2;
    }

    volatile exception_s e;
    TRY_CATCH(e, EXCEPTION_ALL) {
        for(uint32_t i = 0; i < count; i++) {
            if(i % items_per_line == 0) {
                if(i > 0) cli_write_eol(cli);
                cli_printf(cli, "0x%08" PRIx32 ":  ", addr + i * step);
            }
            uint64_t val = 0;
            if(size == 'b') {
                val = target_mem_read8(t, addr + i * step);
            } else if(size == 'h') {
                val = target_mem_read16(t, addr + i * step);
            } else if(size == 'g') {
                uint32_t low = target_mem_read32(t, addr + i * step);
                uint32_t high = target_mem_read32(t, addr + i * step + 4);
                val = ((uint64_t)high << 32) | low;
            } else {
                val = target_mem_read32(t, addr + i * step);
            }
            print_formatted_val(cli, val, format, size);
            cli_printf(cli, "  ");
        }
        cli_write_eol(cli);
    }
    if(e.type != 0) {
        cli_printf(cli, "\r\nException during memory read: %s\r\n", e.msg ? e.msg : "unknown");
    }
}




