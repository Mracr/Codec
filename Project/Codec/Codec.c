/*------------------------------------------------
Needed DEC  [LibraryClasses]:
                MdePkg/Include/Library/:
                    1.UefiApplicationEntryPoint
                    2.UefiLib

Needed DSC  [LibraryClasses(Instance)]:
                MdePkg/Library/:
                    1.UefiApplicationEntryPoint.inf
                    2.UefiLib.inf
                    3.UefiBootServicesTableLib.inf
                    4.UefiRuntimeServicesTableLib.inf
                    5.BaseLib.inf
                    6.PrintLib.inf
                    7.PcdLib.inf
                    8.BaseMemoryLib.inf
                    9.MemoryAllocationLib.inf
                    10.DevicePathLib.inf

            [Components]:
                YourPkgName/YourComponentsName/:
                    1.Codec.inf

Needed INF  [Packages]:
                    1.MdePkg/MdePkg.dec
                    2.ShellPkg/ShellPkg.dec

            [LibraryClasses]:
                MdePkg/Include/Library/:
                    1.UefiApplicationEntryPoint
                    2.UefiLib

            [Protocols]:
                    1.gEfiAcpiSdtProtocolGuid
                    2.gEfiShellInterfaceGuid

-------------------------------------------------*/

#include "Codec.h"

EFI_STATUS EntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    UINTN i = 0;
    
    //
    // Parse Command Line Prag
    //
    Shell.Interface.Protocol.Guid = gEfiShellInterfaceGuid;
    Status = gBS->HandleProtocol(ImageHandle, &Shell.Interface.Protocol.Guid, &Shell.Interface.Protocol.Interface);
    if (Status == EFI_SUCCESS) {
        for (i = 1; i < Shell.Interface.Protocol.Interface->Argc; i++) {
            if (!StrCmp(Shell.Interface.Protocol.Interface->Argv[i], L"/d") ||
                !StrCmp(Shell.Interface.Protocol.Interface->Argv[i], L"/D") ||
                !StrCmp(Shell.Interface.Protocol.Interface->Argv[i], L"-d") ||
                !StrCmp(Shell.Interface.Protocol.Interface->Argv[i], L"-D")
            ) {
                swi = 1;
            } else {
                Print(L"Usage: >%s [/d] [/D] [-d] [-D]\n", Shell.Interface.Protocol.Interface->Argv[0]);
                return 0;
            }
        }
    }

    //
    // PCIe Info
    //
    PRINT_DEBUG_INFO(Status);
    ACPI.Protocol.Guid = gEfiAcpiSdtProtocolGuid;
    ACPI.Protocol.Interface = NULL;
    Status = gBS->LocateProtocol(&ACPI.Protocol.Guid, NULL, &ACPI.Protocol.Interface);
    PRINT_DEBUG_INFO(Status);
    if (Status != EFI_SUCCESS) {return 0;}
    for (ACPI.Index = 0; ACPI.Index < 30; ACPI.Index++) {
        Status = ACPI.Protocol.Interface->GetAcpiTable(ACPI.Index, &ACPI.MCFG.Header, NULL, NULL);
        if (Status != EFI_SUCCESS) {continue;}
        PRINT_DEBUG_INFO(ACPI.Index);
        PRINT_DEBUG_INFO(ACPI.MCFG.Header->Signature);
        if (ACPI.MCFG.Header->Signature == SIGNATURE_32('M', 'C', 'F', 'G')) {
            ACPI.MCFG.Reserved = (UINT64 *)(ACPI.MCFG.Header + 1);
            ACPI.MCFG.PCIe     = (EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *)(ACPI.MCFG.Reserved + 1);
            break;
        }
    }

    PRINT_DEBUG_INFO(ACPI.MCFG.PCIe->PciSegmentGroupNumber);
    PRINT_DEBUG_INFO(ACPI.MCFG.PCIe->BaseAddress);
    PRINT_DEBUG_INFO(ACPI.MCFG.PCIe->StartBusNumber);
    PRINT_DEBUG_INFO(ACPI.MCFG.PCIe->EndBusNumber);

    //
    // HD Audio Interface
    //
    PCIe.ECAM.Address = ACPI.MCFG.PCIe->BaseAddress;
    PRINT_DEBUG_INFO(PCIe.ECAM.Base);
    if (PCIe.ECAM.Address == 0) {return 0;};
    for (PCIe.ECAM.Bus = ACPI.MCFG.PCIe->StartBusNumber; PCIe.ECAM.Bus < (ACPI.MCFG.PCIe->EndBusNumber + 1); PCIe.ECAM.Bus++){
        for (PCIe.ECAM.Dev = 0; PCIe.ECAM.Dev < (31 + 1); PCIe.ECAM.Dev++){
            for (PCIe.ECAM.Fun = 0; PCIe.ECAM.Fun < (7 + 1); PCIe.ECAM.Fun++){

                //
                // HD Audio Interface VID and Class Code
                //
                HDAudio.Interface.DIDVID    = MemRead32(PCIe.ECAM.Address | DIDVIDReg32);
                HDAudio.Interface.ClassCode = (UINT32)(MemRead32(PCIe.ECAM.Address | ClassCodeReg32) & 0xFFFFFF00) >> 8;
                if ((HDAudio.Interface.DIDVID & 0xFFFF) != 0xFFFF && HDAudio.Interface.ClassCode == HDAudioInterface_ClassCode) {

                    //
                    // Memory Space Decode Enable
                    //
                    HDAudio.Interface.Command = MemRead16(PCIe.ECAM.Address | CommandReg16);
                    if ((HDAudio.Interface.Command & 0x6) != 0x6) { // Master and Memory Decode Enable
                        MemWrite16(PCIe.ECAM.Address | CommandReg16, (HDAudio.Interface.Command | 0x6));
                    }
                    PRINT_DEBUG_INFO(PCIe.ECAM.Bus);
                    PRINT_DEBUG_INFO(PCIe.ECAM.Dev);
                    PRINT_DEBUG_INFO(PCIe.ECAM.Fun);
                    PRINT_DEBUG_INFO(HDAudio.Interface.Command);
                    PRINT_DEBUG_INFO(MemRead16(PCIe.ECAM.Address | CommandReg16));

                    //
                    // HD Audio Interface SSIDSVID and HD Audio Controller Base Address
                    //
                    HDAudio.Interface.SSIDSVID     = MemRead32(PCIe.ECAM.Address | SSIDSVIDReg32);
                    HDAudio.Interface.ControllerBA = MemRead32(PCIe.ECAM.Address | ControllerBALowReg32);
                    PRINT_DEBUG_INFO(HDAudio.Interface.ControllerBA);
                    if ((HDAudio.Interface.ControllerBA & 0x6) == 0x4) { // Indicate 64bit Memory Space
                        HDAudio.Interface.ControllerBA = MemRead32(PCIe.ECAM.Address | ControllerBAUpReg32);
                        HDAudio.Interface.ControllerBA <<= 32;
                        HDAudio.Interface.ControllerBA |= MemRead32(PCIe.ECAM.Address | ControllerBALowReg32) & 0xFFFFC000;
                    }
                    PRINT_DEBUG_INFO(HDAudio.Interface.SSIDSVID);
                    PRINT_DEBUG_INFO(HDAudio.Interface.ControllerBA);

                    //
                    // Print HD Audio Interface Info
                    //
                    Print(L"---------- HD Audio Interface ----------\n");
                    Print(L"Bus/Dev/Fun  :    %02X/%02X/%02X\n", PCIe.ECAM.Bus, PCIe.ECAM.Dev, PCIe.ECAM.Fun);
                    Print(L"DID:VID      :  0x%08X\n", HDAudio.Interface.DIDVID);
                    Print(L"SSID:SVID    :  0x%08X\n", HDAudio.Interface.SSIDSVID);

                    //
                    // Entry HD Audio Controller
                    //
                    Status = HDAudioCtrl();

                    //
                    // Exit Loop
                    //
                    //{PCIe.ECAM.Bus = ACPI.MCFG.PCIe->EndBusNumber; PCIe.ECAM.Dev = PCIe_MaxDevNumber; PCIe.ECAM.Fun = PCIe_MaxFunNumber;}
                }
                if (PCIe.ECAM.Fun == 7) break; // Preventing Bit-field Variable Overflow
            }
            if (PCIe.ECAM.Dev == 31) break; // Preventing Bit-field Variable Overflow
        }
        if (PCIe.ECAM.Bus == ACPI.MCFG.PCIe->EndBusNumber) break; // Preventing Bit-field Variable Overflow
    }
    return 0;
}

//
// HD Audio Controller Entry
//
EFI_STATUS HDAudioCtrl(VOID){
    //
    // HD Audio Controller Reset
    //
    if (HDAudio.Interface.ControllerBA == 0) {return 0;;}
    HDAudio.Controller.GlobalControl = MemRead32(HDAudio.Interface.ControllerBA | GlobalControlReg32);
    MemWrite32(HDAudio.Interface.ControllerBA | GlobalControlReg32, HDAudio.Controller.GlobalControl & 0xFFFE);
    gBS->Stall(5000);             // Reset Must Delay
    MemWrite32(HDAudio.Interface.ControllerBA | GlobalControlReg32, HDAudio.Controller.GlobalControl | 0x01);
    PRINT_DEBUG_INFO(HDAudio.Controller.GlobalControl);
    PRINT_DEBUG_INFO(MemRead32(HDAudio.Interface.ControllerBA | GlobalControlReg32));

    //
    // Disable DMA Engine if Enable
    //
    HDAudio.Controller.CORBControl = MemRead8(HDAudio.Interface.ControllerBA | CORBControlReg8);
    HDAudio.Controller.RIRBControl = MemRead8(HDAudio.Interface.ControllerBA | RIRBControlReg8);
    if ((HDAudio.Controller.CORBControl & 0x2) != 0x0) {MemWrite8(HDAudio.Interface.ControllerBA | CORBControlReg8, HDAudio.Controller.CORBControl & 0xFD);}
    if ((HDAudio.Controller.RIRBControl & 0x2) != 0x0) {MemWrite8(HDAudio.Interface.ControllerBA | RIRBControlReg8, HDAudio.Controller.RIRBControl & 0xFD);}
    PRINT_DEBUG_INFO(MemRead8(HDAudio.Interface.ControllerBA | CORBControlReg8));
    PRINT_DEBUG_INFO(MemRead8(HDAudio.Interface.ControllerBA | RIRBControlReg8));

    //
    // Codec
    //
    for (PIO.CAd = 0; PIO.CAd < 0xF; PIO.CAd++){   // CAd(15) is Broadcast Codec, Must ingore

        //
        // Check Codec Whether Exists
        //
        PIO.NID     = NID_RootNID;
        PIO.Verb    = GetParameter;
        PIO.Payload = VendorID;
        PRINT_DEBUG_INFO(PIO.Command);
        Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
        PRINT_DEBUG_INFO(Status);
        if (Status != PIO_SUCCESS) {continue;}
        PRINT_DEBUG_INFO(PIO.Response);
        Codec.CAd = PIO.CAd;
        Codec.NID = PIO.NID;
        Codec.DID = (PIO.Response & 0xFFFF);
        Codec.VID = (PIO.Response & 0xFFFF0000) >> 16;
        PRINT_DEBUG_INFO(Codec.CAd);
        PRINT_DEBUG_INFO(Codec.NID);
        PRINT_DEBUG_INFO(Codec.DID);
        PRINT_DEBUG_INFO(Codec.VID);
        PRINT_DEBUG_INFO(PIO.Verb);

        //
        // All Function Group Node
        //
        PIO.Verb    = GetParameter;
        PIO.Payload = SubordinateNodeCount;
        PRINT_DEBUG_INFO(PIO.Command);
        Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
        PRINT_DEBUG_INFO(Status);
        if (Status == PIO_SUCCESS) {
            PRINT_DEBUG_INFO(PIO.Verb);
            PRINT_DEBUG_INFO(PIO.Payload);
            PRINT_DEBUG_INFO(PIO.Response);
            Codec.NodeCount = (PIO.Response & 0xFF);
            Codec.StartNode = (PIO.Response & 0xFF0000) >> 16;
            Codec.Function.NodeCount = Codec.NodeCount;
            Codec.Function.StartNode = Codec.StartNode;
            PRINT_DEBUG_INFO(Codec.Function.NodeCount);
            PRINT_DEBUG_INFO(Codec.Function.StartNode);
        }

        //
        // Print Root NID Info
        //
        Print(L"\nCAd(%u)\n", Codec.CAd);
        Print(L"DID:VID      :  0x%04X%04X\n", Codec.DID, Codec.VID);

        //
        // Function Group 
        //
        for (Codec.Function.StartNode; Codec.Function.StartNode < (Codec.StartNode + Codec.NodeCount); Codec.Function.StartNode++) {
            
            //
            // Function Group Type
            //     01h      Audio Function Group
            //     02h      Vendor Defined Modem Function Group
            //     80-FFh   Vendor Defined Function Group
            //
            PIO.NID     = Codec.Function.StartNode;
            PIO.Verb    = GetParameter;
            PIO.Payload = FunctionGroupType;
            PRINT_DEBUG_INFO(PIO.Command);
            Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
            PRINT_DEBUG_INFO(Status);
            if (Status == PIO_SUCCESS) {
                PRINT_DEBUG_INFO(Codec.Function.StartNode);
                PRINT_DEBUG_INFO(PIO.NID);
                PRINT_DEBUG_INFO(PIO.Verb);
                PRINT_DEBUG_INFO(PIO.Payload);
                PRINT_DEBUG_INFO(PIO.Response);
                Codec.Function.Type = PIO.Response & 0xFF;
                PRINT_DEBUG_INFO(Codec.Function.Type);
            }

            //
            // All Widget Node
            //
            PIO.NID     = Codec.Function.StartNode;
            PIO.Verb    = GetParameter;
            PIO.Payload = SubordinateNodeCount;
            PRINT_DEBUG_INFO(PIO.Command);
            Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
            PRINT_DEBUG_INFO(Status);
            if (Status == PIO_SUCCESS) {
                PRINT_DEBUG_INFO(Codec.Function.StartNode);
                PRINT_DEBUG_INFO(PIO.NID);
                PRINT_DEBUG_INFO(PIO.Verb);
                PRINT_DEBUG_INFO(PIO.Payload);
                PRINT_DEBUG_INFO(PIO.Response);
                Codec.Function.Widget.NodeCount = (PIO.Response & 0xFF);
                Codec.Function.Widget.StartNode = (PIO.Response & 0xFF0000) >> 16;
                PRINT_DEBUG_INFO(Codec.Function.Widget.StartNode);
                PRINT_DEBUG_INFO(Codec.Function.Widget.NodeCount);
            }

            //
            // Function Group SSID and SVID
            //
            PIO.NID     = Codec.Function.StartNode;
            PIO.Verb    = ImplementationIdentification;
            PIO.Payload = 0;
            PRINT_DEBUG_INFO(PIO.Command);
            Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
            PRINT_DEBUG_INFO(Status);
            if (Status == PIO_SUCCESS) {
                PRINT_DEBUG_INFO(Codec.Function.StartNode);
                PRINT_DEBUG_INFO(PIO.NID);
                PRINT_DEBUG_INFO(PIO.Verb);
                PRINT_DEBUG_INFO(PIO.Payload);
                PRINT_DEBUG_INFO(PIO.Response);
                Codec.Function.SSIDSVID = (PIO.Response & 0xFFFF);
                Codec.Function.SSIDSVID <<= 16;
                Codec.Function.SSIDSVID |= (PIO.Response & 0xFFFF0000) >> 16;
                PRINT_DEBUG_INFO(Codec.Function.SSIDSVID);
            }

            //
            // Print Function Group Info
            //
            Print(L"SSID:SVID    :  0x%08X", Codec.Function.SSIDSVID);
            if (Codec.CAd == 0) {
                if (Codec.Function.SSIDSVID != HDAudio.Interface.SSIDSVID) {
                    Print(L"  ERROR: CodecSSID:SVID(%08X) <> CtrlSSID:SVID(%08X)", Codec.Function.SSIDSVID, HDAudio.Interface.SSIDSVID);
                }
            }
            Print(L"\n");

            //
            // Widget
            //
            for (Codec.Function.Widget.StartNode; Codec.Function.Widget.StartNode < 128; Codec.Function.Widget.StartNode++) {
                
                //
                // Widget Type
                //     0h      Audio Output
                //     1h      Audio Input 
                //     2h      Audio Mixer 
                //     3h      Audio Selector 
                //     4h      Pin Complex 
                //     5h      Power Widget 
                //     6h      Volume Knob Widget11
                //     7h      Beep Generator Widget12
                //     8h-Eh   Reserved 
                //     Fh      Vendor Defined Audio Widget
                //
                PIO.NID     = Codec.Function.Widget.StartNode;
                PIO.Verb    = GetParameter;
                PIO.Payload = AudioWidgetCapabilities;
                PRINT_DEBUG_INFO(PIO.Command);
                Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
                PRINT_DEBUG_INFO(Status);
                if (Status == PIO_SUCCESS) {
                    PRINT_DEBUG_INFO(Codec.Function.Widget.StartNode);
                    PRINT_DEBUG_INFO(PIO.NID);
                    PRINT_DEBUG_INFO(PIO.Verb);
                    PRINT_DEBUG_INFO(PIO.Payload);
                    PRINT_DEBUG_INFO(PIO.Response);
                    Codec.Function.Widget.Type = (PIO.Response & 0x00F00000) >> 20;
                    PRINT_DEBUG_INFO(Codec.Function.Widget.Type);
                }

                //
                // Connection List Length and Form
                //
                PIO.NID     = Codec.Function.Widget.StartNode;
                PIO.Verb    = GetParameter;
                PIO.Payload = ConnectionListLength;
                PRINT_DEBUG_INFO(PIO.Command);
                Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
                PRINT_DEBUG_INFO(Status);
                if (Status == PIO_SUCCESS) {
                    PRINT_DEBUG_INFO(Codec.Function.Widget.StartNode);
                    PRINT_DEBUG_INFO(PIO.NID);
                    PRINT_DEBUG_INFO(PIO.Verb);
                    PRINT_DEBUG_INFO(PIO.Payload);
                    PRINT_DEBUG_INFO(PIO.Response);
                    Codec.Function.Widget.ConnectionList.Length = (PIO.Response & 0x7F);
                    Codec.Function.Widget.ConnectionList.Form   = (PIO.Response & 0x80) >> 7;
                    PRINT_DEBUG_INFO(Codec.Function.Widget.ConnectionList.Length);
                    PRINT_DEBUG_INFO(Codec.Function.Widget.ConnectionList.Form);
                }

                //
                // Configuration Default in Each Pin Weidget
                //
                PIO.NID     = Codec.Function.Widget.StartNode;
                PIO.Verb    = ConfigurationDefault;
                PIO.Payload = VendorID;
                PRINT_DEBUG_INFO(PIO.Command);
                Status      = Initiate_Pio_Protocol(PIO.Command, &PIO.Response);
                PRINT_DEBUG_INFO(Status);
                if (Status == PIO_SUCCESS && Codec.Function.Widget.Type == PinComplex) {
                    PRINT_DEBUG_INFO(Codec.Function.Widget.StartNode);
                    PRINT_DEBUG_INFO(PIO.NID);
                    PRINT_DEBUG_INFO(PIO.Verb);
                    PRINT_DEBUG_INFO(PIO.Payload);
                    PRINT_DEBUG_INFO(PIO.Response);
                    Print(L"\n                //NID(%02X) \n", Codec.Function.Widget.StartNode);
                    Print(L"                0x%01X%02X71C%02X\n", Codec.CAd, Codec.Function.Widget.StartNode, (PIO.Response & 0xFF));
                    Print(L"                0x%01X%02X71D%02X\n", Codec.CAd, Codec.Function.Widget.StartNode, (PIO.Response & 0xFF00) >> 8);
                    Print(L"                0x%01X%02X71E%02X\n", Codec.CAd, Codec.Function.Widget.StartNode, (PIO.Response & 0xFF0000) >> 16);
                    Print(L"                0x%01X%02X71F%02X\n", Codec.CAd, Codec.Function.Widget.StartNode, (PIO.Response & 0xFF000000) >> 24);
                }
                
                //-//
                if (Codec.Function.Widget.StartNode == 0xFF) {break;} // Preventing Variable Overflow
            }
            if (Codec.Function.StartNode == 0xFF) {break;} // Preventing Variable Overflow
        }
        if (Codec.CAd == 0xF) {break;}
    }

    //
    // Work Finished Yet
    //
    Print(L"\n----------------- END ------------------\n\n");
    return Status;
}


EFI_STATUS Initiate_Pio_Protocol(IN UINT32 Command, OUT UINT32 *Response){
    //
    // Ready Work
    //
    HDAudio.Controller.ICStatus = MemRead16(HDAudio.Interface.ControllerBA | ICStatusReg16);
    MemWrite16(HDAudio.Interface.ControllerBA | ICStatusReg16, (HDAudio.Controller.ICStatus & 0xFFFE) | 0x2);

    //
    // Write a Command to ICW and Read a Response from IRR
    //
    MemWrite32(HDAudio.Interface.ControllerBA | ICWriteReg32, Command);
    MemWrite16(HDAudio.Interface.ControllerBA | ICStatusReg16, 0x0001); // Send a Verb to Codec
    gBS->Stall(Delay);                                                  // Delay, PCIe Memory Write May is Post transaction
    _ReadWriteBarrier();                                                // Memory Fence
    PRINT_DEBUG_INFO(MemRead32(HDAudio.Interface.ControllerBA | ICWriteReg32));
    PRINT_DEBUG_INFO(MemRead16(HDAudio.Interface.ControllerBA | ICStatusReg16));
    HDAudio.Controller.ICStatus = MemRead16(HDAudio.Interface.ControllerBA | ICStatusReg16);
    if ((HDAudio.Controller.ICStatus & 0x0003) == 0x0002) {
        HDAudio.Controller.IRRead = MemRead32(HDAudio.Interface.ControllerBA | IRReadReg32);
        *Response = HDAudio.Controller.IRRead;
        return PIO_SUCCESS;
    } else {
        return PIO_FAILED;
    }
}