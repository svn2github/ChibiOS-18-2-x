/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "sama_sdmmc_lld.h"
#include "chprintf.h"


#define BLOCK_CNT_MAX               32u
#define DMADL_CNT_MAX               64u
#define BLOCK_CNT                   3u



const char test_file_path[] = "test.txt";

CACHE_ALIGNED uint8_t data_buf[BLOCK_CNT_MAX * 512ul];

CACHE_ALIGNED static uint32_t dma_table[DMADL_CNT_MAX * SDMMC_DMADL_SIZE];

CACHE_ALIGNED uint8_t sdmmcbuffer[ROUND_UP_MULT(SDMMC_BUFFER_SIZE, L1_CACHE_BYTES)];

static FATFS fs_header;
static FIL f_header;

BaseSequentialStream * ts;

static const SerialConfig sdcfg = { 115200, 0,UART_MR_PAR_NO };

/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 512);
static THD_FUNCTION(Thread1, arg) {

	(void) arg;
	chRegSetThreadName("blinker");

	while (true) {
		palToggleLine(LINE_LED_BLUE);
		chThdSleepMilliseconds(500);
	}
}



static const SamaSDMMCConfig sdmmc_slot1_cfg = {
		SDMMC_SLOT1,
		sdmmcbuffer,
		data_buf,
		sizeof(data_buf),
		dma_table,
		DMADL_CNT_MAX
};

void getdir(SdmmcDriver *sdmmcp);
void writefile(SdmmcDriver *sdmmcp);
void readfile(SdmmcDriver *sdmmcp);
/*
 * Application entry point.
 */
int main(void) {

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	/*
	 * Activates the serial driver 0 using the driver default configuration.
	 */
	sdStart(&SD1, &sdcfg);

	/* Redirecting  UART0 RX on PD2 and UART0 TX on PD3. */
	palSetGroupMode(PIOD, PAL_PORT_BIT(2) | PAL_PORT_BIT(3), 0U,
			PAL_SAMA_FUNC_PERIPH_A | PAL_MODE_SECURE);

	ts = (BaseSequentialStream *) &SD1;

	sdmmcInit();

	sdmmcStart(&SDMMCD1, &sdmmc_slot1_cfg);

	if (SDMMCD1.state != MCID_INIT_ERROR) {

		chprintf(ts,"starting MMC Demo\n\r");

		if (sdmmcOpenDevice(&SDMMCD1)) {

			//sdmmcShowDeviceInfo(&SDMMCD1);
			if ( sdmmcMountVolume(&SDMMCD1,&fs_header) ) {
				writefile(&SDMMCD1);
				chprintf(ts,"reading dir:\n\r");
				getdir(&SDMMCD1);
				readfile(&SDMMCD1);

			}
			sdmmcCloseDevice(&SDMMCD1);
		}


	} else {
		chprintf(ts,"error,stopping MMC\n\r");
		sdmmcStop(&SDMMCD1);
	}

	/*
	 * Creates the blinker thread.
	 */
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	/*
	 * Normal main() thread activity, in this demo it does nothing except
	 * sleeping in a loop and check the button state.
	 */
	while (true) {
		chThdSleepMilliseconds(500);
	}
}


bool sdmmcGetInstance(uint8_t index, SdmmcDriver **sdmmcp)
{
	if (index == SDMMCD1.config->slot_id) {
		*sdmmcp = &SDMMCD1;
		return true;
	}
	return false;
}

void writefile(SdmmcDriver *sdmmcp)
{
	const TCHAR drive_path[] = { '0' + sdmmcp->config->slot_id, ':', '\0' };
	TCHAR file_path[sizeof(drive_path) + sizeof(test_file_path)];
	FRESULT res;
	UINT len;
	uint8_t buffer[]={	0x57,0x65,0x6C,0x63,0x6F,
						0x6D,0x65,0x20,0x74,0x6F,
						0x20,0x43,0x68,0x69,0x62,
						0x69,0x4F,0x53,0x21};


	strcpy(file_path, drive_path);
	strcat(file_path, test_file_path);

	chprintf(ts,"Creating new file ... ");
	res = f_open(&f_header, file_path, FA_WRITE | FA_CREATE_ALWAYS);
	if (res == FR_OK) {
		chprintf(ts,"OK\r\n");
		res = f_write(&f_header, buffer, 19, &len);
		if (res == FR_OK) {
			chprintf(ts,"written %d bytes\n\r", len);
		}
	}
	else
	{
		chprintf(ts,"Failed error %d\n\r", res);
	}
	f_close(&f_header);

}

void readfile(SdmmcDriver *sdmmcp)
{
	const TCHAR drive_path[] = { '0' + sdmmcp->config->slot_id, ':', '\0' };
	TCHAR file_path[sizeof(drive_path) + sizeof(test_file_path)];
	FRESULT res;
	UINT len;
	uint8_t buffer[19];
	UINT i;

	strcpy(file_path, drive_path);
	strcat(file_path, test_file_path);

	chprintf(ts,"Reading back the new file ... ");
	res = f_open(&f_header, file_path, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		chprintf(ts,"OK\r\n");
		res = f_read(&f_header, buffer, 19, &len);
		if (res == FR_OK) {
			chprintf(ts,"read %d bytes\n\r", len);
			for (i=0;i<len;i++) {
				chprintf(ts,"%c", buffer[i]);
			}
		}
	}
	else
	{
		chprintf(ts,"Failed error %d\n\r", res);
	}
	f_close(&f_header);

}

void getdir(SdmmcDriver *sdmmcp)
{
	const TCHAR drive_path[] = { '0' + sdmmcp->config->slot_id, ':', '\0' };
	DIR dir = { .sect = 0 };
	FILINFO fno = { 0 };
	FRESULT res;
	bool is_dir;

	res = f_opendir(&dir, drive_path);
	if (res != FR_OK) {
		chprintf(ts,"Failed to open dir, error %d\n\r", res);
		return;
	}
	chprintf(ts,"files in the root directory:\n\r");
	for (;;) {
		res = f_readdir(&dir, &fno);
		if (res != FR_OK) {
			chprintf(ts,"cannot read dir, error (%d)\n\r", res);
			break;
		}
		if (fno.fname[0] == '\0')
			break;
		is_dir = fno.fattrib & AM_DIR ? true : false;
		chprintf(ts,"    %s%s%c\n\r", is_dir ? "[" : "", fno.fname,
				is_dir ? ']' : ' ');
	}

	res = f_closedir(&dir);
}



