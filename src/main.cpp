#include "main.h"

//#ifdef CONFIG_PHY_USE_POWER_PIN
/* This replaces the default PHY power on/off function with one that
   also uses a GPIO for power on/off.
   If this GPIO is not connected on your device (and PHY is always powered), you
   can use the default PHY-specific power on/off function rather than overriding
   with this one.
*/
// static void phy_device_power_enable_via_gpio(bool enable) {
// 	assert(DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable);

// 	if (!enable) {
// 		/* Do the PHY-specific power_enable(false) function before powering down
// 		 */
// 		DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(false);
// 	}

// 	gpio_pad_select_gpio(PIN_PHY_POWER);
// 	gpio_set_direction(PIN_PHY_POWER, GPIO_MODE_OUTPUT);
// 	if (enable == true) {
// 		gpio_set_level(PIN_PHY_POWER, 1);
// 		ESP_LOGD(TAG, "phy_device_power_enable(TRUE)");
// 	} else {
// 		gpio_set_level(PIN_PHY_POWER, 0);
// 		ESP_LOGD(TAG, "power_enable(FALSE)");
// 	}

// 	// Allow the power up/down to take effect, min 300us
// 	vTaskDelay(1);

// 	if (enable) {
// 		/* Run the PHY-specific power on operations now the PHY has power */
// 		DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(true);
// 	}
// }
// #endif

static void eth_gpio_config_rmii(void) {
	// RMII data pins are fixed:
	// TXD0 = GPIO19
	// TXD1 = GPIO22
	// TX_EN = GPIO21
	// RXD0 = GPIO25
	// RXD1 = GPIO26
	// CLK == GPIO0
	phy_rmii_configure_data_interface_pins();
	// MDC is GPIO 23, MDIO is GPIO 18
	phy_rmii_smi_configure_pins(PIN_SMI_MDC, PIN_SMI_MDIO);
}

void eth_task(void *pvParameter) {
	tcpip_adapter_ip_info_t ip;
	memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	while (1) {

		vTaskDelay(2000 / portTICK_PERIOD_MS);

		if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip) == 0) {
			ESP_LOGI(TAG, "~~~~~~~~~~~");
			ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip.ip));
			ESP_LOGI(TAG, "ETHPMASK:" IPSTR, IP2STR(&ip.netmask));
			ESP_LOGI(TAG, "ETHPGW:" IPSTR, IP2STR(&ip.gw));
			ESP_LOGI(TAG, "~~~~~~~~~~~");
		}
	}
}

void app_main() {
	esp_err_t ret = ESP_OK;
	tcpip_adapter_init();
	esp_event_loop_init(NULL, NULL);

	eth_config_t config = DEFAULT_ETHERNET_PHY_CONFIG;
	/* Set the PHY address in the example configuration */
	config.phy_addr = CONFIG_PHY_ADDRESS;
	config.gpio_config = eth_gpio_config_rmii;
	config.tcpip_input = tcpip_adapter_eth_input;

	// #ifdef CONFIG_PHY_USE_POWER_PIN
	// 	/* Replace the default 'power enable' function with an example-specific
	// 	   one that toggles a power GPIO. */
	// 	config.phy_power_enable = phy_device_power_enable_via_gpio;
	// #endif

	ret = esp_eth_init(&config);

	if (ret == ESP_OK) {
		esp_eth_enable();
		xTaskCreate(eth_task, "eth_task", 2048, NULL, (tskIDLE_PRIORITY + 2),
					NULL);
	}
}