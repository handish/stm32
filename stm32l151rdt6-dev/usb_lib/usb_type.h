// Define to prevent recursive inclusion
#ifndef __USB_TYPE_H
#define __USB_TYPE_H


// Simple boolean
#ifndef BOOL
#define BOOL
typedef enum {
	FALSE = 0,
	TRUE  = !FALSE
} bool;
#endif

// Null pointer
#ifndef NULL
#define NULL ((void *)0)
#endif

// Exported types
typedef enum _EP_DBUF_DIR {
	// double buffered endpoint direction
	EP_DBUF_ERR,
	EP_DBUF_OUT,
	EP_DBUF_IN
} EP_DBUF_DIR;

// endpoint buffer number
enum EP_BUF_NUM {
	EP_NOBUF,
	EP_BUF0,
	EP_BUF1
};

// Type of recipient
typedef enum _RECIPIENT_TYPE {
	DEVICE_RECIPIENT,     // Recipient device
	INTERFACE_RECIPIENT,  // Recipient interface
	ENDPOINT_RECIPIENT,   // Recipient endpoint
	OTHER_RECIPIENT
} RECIPIENT_TYPE;

// Standard USB requests
typedef enum _STANDARD_REQUESTS {
	GET_STATUS        = 0,
	CLEAR_FEATURE,
	RESERVED1,
	SET_FEATURE,
	RESERVED2,
	SET_ADDRESS,
	GET_DESCRIPTOR,
	SET_DESCRIPTOR,
	GET_CONFIGURATION,
	SET_CONFIGURATION,
	GET_INTERFACE,
	SET_INTERFACE,
	TOTAL_sREQUEST,         // Total number of Standard request
	SYNCH_FRAME       = 12
} STANDARD_REQUESTS;

// Definition of "USBwValue"
typedef enum _DESCRIPTOR_TYPE {
	DEVICE_DESCRIPTOR    = 1,
	CONFIG_DESCRIPTOR,
	STRING_DESCRIPTOR,
	INTERFACE_DESCRIPTOR,
	ENDPOINT_DESCRIPTOR
} DESCRIPTOR_TYPE;

// Feature selector of a SET_FEATURE or CLEAR_FEATURE
typedef enum _FEATURE_SELECTOR {
	ENDPOINT_STALL,
	DEVICE_REMOTE_WAKEUP
} FEATURE_SELECTOR;

// The state machine states of a control pipe
typedef enum _CONTROL_STATE {
	WAIT_SETUP,       // 0
	SETTING_UP,       // 1
	IN_DATA,          // 2
	OUT_DATA,         // 3
	LAST_IN_DATA,     // 4
	LAST_OUT_DATA,    // 5
	WAIT_STATUS_IN,   // 7
	WAIT_STATUS_OUT,  // 8
	STALLED,          // 9
	PAUSE             // 10
} CONTROL_STATE;

// All the request process routines return a value of this type
//   If the return value is not SUCCESS or NOT_READY,
//   the software will STALL the correspond endpoint
typedef struct OneDescriptor {
	uint8_t *Descriptor;
	uint16_t Descriptor_Size;
} ONE_DESCRIPTOR, *PONE_DESCRIPTOR;

// USB function result
typedef enum _RESULT {
	USB_SUCCESS = 0,    // Process successfully
	USB_ERROR,
	USB_UNSUPPORT,
	USB_NOT_READY       // The process has not been finished, endpoint will be NAK to further request
} RESULT;

// Endpoint definition
typedef struct _ENDPOINT_INFO {
// When send data out of the device,
//   CopyData() is used to get data buffer 'Length' bytes data
//   if Length is 0, CopyData() returns the total length of the data
//   if the request is not supported, returns 0
//   if CopyData() returns -1, the calling routine should not proceed
//   further and will resume the SETUP process by the class device
//   if Length is not 0, CopyData() returns a pointer to indicate the data location
//   Usb_wLength is the data remain to be sent,
//   Usb_wOffset is the offset of original data
// When receive data from the host,
//   CopyData() is used to get user data buffer which is capable
//   of Length bytes data to copy data from the endpoint buffer.
//   if Length is 0, CopyData() returns the available data length,
//   if Length is not 0, CopyData() returns user buffer address
//   Usb_wLength is the data remain to be received,
//   Usb_wOffset is the offset of data buffer
	uint16_t Usb_wLength;
	uint16_t Usb_wOffset;
	uint16_t PacketSize;
	uint8_t  *(*CopyData)(uint16_t Length);
} ENDPOINT_INFO;

// USB device
typedef struct _DEVICE {
	uint8_t Total_Endpoint;      // Number of endpoints that are used
	uint8_t Total_Configuration; // Number of configuration available
} DEVICE;

// Structure for access to word value as two bytes and vice versa
typedef union {
	uint16_t w;
	struct BW {
		uint8_t bb1;
		uint8_t bb0;
	} bw;
} uint16_t_uint8_t;

// USB device information
typedef struct _DEVICE_INFO {
	uint8_t USBbmRequestType;         // bmRequestType
	uint8_t USBbRequest;              // bRequest
	uint16_t_uint8_t USBwValues;      // wValue
	uint16_t_uint8_t USBwIndexs;      // wIndex
	uint16_t_uint8_t USBwLengths;     // wLength
	uint8_t ControlState;             // of type CONTROL_STATE
	uint8_t Current_Feature;
	uint8_t Current_Configuration;    // Selected configuration
	uint8_t Current_Interface;        // Selected interface of current configuration
	uint8_t Current_AlternateSetting; // Selected Alternate Setting of current interface
	ENDPOINT_INFO Ctrl_Info;
} DEVICE_INFO;

// USB device properties
typedef struct _DEVICE_PROP {
	void (*Init)(void);        // Initialize the device
	void (*Reset)(void);       // Reset routine of this device

	// Device dependent process after the status stage
	void (*Process_Status_IN)(void);
	void (*Process_Status_OUT)(void);

	// Procedure of process on setup stage of a class specified request with data stage
	// All class specified requests with data stage are processed in Class_Data_Setup
	// Class_Data_Setup()
	//  responses to check all special requests and fills ENDPOINT_INFO
	//  according to the request
	//  If IN tokens are expected, then wLength & wOffset will be filled
	//  with the total transferring bytes and the starting position
	//  If OUT tokens are expected, then rLength & rOffset will be filled
	//  with the total expected bytes and the starting position in the buffer
	//   If the request is valid, Class_Data_Setup returns SUCCESS, else UNSUPPORT
	//  CAUTION:
	//  Since GET_CONFIGURATION & GET_INTERFACE are highly related to
	//  the individual classes, they will be checked and processed here.
	RESULT (*Class_Data_Setup)(uint8_t RequestNo);

	// Procedure of process on setup stage of a class specified request without data stage
	// All class specified requests without data stage are processed in Class_NoData_Setup
	// Class_NoData_Setup
	//  responses to check all special requests and perform the request
	//  CAUTION:
	//  Since SET_CONFIGURATION & SET_INTERFACE are highly related to
	//  the individual classes, they will be checked and processed here.
	RESULT (*Class_NoData_Setup)(uint8_t RequestNo);

	// Class_Get_Interface_Setting
	//   This function is used by the file usb_core.c to test if the selected Interface
	//   and Alternate Setting (uint8_t Interface, uint8_t AlternateSetting) are supported by
	//   the application.
	//   This function is writing by user. It should return "SUCCESS" if the Interface
	//   and Alternate Setting are supported by the application or "UNSUPPORT" if they
	//   are not supported.
	RESULT (*Class_Get_Interface_Setting)(uint8_t Interface, uint8_t AlternateSetting);

	uint8_t* (*GetDeviceDescriptor)(uint16_t Length);
	uint8_t* (*GetConfigDescriptor)(uint16_t Length);
	uint8_t* (*GetStringDescriptor)(uint16_t Length);

	// This field is not used in current library version. It is kept only for
	// compatibility with previous versions
	void* RxEP_buffer;

	uint8_t MaxPacketSize;
} DEVICE_PROP;

// USB standard requests
typedef struct _USER_STANDARD_REQUESTS {
	void (*User_GetConfiguration)(void);       // Get Configuration
	void (*User_SetConfiguration)(void);       // Set Configuration
	void (*User_GetInterface)(void);           // Get Interface
	void (*User_SetInterface)(void);           // Set Interface
	void (*User_GetStatus)(void);              // Get Status
	void (*User_ClearFeature)(void);           // Clear Feature
	void (*User_SetEndPointFeature)(void);     // Set Endpoint Feature
	void (*User_SetDeviceFeature)(void);       // Set Device Feature
	void (*User_SetDeviceAddress)(void);       // Set Device Address
} USER_STANDARD_REQUESTS;

// USB peripheral structure
typedef struct {
	__IO uint16_t EP0R;           // USB Endpoint 0 register,       Address offset: 0x00
	__IO uint16_t RESERVED0;      // Reserved
	__IO uint16_t EP1R;           // USB Endpoint 1 register,       Address offset: 0x04
	__IO uint16_t RESERVED1;      // Reserved
	__IO uint16_t EP2R;           // USB Endpoint 2 register,       Address offset: 0x08
	__IO uint16_t RESERVED2;      // Reserved
	__IO uint16_t EP3R;           // USB Endpoint 3 register,       Address offset: 0x0C
	__IO uint16_t RESERVED3;      // Reserved
	__IO uint16_t EP4R;           // USB Endpoint 4 register,       Address offset: 0x10
	__IO uint16_t RESERVED4;      // Reserved
	__IO uint16_t EP5R;           // USB Endpoint 5 register,       Address offset: 0x14
	__IO uint16_t RESERVED5;      // Reserved
	__IO uint16_t EP6R;           // USB Endpoint 6 register,       Address offset: 0x18
	__IO uint16_t RESERVED6;      // Reserved
	__IO uint16_t EP7R;           // USB Endpoint 7 register,       Address offset: 0x1C
	__IO uint16_t RESERVED7[17];  // Reserved
	__IO uint16_t CNTR;           // Control register,              Address offset: 0x40
	__IO uint16_t RESERVED8;      // Reserved
	__IO uint16_t ISTR;           // Interrupt status register,     Address offset: 0x44
	__IO uint16_t RESERVED9;      // Reserved
	__IO uint16_t FNR;            // Frame number register,         Address offset: 0x48
	__IO uint16_t RESERVEDA;      // Reserved
	__IO uint16_t DADDR;          // Device address register,       Address offset: 0x4C
	__IO uint16_t RESERVEDB;      // Reserved
	__IO uint16_t BTABLE;         // Buffer Table address register, Address offset: 0x50
	__IO uint16_t RESERVEDC;      // Reserved
} USB_TypeDef;

#endif // __USB_TYPE_H
