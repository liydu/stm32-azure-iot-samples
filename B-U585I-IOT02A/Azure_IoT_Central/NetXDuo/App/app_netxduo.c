/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo application file
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
/* Declare SNTP servers */
static const CHAR* SNTP_SERVER[] = {
    "0.pool.ntp.org",
    "1.pool.ntp.org",
    "2.pool.ntp.org",
    "3.pool.ntp.org",
};
static UINT sntp_server_count;

static TX_EVENT_FLAGS_GROUP sntp_flags;

ULONG   IpAddress;
ULONG   NetMask;
ULONG   GatewayAddress;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SNTP_UPDATE_EVENT 1

/* Time to wait for each server poll. */
#define SNTP_WAIT_TIME (10 * NX_IP_PERIODIC_RATE)

/* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999). */
#define UNIX_TO_NTP_EPOCH_SECS 0x83AA7E80
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
NX_PACKET_POOL AppPool;
NX_IP          IpInstance;
NX_DHCP        DhcpClient;
NX_DNS         DnsClient;
NX_SNTP_CLIENT SntpClient;

static UCHAR nx_ip_stack[NX_IP_STACK_SIZE];
static UCHAR nx_ip_pool[NX_PACKET_POOL_SIZE];

static ULONG nx_arp_cache[NX_ARP_CACHE_SIZE];

/* Variables to keep track of time. */ 
static ULONG sntp_last_time = 0;
static ULONG tx_last_ticks  = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
static VOID time_update_callback(NX_SNTP_TIME_MESSAGE* time_update_ptr, NX_SNTP_TIME* local_time)
{
  // Set the update flag so we pick up the new time in the SNTP thread
  tx_event_flags_set(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR);
}

static VOID set_sntp_time()
{
  UINT  status;
  ULONG seconds;
  ULONG milliseconds;
  CHAR  time_buffer[64];

  status = nx_sntp_client_get_local_time(&SntpClient, &seconds, &milliseconds, NX_NULL);
  if (status != NX_SUCCESS)
  {
    printf("ERROR: Internal error with getting local time (0x%08x)\n", status);
    return;
  }

  // Stash the Unix and ThreadX times
  sntp_last_time = seconds - UNIX_TO_NTP_EPOCH_SECS;
  tx_last_ticks  = tx_time_get();

  nx_sntp_client_utility_display_date_time(&SntpClient, time_buffer, sizeof(time_buffer));

  printf("\tSNTP time update: %s\r\n", time_buffer);
  printf("SUCCESS: SNTP initialized\r\n");
}

static UINT sntp_client_run()
{
  UINT        status;
  NXD_ADDRESS sntp_address;

  if (sntp_server_count > (sizeof(SNTP_SERVER) / sizeof(&SNTP_SERVER)))
  {
    // We rotated through all the servers, fail out.
    return NX_SNTP_SERVER_NOT_AVAILABLE;
  }

  printf("\tSNTP server %s\r\n", SNTP_SERVER[sntp_server_count]);

  // Stop the server in case it's already running
  nx_sntp_client_stop(&SntpClient);

#ifndef SAMPLE_SNTP_SERVER_ADDRESS
  // Resolve DNS
  if ((status = nxd_dns_host_by_name_get(&DnsClient,
           (UCHAR*)SNTP_SERVER[sntp_server_count],
           &sntp_address,
           5 * NX_IP_PERIODIC_RATE,
           NX_IP_VERSION_V4)))
  {
    printf("ERROR: Unable to resolve SNTP IP %s (0x%08x)\r\n", SNTP_SERVER[sntp_server_count], status);
  }
#endif /* SAMPLE_SNTP_SERVER_ADDRESS */

  // Initialize the service
  // if ((status = nx_sntp_client_initialize_unicast(&SntpClient, SAMPLE_SNTP_SERVER_ADDRESS)))
  if ((status = nxd_sntp_client_initialize_unicast(&SntpClient, &sntp_address)))
  {
    printf("ERROR: Unable to initialize unicast SNTP client (0x%08x)\r\n", status);
  }

  // Run Unicast client
  else if ((status = nx_sntp_client_run_unicast(&SntpClient)))
  {
    printf("ERROR: Unable to start unicast SNTP client (0x%08x)\r\n", status);
  }

  // rotate to the next SNTP service
  sntp_server_count++;

  return status;
}

ULONG sntp_time_get()
{
  // Calculate how many seconds have elapsed since the last sync
  ULONG tx_time_delta = (tx_time_get() - tx_last_ticks) / TX_TIMER_TICKS_PER_SECOND;

  // Add this to the last sync time to get the current time
  ULONG sntp_time = sntp_last_time + tx_time_delta;

  return sntp_time;
}

UINT sntp_time(ULONG* unix_time)
{
  *unix_time = sntp_time_get();

  return NX_SUCCESS;
}

UINT sntp_init()
{
  UINT status;

  if ((status = tx_event_flags_create(&sntp_flags, "SNTP")))
  {
    printf("ERROR: Create SNTP event flags (0x%08x)\r\n", status);
  }

  else if ((status = nx_sntp_client_create(
                &SntpClient, &IpInstance, 0, IpInstance.nx_ip_default_packet_pool, NX_NULL, NX_NULL, NULL)))
  {
    printf("ERROR: SNTP client create failed (0x%08x)\r\n", status);
  }

  else if ((status = nx_sntp_client_set_local_time(&SntpClient, 0, 0)))
  {
    printf("ERROR: Unable to set local time for SNTP client (0x%08x)\r\n", status);
    nx_sntp_client_delete(&SntpClient);
  }

  // Setup time update callback function
  else if ((status = nx_sntp_client_set_time_update_notify(&SntpClient, time_update_callback)))
  {
    printf("ERROR: nx_sntp_client_set_time_update_notify (0x%08x)\r\n", status);
    nx_sntp_client_delete(&SntpClient);
  }

  return status;
}

/* Connect functions. */
UINT sntp_sync()
{
  UINT  status;
  UINT  server_status;
  ULONG events = 0;

  printf("\r\nInitializing SNTP time sync\r\n");

  // Reset the server index so we start from the beginning
  sntp_server_count = 0;

  while (NX_TRUE)
  {
    // Run the client
    sntp_client_run();

    // Wait for new events
    events = 0;
    tx_event_flags_get(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR_CLEAR, &events, SNTP_WAIT_TIME);

    if (events & SNTP_UPDATE_EVENT)
    {
      // Get time updates
      nx_sntp_client_receiving_updates(&SntpClient, &server_status);

      // New time, update our local time and we are done
      set_sntp_time();
      status = NX_SUCCESS;
      break;
    }

    // Otherwise we rotate around to the next server
  }

  nx_sntp_client_stop(&SntpClient);

  return status;
}

UINT dhcp_connect()
{
  UINT  status;
  ULONG ip_status;
  ULONG ip_address      = 0;
  ULONG ip_mask         = 0;
  ULONG gateway_address = 0;

  printf("\r\nInitializing DHCP\r\n");

  // Create DHCP client
  nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client");

  // Start DHCP client
  nx_dhcp_start(&DhcpClient);

  // Wait until IP address is resolved
  nx_ip_status_check(&IpInstance, NX_IP_ADDRESS_RESOLVED, &ip_status, NX_WAIT_FOREVER);

  // Get IP address
  if ((status = nx_ip_address_get(&IpInstance, &ip_address, &ip_mask)))
  {
    printf("ERROR: nx_ip_address_get (0x%08x)\r\n", status);
    return status;
  }

  // Get gateway address
  if ((status = nx_ip_gateway_address_get(&IpInstance, &gateway_address)))
  {
    printf("ERROR: nx_ip_gateway_address_get (0x%08x)\r\n", status);
    return status;
  }

  // Output IP address and gateway address
  PRINT_IP_ADDRESS(ip_address);
  PRINT_IP_ADDRESS(ip_mask);
  PRINT_IP_ADDRESS(gateway_address);

  printf("SUCCESS: DHCP initialized\r\n");

  return NX_SUCCESS;
}

UINT dns_connect()
{
  UINT  status;
  ULONG dns_server_address[2]   = {0};
  UINT  dns_server_address_size = 12;

  printf("\r\nInitializing DNS client\r\n");

  if ((status = nx_dns_server_remove_all(&DnsClient)))
  {
    printf("ERROR: nx_dns_server_remove_all (0x%08x)\r\n", status);
    return status;
  }

  /* Retrieve DNS server address. */
  if ((status = nx_dhcp_interface_user_option_retrieve(
           &DhcpClient, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR*)(dns_server_address), &dns_server_address_size)))
  {
    printf("ERROR: nx_dhcp_interface_user_option_retrieve (0x%08x)\r\n", status);
    return status;
  }

  /* Output DNS Server address. */
  PRINT_IP_ADDRESS(dns_server_address[0]);
  PRINT_IP_ADDRESS(dns_server_address[1]);

  /* Add first IPv4 server address to the Client list */
  if ((status = nx_dns_server_add(&DnsClient, dns_server_address[0])))
  {
    printf("ERROR: nx_dns_server_add (0x%08x)\r\n", status);
    return status;
  }

  printf("SUCCESS: DNS client initialized\r\n");

  return NX_SUCCESS;
}

/**
 * @brief  Application NetXDuo Initialization.
 * @param memory_ptr: memory pointer
 * @retval int
 */
UINT MX_NetXDuo_Init()
{
  UINT status = NX_SUCCESS;

  /* USER CODE BEGIN MX_NetXDuo_Init */
  printf("MX_NetXDuo_Init started..\r\n");

  /* Initialize the NetX system. */
  nx_system_initialize();

  /* Create the Packet pool to be used for packet allocation */
  status = nx_packet_pool_create(&AppPool, "Main Packet Pool", NX_PACKET_SIZE, nx_ip_pool, NX_PACKET_POOL_SIZE);

  if (status != NX_SUCCESS)
  {
    printf("ERROR: nx_packet_pool_create (0x%08x)\r\n", status);
    return NX_NOT_ENABLED;
  }

  /* Create the main NX_IP instance */
  status = nx_ip_create(&IpInstance,
      "Main Ip instance",
      NULL_ADDRESS,
      NULL_ADDRESS,
      &AppPool,
      nx_driver_emw3080_entry,
      nx_ip_stack,
      NX_IP_STACK_SIZE,
      NX_IP_STACK_PRIORITY);

  if (status != NX_SUCCESS)
  {
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_ip_create (0x%08x)\r\n", status);
    return NX_NOT_ENABLED;
  }

  /* Enable the ARP protocol and provide the ARP cache size for the IP instance */
  status = nx_arp_enable(&IpInstance, (VOID*)nx_arp_cache, NX_ARP_CACHE_SIZE);

  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_arp_enable (0x%08x)\r\n", status);
    return NX_NOT_ENABLED;
  }

  /* Enable the ICMP */
  status = nx_icmp_enable(&IpInstance);

  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_icmp_enable (0x%08x)\r\n", status);
    return NX_NOT_ENABLED;
  }

  /* Enable the TCP protocol required for DNS, MQTT.. */
  status = nx_tcp_enable(&IpInstance);

  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_tcp_enable (0x%08x)\r\n", status);
    return NX_NOT_ENABLED;
  }

  /* Enable the UDP protocol required for DHCP communication */
  status = nx_udp_enable(&IpInstance);

  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_udp_enable (0x%08x)\r\n", status);
    return NX_NOT_ENABLED;
  }

  /* Create DNS client. */
  status = nx_dns_create(&DnsClient, &IpInstance, (UCHAR*)"DNS Client");

  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_dns_create (0x%08x)\r\n", status);
    return NX_NOT_ENABLED;
  }

    // Use the packet pool here
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
  status = nx_dns_packet_pool_set(&DnsClient, &IpInstance.nx_ip_default_packet_pool);

  if (status != NX_SUCCESS)
  {
    nx_dns_delete(&DnsClient);
    nx_ip_delete(&IpInstance);
    nx_packet_pool_delete(&AppPool);
    printf("ERROR: nx_dns_packet_pool_set (%0x08)\r\n", status);
  }
#endif

  /* Initialize the SNTP client. */
  status = sntp_init();

  if (status != NX_SUCCESS)
  {
    printf("ERROR: sntp_init (0x%08x)\r\n", status);
  }

  /* Initialize TLS. */
  nx_secure_tls_initialize();

  /* USER CODE END MX_NetXDuo_Init */

  return status;
}

UINT MX_NetXDuo_Connect()
{
  UINT status;

  // Fetch IP details
  if ((status = dhcp_connect()))
  {
    printf("ERROR: dhcp_connect\r\n");
  }

  // Create DNS
  else if ((status = dns_connect()))
  {
    printf("ERROR: dns_connect\r\n");
  }

  // Wait for an SNTP sync
  else if ((status = sntp_sync()))
  {
    printf("ERROR: Failed to sync SNTP time (0x%08x)\r\n", status);
  }

  return status;
}

/* USER CODE END 1 */
