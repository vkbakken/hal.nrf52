#include "board/pins.h"
#include "cpu/io.h"
#include "hal/hal_spi_master.h"

static TaskHandle_t xTaskToNotify = NULL;

__STATIC_INLINE uint32_t hal_spi_freq_convert(spi_frequency_t freq) {
	switch (freq) {
	case SPI_FREQ_125K:
		return SPIM_FREQUENCY_FREQUENCY_K125;
	case SPI_FREQ_250K:
		return SPIM_FREQUENCY_FREQUENCY_K250;
	case SPI_FREQ_500K:
		return SPIM_FREQUENCY_FREQUENCY_K500;
	case SPI_FREQ_1M:
		return SPIM_FREQUENCY_FREQUENCY_M1;
	case SPI_FREQ_2M:
		return SPIM_FREQUENCY_FREQUENCY_M2;
	case SPI_FREQ_4M:
		return SPIM_FREQUENCY_FREQUENCY_M4;
	case SPI_FREQ_8M:
		return SPIM_FREQUENCY_FREQUENCY_M8;
	}
}

void hal_spi_init(hal_spi_instance_t *spi_instance) {
	NRF_P0->OUTCLR = BOARD_ACCEL_CLK_bm;
	NRF_P0->PIN_CNF[BOARD_ACCEL_CLK_bp] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
										  (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
										  (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
										  (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										  (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->OUTCLR = BOARD_ACCEL_MOSI_bm;
	NRF_P0->PIN_CNF[BOARD_ACCEL_MOSI_bp] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
										   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
										   (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
										   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->PIN_CNF[BOARD_ACCEL_MISO_bp] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
										   (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
										   (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos) |
										   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->OUTCLR = 1UL << spi_instance->SS_pin;
	NRF_P0->PIN_CNF[spi_instance->SS_pin] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
											(GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
											(GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
											(GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
											(GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_SPIM0->PSEL.SCK = BOARD_ACCEL_CLK_bp;
	NRF_SPIM0->PSEL.MOSI = BOARD_ACCEL_MOSI_bp;
	NRF_SPIM0->PSEL.MISO = BOARD_ACCEL_MISO_bp;

	NRF_SPIM0->FREQUENCY = hal_spi_freq_convert(spi_instance->config.frequency);
	NRF_SPIM0->CONFIG = (spi_instance->config.bit_order << SPIM_CONFIG_ORDER_Pos) |
						(spi_instance->config.clock_polarity << SPIM_CONFIG_CPOL_Pos) |
						(spi_instance->config.clock_phase << SPIM_CONFIG_CPHA_Pos);

	NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);

	NRF_SPIM0->INTENSET = SPIM_INTENSET_END_Msk;

	NVIC_SetPriority(SPIM0_SPIS0_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(SPIM0_SPIS0_IRQn);
}

void hal_spi_deinit(hal_spi_instance_t *spi_instance) {
	NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos);

	NRF_P0->PIN_CNF[spi_instance->SS_pin] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
											(GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
											(GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
											(GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
											(GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->PIN_CNF[BOARD_ACCEL_MISO_bp] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
										   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
										   (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
										   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->PIN_CNF[BOARD_ACCEL_MOSI_bp] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
										   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
										   (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
										   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->PIN_CNF[BOARD_ACCEL_CLK_bp] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
										  (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
										  (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
										  (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										  (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NVIC_DisableIRQ(SPIM0_SPIS0_IRQn);
}

bool hal_spi_rw(hal_spi_instance_t *spi_instance, uint8_t *data_w, uint8_t size_w, uint8_t *data_r, uint8_t size_r) {
	bool ret = false;

	NRF_SPIM0->TXD.PTR = (uint32_t)data_w;
	NRF_SPIM0->TXD.MAXCNT = size_w;
	NRF_SPIM0->RXD.PTR = (uint32_t)data_r;
	NRF_SPIM0->RXD.MAXCNT = size_r;

	//select a slave and start spi transaction
	NRF_P0->OUTSET = 1UL << spi_instance->SS_pin;

	NRF_SPIM0->EVENTS_END = 0x0UL;
	NRF_SPIM0->TASKS_START = 0x1UL;

	xTaskToNotify = xTaskGetCurrentTaskHandle();
	if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(spi_instance->timeout)) == 1) {
		ret = true;
	}

	return ret;
}

bool hal_spi_write(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size) {
	return hal_spi_rw(spi_instance, data, size, NULL, 0);
}

bool hal_spi_read(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size) {
	return hal_spi_rw(spi_instance, NULL, 0, data, size);
}

void serialbox0_handler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (NRF_SPIM0->EVENTS_END) {
		NRF_SPIM0->EVENTS_END = 0;
		vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}