# stm32-iot-central-sample
STM32CubeIDE projects that connect to Azure IoT Central.

Current boards supported in this repo:
* [32F746GDISCOVERY](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html)
* [B-U585I-IOT02A](https://www.st.com/en/evaluation-tools/b-u585i-iot02a.html)

The folder structure for the boards are identical, in the following sections, we will use the B-U585I-IOT02A as the example.

## Create Azure IoT Central application

* Visit https://apps.azureiotcentral.com/myapps to build a new IoT Central app.
* In the **Build you IoT application** screen, create a Custom app
* Choose the **Free** plan so you have a 7 days free account avaiable for testing.
* After the creation of your app, go to **Device** tab and add a new device.
* In the **Create a new device** page, specify the device details

  | Name      | Description |
  | ----------- | ----------- |
  | Device name      | Name of your device |
  | Device ID   | Unique ID of your device |
  | Device template   | Keep as default **Unassigned** |

* Click on the device you just create, then choose **Connect**. Copy the following device credentials which later you need to configure in your project.

  | Field      |
  | ----------- |
  | ID scope |
  | Device ID | 
  | Primary key |

## Prepare the local environment

* Download and install latest version of [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html).
* Clone the entire repo: `git clone https://github.com/liydu/stm32-iot-central-sample.git`.

## Run the sample

* Double click `B-U585I-IOT02A\Azure IoT Central\STM32CubeIDE\.project` to open the project in STM32CubeIDE.
* Open `B-U585I-IOT02A\Azure_IoT_Central\NetXDuo\App\app_azure_iot.h` and configure the following definitions from the previous fields you copied.

  | Field      | Description |
  | ----------- | ----------- |
  | IOT_DPS_ID_SCOPE      | ID scope |
  | IOT_DPS_REGISTRATION_ID   | Device ID |
  | IOT_DEVICE_SAS_KEY   | Primary key |

* For B-U585I-IOT02A, you also need to configure the Wi-Fi credentials by opening `B-U585I-IOT02A\Azure_IoT_Central\Core\Inc\mx_wifi_conf.h`.

  | Field      | Description |
  | ----------- | ----------- |
  | WIFI_SSID      | Wi-Fi SSID |
  | WIFI_PASSWORD   | Wi-Fi password |

* Now build the entire project and start a debug session within STM32CubeIDE.
* Open the terminal application like [Tera Term](https://ttssh2.osdn.jp/index.html.en). You will see the console output about the board is connecting to Azure IoT Central.

* Also open https://apps.azureiotcentral.com, within the device details page, in the **Raw data** tab, you will see the telemetry data sent from the device.

## How it works

This is a simple demo to connect the STM32 device to Azure IoT Central and capable of sending telemetry data to it. All data is sending to Azure IoT Central in the format of [IoT Plug and Play](https://docs.microsoft.com/azure/iot-develop/overview-iot-plug-and-play). For this demo, we only send the real-time temperature data.

A device is created in IoT Central and connects to the STM32 dev kit via a [SaS Token](https://docs.microsoft.com/azure/iot-central/core/concepts-device-authentication#create-individual-enrollments). For production, we recommended to pre-register all devices using [Device Provisioning Service (DPS)](https://docs.microsoft.com/azure/iot-dps/) and devices are connected using [X.509 certifications](https://docs.microsoft.com/azure/iot-central/core/concepts-device-authentication#x509-enrollment-group).

And security features such as TrustZone on STM32U5 are turned off for this sample.

