
#include "spi.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "freertos/task.h"
#include "esp_log.h"

#define VSPI_DMA_CHANNEL    2
static spi_device_handle_t vspi;
static volatile bool vspi_trans_in_progress;

void spi_post_transfer_callback(spi_transaction_t *t)
{
    uint8_t cs=*((uint8_t*)t->user);
    gpio_set_level(cs, 1);
    vspi_trans_in_progress = false;
}

void spi_pre_transfer_callback(spi_transaction_t *t)
{
    uint8_t cs=*((uint8_t*)t->user);
    gpio_set_level(cs, 0);
}


void spi_init(uint8_t clk, uint8_t mosi, uint8_t miso)
{
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = miso,
        .mosi_io_num = mosi,
        .sclk_io_num = clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 1024,
    };
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,
		.duty_cycle_pos = 128,
		.cs_ena_pretrans = 0,
		.cs_ena_posttrans = 0, // Keep the CS low 3 cycles after transaction, to stop the master from missing the last bit when CS has less propagation delay than CLK
        .clock_speed_hz = 1000 * 1000,      //Clock out
        .mode = 0,                               //SPI mode 0
        .spics_io_num = -1,                      //CS pin
        .queue_size = 8,                         //We want to be able to queue transactions at a time
        .pre_cb = spi_pre_transfer_callback, //Callback to be called before a transmission is started, handle CS pin
        .post_cb = spi_post_transfer_callback, //Callback to be called after a transmission has completed, handle CS pin
    };
    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(miso, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(mosi, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(clk, GPIO_PULLUP_ONLY);
    
    //Initialize the SPI bus
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, VSPI_DMA_CHANNEL);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &vspi);
    ESP_ERROR_CHECK(ret);
}

void spi_send(uint8_t *data, uint16_t len, uint8_t* cs)
{
    esp_err_t ret;
    spi_transaction_t trans;
    if (len == 0) return;             //no need to send anything
    while(vspi_trans_in_progress);
    memset(&trans, 0, sizeof(trans));       //Zero out the transaction
    trans.length = len*8;                 //Len is in bytes, transaction length is in bits.
    trans.tx_buffer = data;               //Data
    trans.user = (void*)cs;                //cs needs to be set to 1
    //t.flags = SPI_TRANS_USE_TXDATA;
    vspi_trans_in_progress = true;
    ret = spi_device_transmit(vspi, &trans);  //Transmit!
    ESP_ERROR_CHECK(ret);            //Should have had no issues.  

}


void spi_receive(uint8_t *txbuff, uint16_t txlen, uint8_t *rxbuff, uint16_t rxlen, uint8_t* cs)
{
    spi_transaction_t trans;
    while(vspi_trans_in_progress);
    memset(&trans, 0, sizeof(trans));
    trans.length = (txlen*8)+(rxlen*8);
    trans.tx_buffer = txbuff;
    trans.rx_buffer = rxbuff;
    trans.rxlength = (txlen*8)+(rxlen*8);
    trans.user = (void*)cs;
    vspi_trans_in_progress = true;
    esp_err_t ret = spi_device_transmit(vspi, &trans);
    ESP_ERROR_CHECK(ret);
}
