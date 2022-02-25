## Azure IoT Central Application Description

This application provides an example of Azure RTOS NetX/NetXDuo stack usage . 

It shows how to exchange data between client and server using MQTT protocol in an encrypted mode supporting TLS v1.2.

The main entry function tx_application_define() is called by ThreadX during kernel start, at this stage, all NetX resources are created.

 + A <i>NX_PACKET_POOL</i>is allocated

 + A <i>NX_IP</i> instance using that pool is initialized

 + The <i>ARP</i>, <i>ICMP</i>, <i>UDP</i> and <i>TCP</i> protocols are enabled for the <i>NX_IP</i> instance

 + A <i>DHCP client is created.</i>

The **AppMainThread** starts and perform the following actions:

  + Starts the DHCP client

  + Waits for the IP address resolution

  + Resumes the **AppMQTTClientThread**

####  Expected success behavior

 + The board IP address is printed on the HyperTerminal
 
 + Connection's information are printed on the HyperTerminal

  ```
Starting Azure thread

MX_NetXDuo_Init started..

Initializing DHCP
        STM32 ip_address: 192.168.1.22 
        STM32 ip_mask: 255.255.255.0 
        STM32 gateway_address: 192.168.1.1 
SUCCESS: DHCP initialized

Initializing DNS client
        STM32 dns_server_address[0]: 192.168.1.1 
        STM32 dns_server_address[1]: 0.0.0.0 
SUCCESS: DNS client initialized

Initializing SNTP time sync
        SNTP server 0.pool.ntp.org
        SNTP time update: Feb 25, 2022 5:21:44.571 UTC 
SUCCESS: SNTP initialized

Initializing Azure IoT DPS client
        DPS endpoint: global.azure-devices-provisioning.net
        DPS ID scope: 0ne00433CCB
        Registration ID: u5
SUCCESS: Azure IoT DPS client initialized

Initializing Azure IoT Hub client
        Hub hostname: iotc-eb07e8f2-2a3a-49da-8f1a-c14db2794701.azure-devices.net
        Device id: u5
        Model id: dtmi:azurertos:devkit:gsgstml4s5;2
SUCCESS: Connected to IoT Hub

Telemetry message sent: {"temperature":30.49}.
 ```
 + Green led is toggling after successfully receiving all messages.

#### Error behaviors

+ The red LED is toggling to indicate any error that has occurred.

#### Assumptions if any
None

#### Known limitations
 

#### ThreadX usage hints

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.

 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define,but this should be reflected in "tx_initialize_low_level.s" file too.

 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.

 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).

 - Using dynamic memory allocation requires to apply some changes to the linker file.

   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function, using the "first_unused_memory" argument.
   This require changes in the linker files to expose this memory location.

    + For EWARM add the following section into the .icf file:
     ```
	 place in RAM_region    { last section FREE_MEM };
	 ```
    + For MDK-ARM:
	```
    either define the RW_IRAM1 region in the ".sct" file
    or modify the line below in "tx_low_level_initilize.s to match the memory region being used
        LDR r1, =|Image$$RW_IRAM1$$ZI$$Limit|
	```
    + For STM32CubeIDE add the following section into the .ld file:
	``` 
    ._threadx_heap :
      {
         . = ALIGN(8);
         __RAM_segment_used_end__ = .;
         . = . + 64K;
         . = ALIGN(8);
       } >RAM_D1 AT> RAM_D1
	``` 
	
       The simplest way to provide memory for ThreadX is to define a new section, see ._threadx_heap above.
       In the example above the ThreadX heap size is set to 64KBytes.
       The ._threadx_heap must be located between the .bss and the ._user_heap_stack sections in the linker script.	 
       Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).	 
       Read more in STM32CubeIDE User Guide, chapter: "Linker script".
	  
    + The "tx_initialize_low_level.s" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.
         
### Keywords

RTOS, Network, ThreadX, NetXDuo, WIFI, MQTT, DNS, TLS, MXCHIP, UART, IoT Central


### Hardware and Software environment

  - This example runs on STM32U585xx devices with a WiFi module (MXCHIP:EMW3080) with this configuration :

    + MXCHIP Firmware 2.1.11

    + Bypass mode 

    + SPI mode used as interface

    The EMW3080B MXCHIP Wi-Fi module firmware and the way to update your board with it are available at <https://www.st.com/en/development-tools/x-wifi-emw3080b.html>

    Before using this project, you shall update your B-U585I-IOT02A board with the EMW3080B firmware version 2.1.11.

    To achieve this, follow the instructions given at the above link, using the EMW3080updateV2.1.11RevC.bin flasher under the V2.1.11/SPI folder.

  - This application has been tested with B-U585I-IOT02A (MB1551-U585AI) boards Revision: RevC and can be easily tailored to any other supported device and development board.

  - This application uses USART1 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = None
      - Flow control = None


###  How to use it ?

In order to make the program work, you must do the following :

#### Azure IoT

- Register a free Azure IoT Central app at: https://apps.azureiotcentral.com/
- Create a new device in it and get the device connection credentials: https://docs.microsoft.com/azure/iot-central/core/concepts-get-connected#device-registration

#### Device

- Open `.project` in `B-U585I-IOT02A/STM32CubeIDE` from STM32CubeIDE.

- On `Core/Inc/mx_wifi_conf.h`, edit your Wifi Settings (`WIFI_SSID`, `WIFI_PASSWORD`)
 
- Edit the file `NetXDuo/App/app_azure_iot.h` : define the `IOT_DPS_ID_SCOPE`, the `IOT_DPS_REGISTRATION_ID` and `IOT_DEVICE_SAS_KEY` retrieved from Azure IoT Central.

- Rebuild all files and load your image into target memory.
 
- Run the application.

### To be done 

***As if 2022/2/24***

- [ ] Implement the STM32 B-U585I-IOT02A IoT Plug and Play device model defined in *Model* folder.
- [ ] Test with IoT Central.
- [ ] Clean up the code. 