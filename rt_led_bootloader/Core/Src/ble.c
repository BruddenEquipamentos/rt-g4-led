/*
 * ble.c
 *
 *  Created on: Mar 28, 2023
 *      Author: fminoru
 */

#include <stdio.h>
#include <string.h>

#include "main.h"

// TEMP
#include "display.h"

extern UART_HandleTypeDef huart3;

static uint16_t crc16(const uint8_t *data_p, uint32_t length, uint16_t crc) {
  uint8_t x;
  while (length--) {
    x = crc >> 8 ^ *data_p++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
  }
  return crc;
}

int ble_read_receive(char *response, const uint32_t timeout) {
  char *rsp = response;
  uint8_t byte;

  while (HAL_UART_Receive(&huart3, &byte, 1, timeout) == HAL_OK) {
    if (*rsp == 0) {
      break;
    } else if (byte == *rsp) {
      rsp++;
    } else {
      rsp = response;
    }
  }
  return *rsp == 0;
}

int ble_read_line(char *buffer, const int max_length, const uint32_t timeout) {
  uint8_t byte = 0;
  int bytes_read = 0;

  while (bytes_read < max_length && HAL_UART_Receive(&huart3, &byte, 1, timeout) == HAL_OK) {
    switch (byte) {
      case 0x02:
        bytes_read = 0;
        break;
      case 0x0A:
	buffer[bytes_read++] = byte;
        return bytes_read;
        break;
      default:
        buffer[bytes_read++] = byte;
        break;
    }
  }

  return bytes_read;
}

int ble_module_flush_rx(void) {
  uint8_t byte;
  while (HAL_UART_Receive(&huart3, &byte, 1, 1) == HAL_OK)
    ;
  return 1;
}

int ble_module_transmit(char *data, uint8_t length, uint32_t timeout) {
  return HAL_UART_Transmit(&huart3, (uint8_t *)data, length, timeout) == HAL_OK;
}

int ble_module_is_connected(void) {
  ble_module_flush_rx();

  ble_module_transmit("\n", 1, 1);

  if (!ble_read_receive("\x02"
                        "AOK\n",
                        1000)) {
    return 0;
  }

  return 1;
}

unsigned int file_size;
uint16_t file_crc16;
uint16_t packet_crc;
int new_file;
volatile uint16_t _crc;

int ble_module_dfu_available(void) {
  char line[128];
  static char *cmd =
      "\x02"
      "rdfu,rtln.bin.meta"
      "\x0A";
  ble_module_transmit(cmd, strlen(cmd), 10);
  if (ble_read_line(line, 128, 500) == 0) return 0;

  volatile int res = sscanf(line,
                            "%x %hx %d"
                            "\x03"
                            "%hx\n",
                            &file_size, &file_crc16, &new_file, &packet_crc);

  if (res == 4) {
    char *etx = strchr(line, 0x03);
    _crc = crc16((uint8_t *)line, (uint32_t)(etx - line), 0xffff);
    if (_crc == packet_crc) {
      if (new_file) {
        return 1;
      }
    }
  }

  return 0;
}

static int erase_flash(const uint32_t start_address, const uint32_t end_address, const uint32_t page_size) {
  assert(start_address > 0x8000000);
  assert(end_address > 0x8000000);
  assert(start_address < end_address);

  uint32_t start_address_length = start_address - 0x8000000;
  uint32_t start_page = (start_address_length / page_size);

  uint32_t end_address_length = end_address - 0x8000000;
  uint32_t end_page = (end_address_length / page_size) - 1;

  //  uint32_t page_bootloader = 0; // 0-7 (0-16)
  //  uint32_t page_app_crc = 8;    // 8 (16-18)
  //  uint32_t page_app = 9;        // 9-49 (18-100)
  //  uint32_t page_app_data = 50;  // 50-63 (100-128)
  // return 0;

  HAL_FLASH_Unlock();
  for (uint32_t page = start_page; page <= end_page; ++page) {
    FLASH_EraseInitTypeDef erase_params;
    uint32_t page_error;
    erase_params.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_params.Banks = FLASH_BANK_1;
    erase_params.NbPages = 1;
    erase_params.Page = page;

    if (HAL_FLASHEx_Erase(&erase_params, &page_error) != HAL_OK) {
      return 0;
    }

    dm_render_progress(page, end_page, 0);
    display_update();
  }
  HAL_FLASH_Lock();

  return 1;
}

volatile uint32_t bytes_written = 0;
HAL_StatusTypeDef _status;
int ble_module_dfu_handle(void) {
  const uint32_t END_ADDRESS = 0x08019000U;  // (Start of APP_Data)
  if (!erase_flash(APPLICATION_ADDRESS, END_ADDRESS, FLASH_PAGE_SIZE)) {
    // TODO: erase flash failed message
    return 0;
  }

  HAL_Delay(1000);

  static char *request_data_command =
      "\x02"
      "gdfu,rtln.bin"
      "\x0A";
  ble_module_transmit(request_data_command, strlen(request_data_command), 100);

  uint8_t buf[288];
  uint16_t file_crc = 0xffff;
  uint32_t address = APPLICATION_ADDRESS;

  while (1) {
    int retry = 3;
    int received = 0;

    while (!received && retry--) {
      if (HAL_UART_Receive(&huart3, buf, 260, 500) == HAL_OK) {
        received = 1;
      } else {
        HAL_UART_Transmit(&huart3, (uint8_t *)"\x15", 1, 1000);
      }
    }

    if (received == 0) {
      __asm("nop\n");
      return 0;
    }

    uint16_t packet_data_length = 0;
    uint16_t packet_crc = 0;
    uint16_t crc = 0;

    packet_data_length = (buf[0] << 8) | (buf[1] & 0xff);
    bytes_written += packet_data_length;

    packet_crc = (buf[258] << 8) | (buf[259] & 0xff);
    crc = crc16(&buf[2], 256, 0xffff);

    if (crc != packet_crc) {
      // TODO: Error message
      return 0;
    }

    {
      uint64_t dword;

      for (uint32_t i = 0; packet_data_length > 0; ++i) {
        if (packet_data_length < 8) {
          dword = 0x0000000000000000;
          memcpy(&dword, &buf[i * 8 + 2], packet_data_length);
          file_crc = crc16((uint8_t *)&dword, packet_data_length, file_crc);
          packet_data_length = 0;
        } else {
          memcpy(&dword, &buf[i * 8 + 2], 8);
          packet_data_length -= 8;
          file_crc = crc16((uint8_t *)&dword, 8, file_crc);
        }

        HAL_FLASH_Unlock();
        //        HAL_Delay(75);
        _status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, dword);
        HAL_FLASH_Lock();

        if (_status == HAL_OK) {
          address += 8;
        } else {
          // TODO: Error message
          HAL_FLASH_GetError();
          HAL_FLASH_Lock();
          return 0;
        }
      }

      dm_render_progress_dfu(bytes_written, file_size);
      display_update();
    }

    if (bytes_written >= file_size) break;

    HAL_UART_Transmit(&huart3, (uint8_t *)"\x06", 1, 100);
  }

  // if reached update, then update
  if (file_crc == file_crc16) {
    erase_flash(APPLICATION_ADDRESS - FLASH_PAGE_SIZE, APPLICATION_ADDRESS, FLASH_PAGE_SIZE);
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, APPLICATION_ADDRESS - FLASH_PAGE_SIZE, (uint64_t)file_crc);
    HAL_FLASH_Lock();


    HAL_Delay(1000);
    HAL_UART_Transmit(&huart3, (uint8_t *)"\x0A", 1, 100);
    static char *update = "\x02""set rtln.bin.meta new=0""\x0A";
    HAL_UART_Transmit(&huart3, (uint8_t *)update, strlen(update), 100);
    HAL_Delay(1000);
  }

  return 1;
}
