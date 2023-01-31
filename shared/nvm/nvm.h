#ifndef NVM_H
#define NVM_H

#include <stdint.h>

#define VIRTADDRESS_START 1
#define VIRTADDRESS_END (0xFFFF-1)

typedef struct NVM {
  int (*NVM_Format)(void);
  int (*NVM_Init)(void);
  int (*NVM_Close)(void);
  int (*NVM_ReadVariable32bits)(uint16_t VirtAddress, uint32_t *pData);
  int (*NVM_WriteVariable32bits)(uint16_t VirtAddress, uint32_t Data);
  int (*NVM_ReadVariable16bits)(uint16_t VirtAddress, uint16_t *pData);
  int (*NVM_WriteVariable16bits)(uint16_t VirtAddress, uint16_t Data);
  int (*NVM_ReadVariable8bits)(uint16_t VirtAddress, uint8_t *pData);
  int (*NVM_WriteVariable8bits)(uint16_t VirtAddress, uint8_t Data);
} NVM_t;

extern NVM_t nvm;

#endif
