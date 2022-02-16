/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_azure_rtos.h"
#include "nx_ip.h"
/* USER CODE END Includes */


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
TX_THREAD AppMainThread;
TX_THREAD AppMQTTClientThread;

TX_SEMAPHORE Semaphore;

static NX_PACKET_POOL AppPool;
static NX_IP          IpInstance;
static NX_DHCP        DhcpClient;
static NX_DNS         DnsClient;
static NX_SNTP_CLIENT SntpClient;

/* System clock time for UTC.  */
static ULONG unix_time_base;

/* Declare SNTP servers */
static const CHAR* sntp_servers[] = {
    "0.pool.ntp.org",
    "1.pool.ntp.org",
    "2.pool.ntp.org",
    "3.pool.ntp.org",
};
static UINT sntp_server_index;

ULONG   IpAddress;
ULONG   NetMask;
ULONG   GatewayAddress;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID App_Main_Thread_Entry(ULONG thread_input);
static VOID App_MQTT_Client_Thread_Entry(ULONG thread_input);
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);
/* USER CODE END PFP */

extern UINT azure_iot_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

   /* USER CODE BEGIN App_NetXDuo_MEM_POOL */

  /* USER CODE END App_NetXDuo_MEM_POOL */

  /* USER CODE BEGIN MX_NetXDuo_Init */
#if (USE_MEMORY_POOL_ALLOCATION == 1)  
  printf("Nx_MQTT_Client application started..\n");
  
  CHAR *pointer;

  /* Initialize the NetX system. */
  nx_system_initialize();
  
  /* Allocate the memory for packet_pool. */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,  NX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  
  /* Create the Packet pool to be used for packet allocation */
  ret = nx_packet_pool_create(&AppPool, "Main Packet Pool", PAYLOAD_SIZE, pointer, NX_PACKET_POOL_SIZE);
  
  if (ret != NX_SUCCESS)
  {
    printf("ERROR: nx_packet_pool_create (0x%08x)\r\n", ret);
    return NX_NOT_ENABLED;
  }
  
  /* Allocate the memory for Ip_Instance */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 2 * DEFAULT_IP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  
  /* Create the main NX_IP instance */
  ret = nx_ip_create(&IpInstance, "Main Ip instance", NULL_ADDRESS, NULL_ADDRESS, &AppPool, nx_driver_emw3080_entry,
                     pointer, 2 * DEFAULT_IP_STACK_SIZE, DEFAULT_IP_STACK_PRIORITY);
  
  if (ret != NX_SUCCESS)
  {
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_ip_create (0x%08x)\r\n", ret);
    return NX_NOT_ENABLED;
  }
  
  /* create the DHCP client */
  ret = nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client");
  
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }
  
  /* Allocate the memory for ARP */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, ARP_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  
  /* Enable the ARP protocol and provide the ARP cache size for the IP instance */
  ret = nx_arp_enable(&IpInstance, (VOID *)pointer, ARP_MEMORY_SIZE);
  
  if (ret != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_arp_enable (0x%08x)\r\n", ret);
    return NX_NOT_ENABLED;
  }
  
  /* Enable the ICMP */
  ret = nx_icmp_enable(&IpInstance);
  
  if (ret != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_icmp_enable (0x%08x)\r\n", ret);
    return NX_NOT_ENABLED;
  }
  
  /* Enable the UDP protocol required for DHCP communication */
  ret = nx_udp_enable(&IpInstance);
  
  if (ret != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_udp_enable (0x%08x)\r\n", ret);
    return NX_NOT_ENABLED;
  }
  
  /* Enable the TCP protocol required for DNS, MQTT.. */
  ret = nx_tcp_enable(&IpInstance);
  
  if (ret != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_tcp_enable (0x%08x)\r\n", ret);
    return NX_NOT_ENABLED;
  }
  
  /* Allocate the memory for main thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, THREAD_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Initialize TLS. */
  nx_secure_tls_initialize();
  
  /* Create the main thread */
  ret = tx_thread_create(&AppMainThread, "App Main thread", App_Main_Thread_Entry, 0, pointer, THREAD_MEMORY_SIZE,
                         DEFAULT_IP_STACK_PRIORITY, DEFAULT_IP_STACK_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
  
  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }
  
  /* Allocate the memory for MQTT client thread   */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, THREAD_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  
  /* create the MQTT client thread */
  ret = tx_thread_create(&AppMQTTClientThread, "App MQTT Thread", App_MQTT_Client_Thread_Entry, 0, pointer, THREAD_MEMORY_SIZE,
                         DEFAULT_PRIORITY, DEFAULT_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);
  
  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }
  
  /* set DHCP notification callback  */
  tx_semaphore_create(&Semaphore, "DHCP Semaphore", 0);
#endif 
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
/**
* @brief  ip address change callback.
* @param ip_instance: NX_IP instance
* @param ptr: user data
* @retval none
*/
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  /* release the semaphore as soon as an IP address is available */
  tx_semaphore_put(&Semaphore);
}

/**
* @brief  Main thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID App_Main_Thread_Entry(ULONG thread_input)
{
  UINT ret = NX_SUCCESS;
  
  ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  
  /* start DHCP client */
  ret = nx_dhcp_start(&DhcpClient);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  
  /* wait until an IP address is ready */
  if(tx_semaphore_get(&Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS)
  {
    Error_Handler();
  }
  
  ret = nx_ip_address_get(&IpInstance, &IpAddress, &NetMask);
  
  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }
  
  PRINT_IP_ADDRESS(IpAddress);
  
  /* start the MQTT client thread */
  tx_thread_resume(&AppMQTTClientThread);
  
  /* this thread is not needed any more, we relinquish it */
  tx_thread_relinquish();
  
  return;  
}

/**
* @brief  DNS Create Function.
* @param dns_ptr
* @retval ret
*/
UINT dns_create(NX_DNS *dns_ptr)
{
  UINT ret = NX_SUCCESS;
  
  /* Create a DNS instance for the Client */
  ret = nx_dns_create(dns_ptr, &IpInstance, (UCHAR *)"DNS Client");
  if (ret)
  {
    Error_Handler();
  }
  /* Initialize DNS instance with a dummy server */
  ret = nx_dns_server_add(dns_ptr, USER_DNS_ADDRESS);
  if (ret)
  {
    Error_Handler();
  }
  
  return ret;
}

/**
 * @brief  Sync up the local time.
 * @param sntp_server_address
 * @retval ret
 */
UINT sntp_time_sync_internal(ULONG sntp_server_address)
{
  UINT ret;
  UINT server_status;
  UINT i;

  /* Create the SNTP Client to run in broadcast mode.. */
  ret = nx_sntp_client_create(
      &SntpClient, &IpInstance, 0, &AppPool, NX_NULL, NX_NULL, NX_NULL /* no random_number_generator callback */);

  /* Check ret.  */
  if (ret)
  {
    return (ret);
  }

  /* Use the IPv4 service to initialize the Client and set the IPv4 SNTP server. */
  ret = nx_sntp_client_initialize_unicast(&SntpClient, sntp_server_address);

  /* Check ret.  */
  if (ret)
  {
    nx_sntp_client_delete(&SntpClient);
    return (ret);
  }

  /* Set local time to 0 */
  ret = nx_sntp_client_set_local_time(&SntpClient, 0, 0);

  /* Check ret. */
  if (ret)
  {
    nx_sntp_client_delete(&SntpClient);
    return (ret);
  }

  /* Run Unicast client */
  ret = nx_sntp_client_run_unicast(&SntpClient);

  /* Check ret.  */
  if (ret)
  {
    nx_sntp_client_stop(&SntpClient);
    nx_sntp_client_delete(&SntpClient);
    return (ret);
  }

  /* Wait till updates are received */
  for (i = 0; i < SNTP_UPDATE_MAX; i++)
  {

    /* First verify we have a valid SNTP service running. */
    ret = nx_sntp_client_receiving_updates(&SntpClient, &server_status);

    /* Check ret.  */
    if ((ret == NX_SUCCESS) && (server_status == NX_TRUE))
    {

      /* Server ret is good. Now get the Client local time. */
      ULONG sntp_seconds, sntp_fraction;
      ULONG system_time_in_second;

      /* Get the local time.  */
      ret = nx_sntp_client_get_local_time(&SntpClient, &sntp_seconds, &sntp_fraction, NX_NULL);

      /* Check ret.  */
      if (ret != NX_SUCCESS)
      {
        continue;
      }

      /* Get the system time in second.  */
      system_time_in_second = tx_time_get() / TX_TIMER_TICKS_PER_SECOND;

      /* Convert to Unix epoch and minus the current system time.  */
      unix_time_base = (sntp_seconds - (system_time_in_second + UNIX_TO_NTP_EPOCH_SECOND));

      /* Time sync successfully.  */

      /* Stop and delete SNTP.  */
      nx_sntp_client_stop(&SntpClient);
      nx_sntp_client_delete(&SntpClient);

      return (NX_SUCCESS);
    }

    /* Sleep.  */
    tx_thread_sleep(SNTP_UPDATE_INTERVAL);
  }

  /* Time sync failed.  */

  /* Stop and delete SNTP.  */
  nx_sntp_client_stop(&SntpClient);
  nx_sntp_client_delete(&SntpClient);

  /* Return success.  */
  return (NX_NOT_SUCCESSFUL);
}

/**
 * @brief  Sync up the local time.
 * @param void
 * @retval ret
 */
UINT sntp_time_sync()
{
  UINT  ret;
  ULONG gateway_address;
  ULONG sntp_server_address[3];
  UINT  sntp_server_address_size = sizeof(sntp_server_address);

  /* Retrieve NTP server address.  */
  ret = nx_dhcp_interface_user_option_retrieve(
      &DhcpClient, 0, NX_DHCP_OPTION_NTP_SVR, (UCHAR*)(sntp_server_address), &sntp_server_address_size);

  /* Check ret.  */
  if (ret == NX_SUCCESS)
  {
    for (UINT i = 0; (i * 4) < sntp_server_address_size; i++)
    {
      PRINT_IP_ADDRESS(sntp_server_address[i]);

      /* Start SNTP to sync the local time.  */
      ret = sntp_time_sync_internal(sntp_server_address[i]);

      /* Check ret.  */
      if (ret == NX_SUCCESS)
      {
        return (NX_SUCCESS);
      }
    }
  }

  /* Sync time by NTP server array.  */
  for (UINT i = 0; i < SNTP_SYNC_MAX; i++)
  {
    printf("SNTP Time Sync...%s\r\n", sntp_servers[sntp_server_index]);

    /* Make sure the network is still valid.  */
    while (nx_ip_gateway_address_get(&IpInstance, &GatewayAddress))
    {
      tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

    /* Look up SNTP Server address. */
    ret = nx_dns_host_by_name_get(
        &DnsClient, (UCHAR*)sntp_servers[sntp_server_index], &sntp_server_address[0], 5 * NX_IP_PERIODIC_RATE);

    /* Check ret.  */
    if (ret == NX_SUCCESS)
    {

      /* Start SNTP to sync the local time.  */
      ret = sntp_time_sync_internal(sntp_server_address[0]);

      /* Check ret.  */
      if (ret == NX_SUCCESS)
      {
        return (NX_SUCCESS);
      }
    }

    /* Switch SNTP server every time.  */
    sntp_server_index = (sntp_server_index + 1) % (sizeof(sntp_servers) / sizeof(sntp_servers[0]));
  }

  return (NX_NOT_SUCCESSFUL);
}

/**
 * @brief  Unix Time Get Function.
 * @param unix_time
 * @retval ret
 */
UINT unix_time_get(ULONG* unix_time)
{
  UINT ret = NX_SUCCESS;

  /* Return number of seconds since Unix Epoch (1/1/1970 00:00:00).  */
  *unix_time = unix_time_base + (tx_time_get() / TX_TIMER_TICKS_PER_SECOND);

  return ret;
}

/**
* @brief  MQTT Client thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID App_MQTT_Client_Thread_Entry(ULONG thread_input)
{
  UINT ret = NX_SUCCESS;
  UINT        unix_time;

  //mqtt_server_ip.nxd_ip_version = 4;
  
  /* Create a DNS client */
  ret = dns_create(&DnsClient);
  
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  
  /* Sync up time by SNTP at start up.  */
  ret = sntp_time_sync();

  if (ret != NX_SUCCESS)
  {
    printf("SNTP Time Sync failed.\r\n");
    printf("Set Time to default value: %u.", DEFAULT_SYSTEM_TIME);
    unix_time_base = DEFAULT_SYSTEM_TIME;
  }
  else
  {
    printf("SNTP Time Sync successfully.\r\n");
  }

  unix_time_get((ULONG*)&unix_time);
  srand(unix_time);

  /* Start the IoT entry. */
  ret = azure_iot_entry(&IpInstance, &AppPool, &DnsClient, unix_time_get);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: Failed to run Azure IoT (0x%04x)\r\n", ret);
    Error_Handler();
  }
}
/* USER CODE END 1 */
