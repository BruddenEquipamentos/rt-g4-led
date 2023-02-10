#ifndef NVM_H
#define NVM_H

#include <stdint.h>

typedef enum NVM_Status {
  NVM_STATUS_ERR,
  NVM_STATUS_OK,
  NVM_STATUTS_NOT_IMPLEMENTED
} NVM_Status_t;

typedef struct NVM {
  NVM_Status_t (*NVM_Format)(void);
  NVM_Status_t (*NVM_Init)(void);
  NVM_Status_t (*NVM_Close)(void);
  NVM_Status_t (*NVM_ReadVariable32bits)(uint16_t VirtAddress, uint32_t *pData);
  NVM_Status_t (*NVM_WriteVariable32bits)(uint16_t VirtAddress, uint32_t Data);
  NVM_Status_t (*NVM_ReadVariable16bits)(uint16_t VirtAddress, uint16_t *pData);
  NVM_Status_t (*NVM_WriteVariable16bits)(uint16_t VirtAddress, uint16_t Data);
  NVM_Status_t (*NVM_ReadVariable8bits)(uint16_t VirtAddress, uint8_t *pData);
  NVM_Status_t (*NVM_WriteVariable8bits)(uint16_t VirtAddress, uint8_t Data);
  NVM_Status_t (*NVM_ReadString)(uint16_t VirtAddress, uint8_t *pData, uint32_t length);
  NVM_Status_t (*NVM_WriteString)(uint16_t VirtAddress, const char *data, uint32_t length,
                         uint32_t max_length);
} NVM_t;

extern NVM_t nvm;

#endif
