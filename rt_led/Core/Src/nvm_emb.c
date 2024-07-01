#include "nvm_emb.h"
#include "eeprom_emul.h"
#include "eeprom_emul_types.h"
// eeprom emulator

//NVM_t nvm;
EE_Status ee_status;
uint32_t pages_number_ = 0;

static NVM_Status_t NVM_Format(void) {
  if ((ee_status = EE_Init(EE_FORCED_ERASE)) != EE_OK) return NVM_STATUS_ERR;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_Init(void) {
  pages_number_ = PAGES_NUMBER;
  if ((ee_status = EE_Init(EE_CONDITIONAL_ERASE)) != EE_OK) return NVM_STATUS_ERR;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_Close(void) {
  ee_status = EE_OK;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_WriteVariable32bits(uint16_t VirtAddress, uint32_t Data) {
  EE_Status status = EE_WriteVariable32bits(VirtAddress, Data);
  if ((status & EE_STATUSMASK_CLEANUP) == EE_STATUSMASK_CLEANUP) {
    EE_CleanUp();
    status = EE_WriteVariable32bits(VirtAddress, Data);
  }
  ee_status = status;
  if (ee_status != EE_OK) return NVM_STATUS_ERR;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_WriteVariable16bits(uint16_t VirtAddress, uint16_t Data) {
  return NVM_WriteVariable32bits(VirtAddress, (uint32_t)Data);
}

static NVM_Status_t NVM_WriteVariable8bits(uint16_t VirtAddress, uint8_t Data) {
  return NVM_WriteVariable32bits(VirtAddress, (uint8_t)Data);
}

static NVM_Status_t NVM_ReadVariable32bits(uint16_t VirtAddress, uint32_t *pData) {

  if ((ee_status = EE_ReadVariable32bits(VirtAddress, pData)) != EE_OK) return NVM_STATUS_ERR;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_ReadVariable16bits(uint16_t VirtAddress, uint16_t *pData) {
  if ((ee_status = EE_ReadVariable16bits(VirtAddress, pData) != EE_OK)) return NVM_STATUS_ERR;
  return NVM_STATUS_OK;
}

static NVM_Status_t NVM_ReadVariable8bits(uint16_t VirtAddress, uint8_t *pData) {
  if ((ee_status = EE_ReadVariable8bits(VirtAddress, pData) != EE_OK)) return NVM_STATUS_ERR;
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
}
