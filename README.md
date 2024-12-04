# Conncted Weather Station

Code for 6tron module to publish weather data in Adafruit platforme.
![image](https://github.com/user-attachments/assets/811d5fde-ee28-47b3-b844-5a8ce48d2a57)

## Requirements

Docker
Usbidp / WSL

### Hardware requirements

The following boards are required:
- [Zest_Core_STM32L4A6RG](https://6tron.io/zest_core/zest_core_stm32l4a6rg_3_0_0/)
- [Zest_Adapter_Shield](https://6tron.io/zest/zest_adapter_shield_1_0_0/)
- [L-Tek 6LoWPAN Arduino Shield](https://www.l-tek.com/web-shop/l-tek-6lowpan-arduino-shield/)

### Software requirements

This app makes use of the following libraries:
- [Mbed MQTT library](https://github.com/ARMmbed/mbed-mqtt/#7fa219e87b3355e8a6fd281659697fe3a0c87630)
- [Zest_Core_STM32L4A6RG BSP (board support package)](https://github.com/catie-aq/mbed_zest-core-stm32l4a6rg/#e96c03e3644b204a877c24d7988a2c58c61552f8)

  
### Windows

- Open VS Code IDE.

- Clone the repository directly in a Dev Container, in the desited volume (Ctrl + Shift + P):
  `Dev Containers: Clone Repository in Named Container Volume`

> [!IMPORTANT]
> Cloning to a container volume in WSL increases massively the performance.

### Linux

- Clone the project:

    ```shell
    git clone https://gitlab.com/catie_estia-students/mbed-os-basic-template.git YOUR_PROJECT_NAME
    cd YOUR_PROJECT_NAME
    ```

- Open the project in VS Code IDE:

    ```shell
    cd YOUR_PROJECT_NAME
    code .
    ```

- Launch the Dev Container:

    ```
    Dev Containers: Reopen in Container
    ```

- In a VS Code terminal, export the project:

    ```shell
    mbed export -i vscode_gcc_arm
    ```

## Configure

Edit 'AccessCode.h' to set your 'API_ADAFRUIT'.

Edit 'Main.cpp' to put your information in namespace.

### On Adafruit: 
              -create groupe 'Station_Meteo'
              -create Feeds : Temperature / Humidity / Pressure / Alerte.
              -you can put this Feeds in your DashBoard
              -creat action who pubilsh :) or :( depending on temperatur feed to controle led
### Dashboard

Link to our Adafruits Dashboard : https://io.adafruit.com/Bodhix20/dashboards/station-meteo

## Working from command line

Compile the project:

```shell
mbed compile
```

Program the target device (eg. `STM32L4A6RG` for the Zest_Core_STM32L4A6RG) with a J-Link
debug probe:

```shell
python dist/program.py STM32L4A6RG BUILD/ZEST_CORE_STM32L4A6RG/GCC_ARM/basic-mqtt-example.elf
```
## Build and Flash

- Compile the project:

    ```shell
    mbed deploy # Deploy the dependencies
    mbed compile
    ```

- Program the target device with the compiled binary using a J-Link debug probe:

    ```shell
    sixtron_flash
    ```

## Export to your repository

Modify the remote URL to point to your repository and push the source code:

```shell
git remote set-url origin YOUR_REPOSITORY_URL
git push -u origin main
```
