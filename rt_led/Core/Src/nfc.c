#include "nfc.h"

#include "st25dx_discovery_nfctag.h"
#include "lib_NDEF.h"
#include "lib_wrapper.h"
#include "tagtype5_wrapper.h"
#include "lib_NDEF_URI.h"

#include "st25dx_discovery_nfctag.h"

#include "commonfunc.h"

#include "lib_NDEF_Bluetooth.h"

extern volatile uint8_t GPO_Activated;

static ST25DV_Object_t NfcTagObj;

#define NDEF_DEMO_CC3_COMPLIANT_VALUE     ((uint8_t)0x5)
#define NDEF_DEMO_URL                     "treadmillreviews.com"

static uint16_t getNDEFRecord(sRecordInfo_t *p_record)
{
  uint16_t status;
  static uint8_t *p_ndef_buffer;

#if (NDEF_DEMO_USE_MALLOC == 1)
  uint16_t ndef_size;
  /* allocate the ndef buffer */
  status = NDEF_getNDEFSize(&ndef_size);
  if(status != NDEF_OK)
  {
    return status;
  }

  if(ndef_buffer != NULL)
  {
    free(ndef_buffer);
    ndef_buffer = NULL;
  }
  ndef_buffer = malloc(ndef_size);
  if(ndef_buffer == NULL)
  {
    return NDEF_ERROR_MEMORY_INTERNAL;
  }
#else /* (NDEF_DEMO_USE_MALLOC != 1) */
  p_ndef_buffer = NDEF_Buffer;
#endif /* (NDEF_DEMO_USE_MALLOC == 1) */

  status = NDEF_ReadNDEF(p_ndef_buffer);
  if(status != NDEF_OK)
  {
    return status;
  }
  status = NDEF_IdentifyBuffer(p_record,p_ndef_buffer);

  return status;
}


static uint16_t getURI(sURI_Info * r_uri)
{
	sRecordInfo_t record;
 	uint16_t status;

  /* To avoid displaying random memory content in case URI part is missing */
  r_uri->Information[0] = '\0';
  r_uri->protocol[0] = '\0';
  r_uri->URI_Message[0] = '\0';

	status= getNDEFRecord(&record);
  if(status != NDEF_OK)
  {
    return status;
  }

  status = NDEF_ReadURI(&record, r_uri);

  return status;

}

///**
//  * @brief Display an error message depending on the return value of a NFCTAG driver function.
//  * @param status Return value from a NFCTAG driver function.
//  */
//void st25_error(int32_t status)
//{
//  uint32_t error_index = 5;
//  char titles[6][20] = {
//                          "",                   /* Success, so no message displayed! */
//                          "ST25DV I2C Error",
//                          "ST25DV I2C Busy",
//                          "ST25DV I2C Timeout",
//                          "ST25DV I2C NACK",
//                          "ST25DV I2C Unknown"
//                        };
//
//  switch(status)
//  {
//    case NFCTAG_OK:
//      error_index = 0;
//      break;
//
//    case NFCTAG_ERROR:
//      error_index = 1;
//      break;
//
//    case NFCTAG_BUSY:
//      error_index = 2;
//      break;
//
//    case NFCTAG_TIMEOUT:
//      error_index = 3;
//      break;
//
//    case NFCTAG_NACK:
//      error_index = 4;
//      break;
//
//    default:
//      error_index = 5;
//      break;
//  }
//}

void NDEF_Init_Tag(void)
{
    /* Tag configuration for NDEF */
//    BSP_LPD_WritePin(GPIO_PIN_RESET);
    PresentPasswd( true );

    /* Disable Mailbox mode */
//    DeInitMailBoxMode();

    /* Activate RF */
    BSP_NFCTAG_WriteRFMngt(BSP_NFCTAG_INSTANCE, 0);

    /* Disable Energy Harvesting */
    BSP_NFCTAG_ResetEHENMode_Dyn(BSP_NFCTAG_INSTANCE);

    /* De-activate any protection */
    uint32_t mem_size = BSP_NFCTAG_GetByteSize(BSP_NFCTAG_INSTANCE);
    ST25DV_PROTECTION_ZONE ZoneRF;
    ST25DV_RF_PROT_ZONE no_rf_prot = {.PasswdCtrl = ST25DV_NOT_PROTECTED , .RWprotection = ST25DV_NO_PROT};
    for(ZoneRF = ST25DV_PROT_ZONE1; ZoneRF <= ST25DV_PROT_ZONE4 ; ZoneRF++)
    {
      ST25_RETRY(BSP_NFCTAG_WriteRFZxSS(BSP_NFCTAG_INSTANCE, ZoneRF, no_rf_prot));

      ST25_RETRY(BSP_NFCTAG_WriteI2CProtectZonex(BSP_NFCTAG_INSTANCE, ZoneRF, ST25DV_NO_PROT));
    }
    ST25_RETRY(BSP_NFCTAG_InitEndZone(BSP_NFCTAG_INSTANCE));

    ST25_RETRY(BSP_NFCTAG_WriteLockCCFile(BSP_NFCTAG_INSTANCE, ST25DV_CCFILE_2BLCK, ST25DV_UNLOCKED));

    NfcTag_SelectProtocol(NFCTAG_TYPE5);

    if(NfcType5_NDEFDetection() != NDEF_OK)
    {
      /* Write CC file content */
      CCFileStruct.MagicNumber  = NFCT5_MAGICNUMBER_E1_CCFILE;
      CCFileStruct.Version      = NFCT5_VERSION_V1_0;
      /* Size must be specified by block of 8 bytes */
      if(mem_size > (0xFF*8))
      {
        CCFileStruct.MagicNumber  = NFCT5_MAGICNUMBER_E2_CCFILE;
        CCFileStruct.MemorySize   = NFCT5_EXTENDED_CCFILE;
        CCFileStruct.ExtMemorySize   = (uint16_t)(mem_size / 8);

      } else {
        CCFileStruct.MemorySize   = (uint8_t)(mem_size / 8);
      }
      CCFileStruct.TT5Tag       =  NDEF_DEMO_CC3_COMPLIANT_VALUE;

      if(NfcType5_TT5Init() != NDEF_OK)
      {
//        Menu_MsgStatus("No CCFILE","CCFILE init failed!",MSG_STATUS_ERROR);
      }
    }
}

void NDEF_Write_URI_URL(void)
{
	uint16_t status;
	sURI_Info r_uri = {0};

	NDEF_Init_Tag();

	/* Write URI for ST25 products web page */
	sURI_Info w_uri = {URI_ID_0x01_STRING, NDEF_DEMO_URL ,""};
	status = NDEF_WriteURI(&w_uri);

	/* Read back written URI */
	if(status == NDEF_OK)
	{
		status = getURI(&r_uri);
	}

	if((status != NDEF_OK) || \
			strcmp(r_uri.protocol,w_uri.protocol) || \
			strcmp(r_uri.URI_Message,w_uri.URI_Message))
	{
//		Menu_MsgStatus("Store URI failure!","URL has not been written!",MSG_STATUS_ERROR);
	}
	else
	{
		char msg[200] = "The URL has been written:\n";
		strcat(msg,URI_ID_0x01_STRING);
		strcat(msg,NDEF_DEMO_URL);
		strcat(msg,"\n\nPlease read it with a phone!");
//		Menu_MsgStatus("Store URI success!",msg,MSG_STATUS_SUCCESS);
	}
}


/*********************************** Bluetooth Low Energy *****************************************/
/** Bluetooth module name */
#define NDEF_DEMO_BLE_NAME                "HID"

/** Bluetooth Device Address big endian (default value, partly overriden to uniquify the DeviceAddress) */
#define NDEF_DEMO_BLE_ADDR_BIG_ENDIAN     {0x02, 0x80, 0xE1, 0x00, 0x34, 0x12}


/* Set OOB values aligned with BLE_Init */
  Ndef_Bluetooth_OOB_t BleOOB = {.Type = NDEF_BLUETOOTH_BLE,
                                    .DeviceAddress = NDEF_DEMO_BLE_ADDR_BIG_ENDIAN,
                                    .DeviceAddressType = NDEF_BLE_PUBLIC_ADDRESS_TYPE,
                                    .Role = NDEF_BLE_ROLE_PERIPH_ONLY,
                                    .OptionalMask = NDEF_BLUETOOTH_OPTION(BLUETOOTH_EIR_COMPLETE_LOCAL_NAME),
                                    .LocalName = NDEF_DEMO_BLE_NAME,
                                    };

/**
  * @brief  This function writes a Bluetooth Low Energy OOB record to the Tag and starts the BLE module, waiting for a connection to occur.
  */
void NDEF_Write_BLE_OOB(void)
{
  uint16_t status;
  static uint8_t get_unique_id = 1;

  if(get_unique_id == 1)
  {
    /* Make sure, that initial DeviceAddress is different between boards at reset, using the STM32 Unique Id LSB */
    static uint8_t addr[3] = {0xff, 0xaa, 0x55};
    memcpy(&BleOOB.DeviceAddress[3],addr,3);
    get_unique_id = 0;
  }

  NDEF_Init_Tag();
  /* Clear NDEF before adding OOB */
  NDEF_ClearNDEF();

  /* Write the OOB to the Tag */
	status = NDEF_AppendBluetoothOOB ( &BleOOB, NULL );

  if(status != NDEF_OK)
  {
//    Menu_MsgStatus("Store OOB failure!","OOB has not been written!",MSG_STATUS_ERROR);
    return;
	}
  else
  {
//    Menu_MsgStatus("Store OOB success!","OOB has been written!\n\nPlease read it with a phone!",MSG_INFO);
//    Menu_DisplayCenterString(Line10,"Wait connection...");
  }

   /* Start BLE module */
  PresentPasswd(true);
  InitITGPOMode(ST25DV_GPO_RFACTIVITY_MASK | ST25DV_GPO_ENABLE_MASK);

    /* Start BLE module */
  while(1)
  {
//    if(Menu_GetUserInput())
//    {
//      break;
//    }
    if(GPO_Activated)
    {
//      BLE_Init(BleOOB.DeviceAddress);
      GPO_Activated = 0;
      break;
    }
  }
  DeInitITGPOMode();
/* Prevent connection outside the demo. */
//  HAL_GPIO_WritePin(BNRG_SPI_RESET_PORT, BNRG_SPI_RESET_PIN, GPIO_PIN_RESET);

}

