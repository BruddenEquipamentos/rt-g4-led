#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../shared/nvm/nvm.h"

static FILE *fp = NULL;
static const char *filename = "data.dat";

static NVM_Status_t NVM_Format(void) {
  fp = fopen(filename, "w");
  if (fp == NULL) return NVM_STATUS_ERR;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_Init(void) {
  fp = fopen(filename, "r+");
  if (fp == NULL) return NVM_Format();
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_Close(void) {
  if (fp != NULL) fclose(fp);
  fp = NULL;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_WriteVariable32bits(uint16_t VirtAddress,
                                            uint32_t Data) {
  char line[19];
  char prefix[8];
  if (fp == NULL) NVM_Init();
  snprintf(prefix, 8, "0x%04X,", VirtAddress);
  fseek(fp, 0, SEEK_SET);
  while ((fgets(line, 19, fp)) != NULL) {
    if (strncmp(line, prefix, 7) == 0) {
      fseek(fp, -18, SEEK_CUR);
      fprintf(fp, "0x%04X,%10u\n", VirtAddress, Data);
      NVM_Close();
      return NVM_STATUS_OK;
    }
  }
  fprintf(fp, "0x%04X,%10u\n", VirtAddress, Data);
  NVM_Close();
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_WriteVariable16bits(uint16_t VirtAddress,
                                            uint16_t Data) {
  return NVM_WriteVariable32bits(VirtAddress, (uint32_t)Data);
}

static NVM_Status_t NVM_WriteVariable8bits(uint16_t VirtAddress, uint8_t Data) {
  return NVM_WriteVariable32bits(VirtAddress, (uint32_t)Data);
}

static NVM_Status_t NVM_ReadVariable32bits(uint16_t VirtAddress,
                                           uint32_t *pData) {
  uint16_t address;
  uint32_t value;
  if (fp == NULL) NVM_Init();
  fseek(fp, 0, SEEK_SET);
  char line[80];
  while ((fgets(line, 80, fp)) != NULL) {
    if (sscanf(line, "%hx,%u", &address, &value) == 2) {
      if (address == VirtAddress) {
        *pData = value;
        NVM_Close();
        return NVM_STATUS_OK;
      }
    }
  }
  NVM_Close();
  return NVM_STATUS_ERR;
}

static NVM_Status_t NVM_ReadVariable16bits(uint16_t VirtAddress,
                                           uint16_t *pData) {
  uint32_t value;
  if (NVM_ReadVariable32bits(VirtAddress, &value)) {
    *pData = (uint16_t)value;
    return NVM_STATUS_OK;
  }
  return NVM_STATUS_ERR;
}

static NVM_Status_t NVM_ReadVariable8bits(uint16_t VirtAddress,
                                          uint8_t *pData) {
  uint32_t value;
  if (NVM_ReadVariable32bits(VirtAddress, &value)) {
    *pData = (uint8_t)value;
    return NVM_STATUS_OK;
  }
  return NVM_STATUS_ERR;
}

// TODO: Optimize by read/write 32 bits (4 chars per VirtAddress)
static NVM_Status_t NVM_ReadString(uint16_t VirtAddress, uint8_t *pData,
                                   uint32_t length) {
  uint8_t data;
  for (uint32_t i = 0; i < length; ++i) {
    if (nvm.NVM_ReadVariable8bits(VirtAddress + i, &data)) {
      *pData++ = data;
    } else {
      return NVM_STATUS_ERR;
    }
  }
  return NVM_STATUS_OK;
}

// TODO: Optimize by read/write 32 bits (4 chars per VirtAddress)
static NVM_Status_t NVM_WriteString(uint16_t VirtAddress, const char *data,
                                    uint32_t length, uint32_t max_length) {
  if (length > max_length) return 0;
  for (uint32_t i = 0; i < max_length; ++i) {
    if (i >= length)
      nvm.NVM_WriteVariable8bits(VirtAddress + i, 0);
    else
      nvm.NVM_WriteVariable8bits(VirtAddress + i, (uint8_t)data[i]);
  }
  return NVM_STATUS_OK;
}

void NVM_Setup(void) {
  nvm.NVM_Format = NVM_Format;
  nvm.NVM_Init = NVM_Init;
  nvm.NVM_Close = NVM_Close;
  nvm.NVM_ReadVariable32bits = NVM_ReadVariable32bits;
  nvm.NVM_WriteVariable32bits = NVM_WriteVariable32bits;
  nvm.NVM_ReadVariable16bits = NVM_ReadVariable16bits;
  nvm.NVM_WriteVariable16bits = NVM_WriteVariable16bits;
  nvm.NVM_ReadVariable8bits = NVM_ReadVariable8bits;
  nvm.NVM_WriteVariable8bits = NVM_WriteVariable8bits;
  nvm.NVM_ReadString = NVM_ReadString;
  nvm.NVM_WriteString = NVM_WriteString;
}
