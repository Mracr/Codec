
#ifndef __Codec_h__
#define __Codec_h__
// Longhai +

//
// Include Header Files
//
// Base Header 
#include <Uefi.h>                             // For Base Data Type and UEFI Specification
// Needed LibraryClasses
#include <Library/UefiLib.h>                  // For Print,
#include <Library/UefiBootServicesTableLib.h> // For gBS
// Industry Standard 
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>
// Protocol
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/EfiShellInterface.h>      // ShellPkg

//
// Memory Access
//
#define MemRead8(Address)          *((volatile UINT8 *)((UINTN)Address))
#define MemRead16(Address)         *((volatile UINT16 *)((UINTN)Address))
#define MemRead32(Address)         *((volatile UINT32 *)((UINTN)(Address)))
#define MemRead64(Address)         *((volatile UINT64 *)((UINTN)(Address)))

#define MemWrite8(Address, Data)   *((volatile UINT8 *)((UINTN)Address)) = Data
#define MemWrite16(Address, Data)  *((volatile UINT16 *)((UINTN)Address)) = Data
#define MemWrite32(Address, Data)  *((volatile UINT32 *)((UINTN)(Address))) = Data
#define MemWrite64(Address, Data)  *((volatile UINT64 *)((UINTN)(Address))) = Data

//
// DEBUG Info 
//
#define DEBUG_INFO_ENABLE 1
#if DEBUG_INFO_ENABLE
  UINT8 swi = 0;
  #define PRINT_DEBUG_INFO(Value) {if (swi == 1) {AsciiPrint("[DEBUG_INFO LINE(%03u)]" #Value ": 0x%lX\n",__LINE__ ,(UINT64)(Value));}}
#else
  #define PRINT_DEBUG_INFO(Value) {}
#endif

//
// Memory Fence 
//
#if defined(_MSC_VER)
    void _ReadWriteBarrier(void);
  #pragma intrinsic(_ReadWriteBarrier)
#elif defined(__GCC__)
    MemoryBarrier() __asm__ __volatile__("mFence" ::: "memory");
#endif

//
// Disable Compiler Warning
//
#pragma warning(disable : 4101) // No Used Variable warning Disable

//
// Constant 
//
#define Delay                           120           // Microsecond

#define PCIe_BaseAddress_Mask           0x3FFFC000000   // 512GB Align 

#define HDAudioInterface_ClassCode      0x00040300      // Bit[23:0]

#define PIO_SUCCESS                     0x0
#define PIO_FAILED                      0x1

#define NID_RootNID                     0x00

//
// Data Struct
//
EFI_STATUS Status = EFI_SUCCESS;

struct {
  struct {
    struct {
      EFI_GUID Guid;
      EFI_SHELL_INTERFACE *Interface;
    } Protocol;
  } Interface;
} Shell;

struct ACPI_Table{
  UINTN Index;
  struct {
    EFI_GUID Guid;
    EFI_ACPI_SDT_PROTOCOL *Interface;
  } Protocol;
  struct PCIeMemoryMapBaseAddressTable{
    EFI_ACPI_SDT_HEADER *Header;
    UINT64 *Reserved;
    EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *PCIe;
  } MCFG;
} ACPI;

enum HDAudioInterface{
  DIDVIDReg32 = 0x0,
  CommandReg16 = 0x04,
  ClassCodeReg32 = 0x08,
  ControllerBALowReg32 = 0x10,
  ControllerBAUpReg32 = 0x14,
  SSIDSVIDReg32 = 0x2C
} ;

struct {
  union {
      struct { // Little-End Order
        UINT64 Reg : 12;
        UINT64 Fun : 3;
        UINT64 Dev : 5;
        UINT64 Bus : 8;
        UINT64 Base : 4;
        UINT64 Upper : 32;
      };
      UINT64 Address;
  } ECAM;
  union {
      struct {
        UINT64     : 2;
        UINT64 Reg : 6;
        UINT64 Fun : 3;
        UINT64 Dev : 5;
        UINT64 Bus : 8;
        UINT64     : 7;
        UINT64 Enable : 1;
        UINT64 Upper : 32;
      };
      UINT64 Address;
  } CAM;
} PCIe = {0x0};

enum HDAudioController{
  GlobalControlReg32 = 0x08,
  StateChangeStatusReg16 = 0x0E,
  CORBControlReg8 = 0x4C,
  RIRBControlReg8 = 0x5C,
  ICWriteReg32 = 0x60,
  IRReadReg32 = 0x64,
  ICStatusReg16 = 0x68,
} ;

struct {
  struct HDAudioPCIConfigReg{
    UINT32 DIDVID;
    UINT32 SSIDSVID;
    UINT32 ClassCode;
    UINT16 Command;
    UINT16 Status;
    UINT64 ControllerBA;
  } Interface;
  struct {
    UINT16 GlobalControl;
    UINT16 StateChangeStatus;
    UINT16 ICStatus;
    UINT32 ICWrite;
    UINT32 IRRead;
    UINT8 CORBControl;
    UINT8 RIRBControl;
  } Controller;
} HDAudio;

enum Verb{
  GetParameter = 0xF00,
  ConfigurationDefault = 0xF1C,
  ImplementationIdentification = 0xF20
};

enum Payload{
  VendorID = 0x0,
  RevisionID = 0x02,
  SubordinateNodeCount = 0x04,
  FunctionGroupType = 0x05,
  AudioWidgetCapabilities = 0x09,
  ConnectionListLength = 0x0E
};

struct PIO_Config {
  union {
    struct { // Little-End Order
      UINT32 Payload : 8;
      UINT32 Verb    : 12;
      UINT32 NID     : 8;
      UINT32 CAd     : 4;
    };
    UINT32 Command;
  };
  UINT32 Response;
} PIO;

typedef enum {
  AudioFunctionGroup = 0x01,
  VendorDefinedModemFunctionGroup = 0x02,
  VendorDefinedFunctionGroup_Start = 0x80,
  VendorDefinedFunctionGroup_End = 0xFF
} FunctionGroup_Type;

typedef enum {
  AudioOutput = 0x0,
  AudioInput = 0x1,
  AudioMixer = 0x2,
  AudioSelector = 0x3,
  PinComplex = 0x4,
  PowerWidget = 0x5,
  VolumeKnobWidget11 = 0x6,
  BeepGeneratorWidget12 = 0x7,
  VendorDefinedAudioWidget = 0xF
} Widget_Type;

struct RootNode {
  UINT8 CAd;
  UINT8 NID;
  UINT16 VID;
  UINT16 DID;
  UINT8 StartNode;
  UINT8 NodeCount;
  struct FunctionGroup {
    UINT8 StartNode;
    UINT8 NodeCount;
    FunctionGroup_Type Type;
    struct Widget {
      UINT8 StartNode;
      UINT8 NodeCount;
      Widget_Type Type;
      struct ConnectionList {
        UINT8 Length : 7;
        UINT8 Form   : 1;
      } ConnectionList;
    }Widget;
    UINT32 SSIDSVID;
  } Function;
} Codec;

//
// Function Declare
//
EFI_STATUS HDAudioCtrl();
EFI_STATUS Initiate_Pio_Protocol(IN UINT32 Command, OUT UINT32 *Response);

#endif