#include "nvm.h"

NVM_Status_t NVM_Format(void) { return NVM_STATUTS_NOT_IMPLEMENTED; }
NVM_Status_t NVM_Init(void) { return NVM_STATUTS_NOT_IMPLEMENTED; }
NVM_Status_t NVM_Close(void) { return NVM_STATUTS_NOT_IMPLEMENTED; }
NVM_Status_t NVM_ReadVariable32bits(uint16_t VirtAddress, uint32_t *pData) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}
NVM_Status_t NVM_WriteVariable32bits(uint16_t VirtAddress, uint32_t Data) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}
NVM_Status_t NVM_ReadVariable16bits(uint16_t VirtAddress, uint16_t *pData) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}
NVM_Status_t NVM_WriteVariable16bits(uint16_t VirtAddress, uint16_t Data) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}
NVM_Status_t NVM_ReadVariable8bits(uint16_t VirtAddress, uint8_t *pData) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}
NVM_Status_t NVM_WriteVariable8bits(uint16_t VirtAddress, uint8_t Data) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}
NVM_Status_t NVM_ReadString(uint16_t VirtAddress, uint8_t *pData,
                            uint32_t length) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}
NVM_Status_t NVM_WriteString(uint16_t VirtAddress, const char *data, uint32_t length,
                             uint32_t max_length) {
  return NVM_STATUTS_NOT_IMPLEMENTED;
}

NVM_t nvm = {
    .NVM_Format = NVM_Format,
    .NVM_Init = NVM_Init,
    .NVM_Close = NVM_Close,
    .NVM_ReadVariable32bits = NVM_ReadVariable32bits,
    .NVM_WriteVariable32bits = NVM_WriteVariable32bits,
    .NVM_ReadVariable16bits = NVM_ReadVariable16bits,
    .NVM_WriteVariable16bits = NVM_WriteVariable16bits,
    .NVM_ReadVariable8bits = NVM_ReadVariable8bits,
    .NVM_WriteVariable8bits = NVM_WriteVariable8bits,
    .NVM_ReadString = NVM_ReadString,
    .NVM_WriteString = NVM_WriteString,
};
