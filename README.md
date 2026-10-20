# Black Magic Probe for ESP32-C6 (Seeed Studio XIAO ESP32C6)

WiFi and USB-capable version of the Black Magic Probe debugger ported to ESP32-C6, optimized for the **Seeed Studio XIAO ESP32C6**.

## XIAO ESP32C6 Pinout & Connections

| Signal | XIAO Pin | ESP32-C6 GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **SWCLK** | **D0** | GPIO 0 | SWD Clock to target |
| **SWDIO** | **D1** | GPIO 1 | SWD Data I/O to target |
| **NRST**  | **D2** | GPIO 2 | Target Reset (active low open-drain) |
| **TRACESWO** | **D3** | GPIO 21 | Trace SWO (optional) |
| **UART TX** | **D6** | GPIO 16 | Hardware UART TX (CLI) |
| **UART RX** | **D7** | GPIO 17 | Hardware UART RX (CLI) |
| **User LED** | Onboard | GPIO 15 | Active LOW onboard yellow LED |
| **USB CDC/JTAG** | USB-C | GPIO 12/13 | Built-in USB Serial & JTAG |

*(Pins can be modified in [components/blackmagic/esp32-platform/platform.h](file:///d:/Programming/Workspaces/Stuff/Esp32_BMP/components/blackmagic/esp32-platform/platform.h))*

---

## Build and Flash

### 1. Environment Setup (PowerShell)
Activate ESP-IDF:
```powershell
. "C:\Users\Brauni\esp\v5.4.1\esp-idf\export.ps1"
```

### 2. Build
```powershell
idf.py build
```

### 3. Flash to XIAO ESP32C6
Connect your XIAO ESP32C6 board via USB-C and run:
```powershell
idf.py -p COMx flash
```
*(Replace `COMx` with your board's COM port, e.g., `COM3`)*

If the board does not automatically reset into bootloader mode, hold down the **BOOT** button (D9 / GPIO 9) while plugging in the USB cable or while pressing reset.

### 4. Monitor Serial Output & CLI
```powershell
idf.py -p COMx monitor
```
The interactive CLI and ESP-IDF logs run directly over the USB-C connection (`USB Serial/JTAG`) as well as hardware UART0 (`D6`/`D7`).

---

## Using Black Magic Probe

### Over WiFi
1. On first boot, the ESP32-C6 provides an Access Point (or connects to your configured network).
2. Connect to the GDB server over TCP port `2345`:
```bash
arm-none-eabi-gdb
(gdb) target extended-remote <esp32-ip>:2345
(gdb) monitor swdp_scan
(gdb) attach 1
```

### Over Web Portal
Open `http://<esp32-ip>` in your web browser to configure WiFi settings, inspect task telemetry, and manage GPIO configurations.
