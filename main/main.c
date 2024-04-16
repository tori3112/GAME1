/* HTTP Restful API Server Example
 * Mirf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include "protocol_examples_common.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mirf.h"

/*
 * GAME HEADERS
 */
#include "alphabeta.h"
#include "board.h"
#include "limits.h"

/**
 * GLOBAL VARIABLES
 **/
bool game_over;
NRF24_t dev;
bool new_val;
#define NUM_CHANNELS 8
char position_string[41];
char outcome_message[85];
int adc_values[NUM_CHANNELS] = {0};
int channels[NUM_CHANNELS] = {ADC1_CHANNEL_0, ADC1_CHANNEL_1, ADC1_CHANNEL_3,
                              ADC1_CHANNEL_5, ADC1_CHANNEL_6, ADC1_CHANNEL_2, ADC1_CHANNEL_7};
TaskHandle_t task_handler = NULL;
int thresholds[NUM_CHANNELS];
uint8_t moveC;

/**
 * nRF24 MODULE configs
 */
#if CONFIG_ADVANCED
void AdvancedSettings(NRF24_t * dev)
{
#if CONFIG_RF_RATIO_2M
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 2MBps");
	Nrf24_SetSpeedDataRates(dev, 1);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_1M
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 1MBps");
	Nrf24_SetSpeedDataRates(dev, 0);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_250K
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 250KBps");
	Nrf24_SetSpeedDataRates(dev, 2);
#endif // CONFIG_RF_RATIO_2M

	ESP_LOGW(pcTaskGetName(0), "CONFIG_RETRANSMIT_DELAY=%d", CONFIG_RETRANSMIT_DELAY);
	Nrf24_setRetransmitDelay(dev, CONFIG_RETRANSMIT_DELAY);
}
#endif // CONFIG_ADVANCED

#if CONFIG_RECEIVER
void receiver(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	uint8_t payload = 32;
	uint8_t channel = CONFIG_RADIO_CHANNEL;
	Nrf24_config(&dev, channel, payload);

	//Set own address using 5 characters
	esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

#if CONFIG_ADVANCED
	AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

	//Print settings
	Nrf24_printDetails(&dev);
	ESP_LOGI(pcTaskGetName(0), "Listening...");

	uint8_t buf[32];

	// Clear RX FiFo
	while(1) {
		if (Nrf24_dataReady(&dev) == false) break;
		Nrf24_getData(&dev, buf);
	}

	while(1) {
		//When the program is received, the received data is output from the serial port
		if (Nrf24_dataReady(&dev)) {
			Nrf24_getData(&dev, buf);
			ESP_LOGI(pcTaskGetName(0), "Got data:%s", buf);
			//ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), buf, payload, ESP_LOG_INFO);
		}
		vTaskDelay(1);
	}
}
#endif // CONFIG_RECEIVER


#if CONFIG_SENDER
void sender(void *pvParameters)
{
    ESP_LOGI(pcTaskGetName(0), "Start Sender Configuration");
    Nrf24_init(&dev);
    uint8_t payload = 4;
    uint8_t channel = 112;
    Nrf24_config(&dev, channel, payload);

    //Set the receiver address using 5 characters
    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"ABCDE");
    if (ret != ESP_OK) {
        ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
        while(1) { vTaskDelay(1); }
    }

#if CONFIG_ADVANCED
    AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

    //Print settings
    Nrf24_printDetails(&dev);

    uint8_t buf[4];
    int i;
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGW("LDR","ldr7 values: %d",adc_values[7]);
    while(1) {
        if (new_val) {
            //TickType_t nowTick = xTaskGetTickCount();
            //sprintf((char *)buf, &moveC, sizeof(moveC));
            ESP_LOGI("GAME", "moveC is %hhu and buffer is %s", moveC, buf);
            memcpy(buf, &moveC, sizeof(moveC));
            ESP_LOGI("GAME", "moveC is %hhu and buffer is %s", moveC, buf);
            //i = 6;
            Nrf24_send(&dev, &moveC);
            //vTaskDelay(100 / portTICK_PERIOD_MS);
            if (Nrf24_isSend(&dev, 1000)) {
                ESP_LOGI(pcTaskGetName(0), "Send success: %hhu", moveC);
                new_val=false;
                vTaskDelay(7000/portTICK_PERIOD_MS);
                ESP_LOGI("GAME","waiting to make human move done");
                //vTaskDelete(task_handler);
            } else {
                ESP_LOGW(pcTaskGetName(0), "Send fail:");
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
#endif // CONFIG_SENDER

/**
 * RESTful SERVER configs
 */
#define MDNS_HOST_NAME "GAME1"
#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
#include "driver/sdmmc_host.h"
#endif

#define MDNS_INSTANCE "esp home web server"
/* TAGS FOR LOGS*/
static const char *TAG = "GAME1";
static const char *TEST_TAG = "game-test";

/* TESTING ASSERTIONS */
#define TEST_ASSERT_MESSAGE( condition, ...)                                \
    if (condition)                                                          \
    {                                                                       \
        ESP_LOGI(TEST_TAG, "test passed");                                  \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        ESP_LOGE(TEST_TAG, __VA_ARGS__);                                    \
    }

esp_err_t start_rest_server(const char *base_path);

static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set(CONFIG_EXAMPLE_MDNS_HOST_NAME);
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
            {"board", "esp32"},
            {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

#if CONFIG_EXAMPLE_WEB_DEPLOY_SEMIHOST
esp_err_t init_fs(void)
{
    esp_err_t ret = esp_vfs_semihost_register(CONFIG_EXAMPLE_WEB_MOUNT_POINT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register semihost driver (%s)!", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    return ESP_OK;
}
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
esp_err_t init_fs(void)
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY); // CMD
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);  // D0
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);  // D1
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY); // D2
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY); // D3

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(CONFIG_EXAMPLE_WEB_MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    /* print card info if mount successfully */
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SF
esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_EXAMPLE_WEB_MOUNT_POINT,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}
#endif

int get_idx_from_value(int array[], int value) {
    for (int i=0; i<NUM_CHANNELS+1; i++) {
        if (i==4) continue;
        if (value == array[i]) {
            return i;
        }
    }
    return -1;
}

void setup_adc() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_1,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_5,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_2,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);

    /*thresholds[0] = adc1_get_raw(ADC1_CHANNEL_0) - 300;
    thresholds[1] = adc1_get_raw(ADC1_CHANNEL_1) - 300;
    thresholds[2] = adc1_get_raw(ADC1_CHANNEL_2) - 300;
    thresholds[3] = adc1_get_raw(ADC1_CHANNEL_3) - 300;
    thresholds[5] = adc1_get_raw(ADC1_CHANNEL_5) - 300;
    thresholds[6] = adc1_get_raw(ADC1_CHANNEL_6) - 300;
    thresholds[7] = adc1_get_raw(ADC1_CHANNEL_7) - 300;*/
    thresholds[0] = (adc1_get_raw(ADC1_CHANNEL_0) + adc1_get_raw(ADC1_CHANNEL_0) + adc1_get_raw(ADC1_CHANNEL_0)) / 3 - 350;
    thresholds[1] = (adc1_get_raw(ADC1_CHANNEL_1) + adc1_get_raw(ADC1_CHANNEL_1) + adc1_get_raw(ADC1_CHANNEL_1)) / 3 - 350;
    thresholds[2] = (adc1_get_raw(ADC1_CHANNEL_2) + adc1_get_raw(ADC1_CHANNEL_2) + adc1_get_raw(ADC1_CHANNEL_2)) / 3 - 350;
    thresholds[3] = (adc1_get_raw(ADC1_CHANNEL_3) + adc1_get_raw(ADC1_CHANNEL_3) + adc1_get_raw(ADC1_CHANNEL_3)) / 3 - 350;
    thresholds[4] = (adc1_get_raw(ADC1_CHANNEL_4) + adc1_get_raw(ADC1_CHANNEL_4) + adc1_get_raw(ADC1_CHANNEL_4)) / 3 - 350;
    thresholds[5] = (adc1_get_raw(ADC1_CHANNEL_5) + adc1_get_raw(ADC1_CHANNEL_5) + adc1_get_raw(ADC1_CHANNEL_5)) / 3 - 350;
    thresholds[6] = (adc1_get_raw(ADC1_CHANNEL_6) + adc1_get_raw(ADC1_CHANNEL_6) + adc1_get_raw(ADC1_CHANNEL_6)) / 3 - 350;
    thresholds[7] = (adc1_get_raw(ADC1_CHANNEL_7) + adc1_get_raw(ADC1_CHANNEL_7) + adc1_get_raw(ADC1_CHANNEL_7)) / 3 - 350;
}

void run_game() {
    setup_adc();
    while (1) {
        vTaskDelay(100/portTICK_PERIOD_MS);
        //GAME INITIALISATION
        ESP_LOGI("GAME1","game initialised");
        bitboard bb = {0, 0, 0};
        game_over = false;
        int turn = HUMAN;

        while (!game_over) {
            switch (turn) {
                case HUMAN: {
                    ESP_LOGI("GAME1","human turn");
                    /**
                     * PLACE FOR GETTING SIGNALS
                     * FROM SENSORS
                     */
                    bool keepRunning = true;
                    int moveH;
                    while(keepRunning) {
                        for (int i = 0; i < NUM_CHANNELS; i++) {
                            if (i == 4) continue; // Skip channel 4 since it's not configured
                            adc_values[i] = adc1_get_raw(i);
                        }
                        for (int i=0; i<NUM_CHANNELS; i++) {
                            if (i==4) continue;
                            if (adc_values[i] < thresholds[i]) {
                                //vTaskDelay(4000/portTICK_PERIOD_MS);
                                keepRunning = false;
                                moveH=get_idx_from_value(adc_values,adc_values[i]);
                                ESP_LOGI("GAME:ADC", "channel %d (Column %d) is below the threshold: %d < %d", channels[i], moveH, adc_values[i], thresholds[i]);
                                break;
                            }
                        }
                    }
                    play(&bb,moveH);
                    TEST_ASSERT_MESSAGE(false,"MASK is %llu, POSITION is %llu", bb.mask, bb.position);
                    //strncat(position_string,&moveH,1);
                    game_over = check_win(bb.position);
                    if (game_over) {
                        ESP_LOGI("GAME1","game won by human");
                        strcpy(outcome_message,"Conratulations, you won! Press the reset button to play again.");
                        vTaskDelay(100/portTICK_PERIOD_MS);
                        vTaskDelete(task_handler);
                        //initialise_bitboard(&bb);
                    } else {
                        turn = COMPUTER;
                    }
                    break;}
                case COMPUTER: {
                    setup_adc();
                    ESP_LOGI("GAME1","computer turn");
                    move next_move = negamax_ab_bb(bb,-UINT64_MAX,UINT64_MAX,5);
                    new_val=true;
                    moveC = next_move.col;
                    ESP_LOGI("GAME","next move C is %hhu", moveC);
                    //if (Nrf24_isSend(&dev,moveC);
                    /**
                     * PLACE FOR SENDING SIGNAL
                     * TO THE SERVOMOTORS
                     */
                    play(&bb,moveC);
                    TEST_ASSERT_MESSAGE(false,"MASK is %llu, POSITION is %llu", bb.mask, bb.position);
                    //strncat(position_string,&moveC,1);
                    game_over = check_win(bb.position);
                    if (game_over) {
                        ESP_LOGI("GAME1","game won by computer");
                        strcpy(outcome_message,
                               "Unfortunately you lost, better luck next time! Press the reset button to try again.");
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        vTaskDelete(task_handler);
                        //initialise_bitboard(&bb);
                    } else {
                        turn = HUMAN;
                    }
                    break;
                }
                default:
                    TEST_ASSERT_MESSAGE(false, "something went wrong, shouldn't get to default");
            }
        }
    }
}

void app_main(void)
{   /* INITIALISE SOME COMPONENTS *//*
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name(MDNS_HOST_NAME);
    *//* CONNECT TO WIFI AND START RESTFUL SERVER *//*
    ESP_ERROR_CHECK(example_connect());
    ESP_ERROR_CHECK(init_fs());
    ESP_ERROR_CHECK(start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT));*/
    /* TASKS FOR nRF24 MODULE */
#if CONFIG_SENDER
    xTaskCreate(&sender, "SENDER", 1024*3, NULL, 2, &task_handler);
#endif
    /* CREATE TASK FOR THE GAME LOOP*/
    vTaskDelay(100 / portTICK_PERIOD_MS);
    xTaskCreate(run_game,"GAME",4096,NULL,10,NULL);
}