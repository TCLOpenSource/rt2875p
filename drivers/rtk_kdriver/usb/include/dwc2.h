#ifndef __dwc2_H__
#define __dwc2_H__

#define DWC2_VERSION                     "2.98a"
#define EHCI_SYNOPSYS_REGS_OFFSET        0x80

struct ehci_synopsys_regs {
    __le32 insnreg00;  /* USBOPBASE + 80h */
    __le32 insnreg01;
    __le32 insnreg02;
    __le32 insnreg03;
    __le32 insnreg04;
    __le32 insnreg05;
    __le32 insnreg06;
    __le32 insnreg07;
    __le32 insnreg08;
};

struct ohci_synopsys_regs {
    __le32 insnreg06;  /* USBBASE + 98h*/
    __le32 insnreg07;
};

#endif  /* __dwc2_H__ */
