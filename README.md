# AirNet CO2 - Low-Power CO2 Monitoring Device

AirNet CO2 is a low-power, relatively affordable sensing device
featuring an ePaper display, designed to monitor CO2 concentration,
temperature, and relative humidity.

> [!WARNING]
> This is a work in progress.

The device is designed for low-power battery operation. It is based on
an ultra-low-power STM32U031 microcontroller. CO2 concentration is
measured using the STCC4 sensor from Sensirion, temperature
and humidity are provided by the SHT40. The display is a 1.54-inch
ePaper (eInk) panel with a 27 mm square active area. Three status LEDs
(green, yellow, and red) indicate CO2 levels at a glance. The device
is powered by a standard 14500 Li-ion battery, rechargeable via USB-C
using the BQ24074 from TI, with a regulated 3.3 V
supply provided by the TPS63900.

## Images

![Top kicad](/images/airnet_co2_rev0_1_top.png)

![Bottom kicad](/images/airnet_co2_rev0_1_bottom.png)

![Top irl](/images/airnet_co2_rev01_irl.jpg)
