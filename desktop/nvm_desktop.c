#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../shared/nvm/nvm.h"

static FILE *fp = NULL;
static const char *filename = "data.dat";

NVM_t nvm;

static int NVM_Format(void) {
  fp = fopen(filename, "w");
  if (fp == NULL) return 0;
  return 1;
}

static int NVM_Init(void) {
  fp = fopen(filename, "r+");
  if (fp == NULL) return NVM_Format();
  return 1;
}

static int NVM_Close(void) {
  if (fp != NULL) fclose(fp);
  fp = NULL;
  return 1;
}

static int NVM_WriteVariable32bits(uint16_t VirtAddress, uint32_t Data) {
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
      return 1;
    }
  }
  fprintf(fp, "0x%04X,%10u\n", VirtAddress, Data);
  NVM_Close();
  return 1;
}

static int NVM_WriteVariable16bits(uint16_t VirtAddress, uint16_t Data) {
  return NVM_WriteVariable32bits(VirtAddress, (uint32_t)Data);
}

static int NVM_WriteVariable8bits(uint16_t VirtAddress, uint8_t Data) {
  return NVM_WriteVariable32bits(VirtAddress, (uint32_t)Data);
}

static int NVM_ReadVariable32bits(uint16_t VirtAddress, uint32_t *pData) {
  uint16_t address;
  uint32_t value;
  if (fp == NULL) NVM_Init();
  fseek(fp, 0, SEEK_SET);
  char line[80];
  while ((fgets(line, 80, fp)) != NULL) {
    if (sscanf(line, "%hx,%d", &address, &value) == 2) {
      if (address == VirtAddress) {
        *pData = value;
        NVM_Close();
        return 1;
      }
    }
  }
  NVM_Close();
  return 0;
}

static int NVM_ReadVariable16bits(uint16_t VirtAddress, uint16_t *pData) {
  uint32_t value;
  if (NVM_ReadVariable32bits(VirtAddress, &value)) {
    *pData = (uint16_t)value;
    return 1;
  }
  return 0;
}

static int NVM_ReadVariable8bits(uint16_t VirtAddress, uint8_t *pData) {
  uint32_t value;
  if (NVM_ReadVariable32bits(VirtAddress, &value)) {
    *pData = (uint8_t)value;
    return 1;
  }
  return 0;
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
}
