/*
 * VIA TV additions
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/trident/trident_tv.c,v 1.2 2003/04/21 20:42:30 dawes Exp $ */

#include "xf86_ansic.h"
#include "trident.h"
#include "trident_regs.h"

/***************************************************************************
 *
 * TV parameters for VT1621
 *
 ***************************************************************************/
#define TV_MODE  8
#define TVX_MODE_SIZE 0X72
#define TVX_CRTC_NUM 0x10
#define TVX_REG_NUM 0x62
#define TVX_VT1621_PORT 0x40
#define SMBUS_BASE 0x5000
unsigned char TVX_VT1621_Table[TV_MODE][TVX_MODE_SIZE] = {
{
/* NTSC, 640x480, bpp=8,16 */
0x02, 0x54, 0xE0, 0xFA, 0x11, 0x5D, 0x11, 0x57, 0x5A, 0x56,
0xA0, 0x26, 0x0A, 0x55, 0x37, 0x86,

0x6A, 0x0B, 0x22, 0x27, 0x43, 0x50, 0x13, 0x50,
0xB0, 0x07, 0xEE, 0x15, 0x90, 0xE4, 0x00, 0xA8,
0x00, 0x00, 0x0E, 0x48, 0x38, 0x38, 0x00, 0x1C,
0x00, 0x40, 0x0C, 0x02, 0x01, 0x80, 0x00, 0x00,
0x0F, 0x06, 0x99, 0x7C, 0x04, 0x5D, 0x36, 0x9B,
0x54, 0x00, 0x00, 0xB4, 0x2F, 0x85, 0xFF, 0x00,
0x00, 0x17, 0x15, 0x21, 0x15, 0x05, 0x05, 0x02,
0x1B, 0x1B, 0x24, 0xF8, 0x07, 0x00, 0x00, 0x0F,
0x0F, 0x60, 0x01, 0x0A, 0x00, 0x05, 0x04, 0xFF,
0x03, 0x01, 0x90, 0x33, 0x00, 0x00, 0x00, 0x00,
0x00, 0x04, 0x47, 0x02, 0x02, 0xFD, 0x06, 0xf8,
0x0B, 0xF3, 0x0F, 0x70, 0x05, 0xF9, 0x0B, 0xF1,
0x11, 0x6E
},
{
/* NTSC, 800x600, bpp=8,16 */
0X02, 0x79, 0XE0, 0x73, 0x02, 0x80, 0x01, 0x7A, 0x7E, 0xEC,
0xA0, 0x8A, 0x0E, 0xEB, 0x8B, 0x89,

0x8B, 0x0B, 0x6A, 0x27, 0x43, 0x50, 0x12, 0x50,
0xBC, 0x0A, 0XE8, 0x15, 0x88, 0xDC, 0x00, 0x98,
0x00, 0x00, 0x0A, 0x48, 0x1C, 0x28, 0x03, 0x20,
0x00, 0x40, 0x36, 0x02, 0x03, 0x80, 0x00, 0x00,
0x0D, 0x04, 0x04, 0x7B, 0x00, 0x5D, 0xC1, 0x9B,
0x6B, 0x00, 0x00, 0xA1, 0x3F, 0x9D, 0x2F, 0x10,
0x00, 0x17, 0x15, 0x21, 0x15, 0x05, 0x05, 0x02,
0x1B, 0x1B, 0x24, 0xF8, 0x07, 0x00, 0x00, 0x0F,
0x0F, 0x60, 0x01, 0x0A, 0x00, 0x05, 0x04, 0xFF,
0x03, 0x01, 0xD6, 0x80, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0C, 0x46, 0x02, 0x02, 0xFD, 0x06, 0xF8,
0x0B, 0xF3, 0x0F, 0x70, 0x05, 0xF9, 0x0B, 0xF1,
0x11, 0x6E
},
{
/* NTSC, 640x480, bpp=32 */
0X02, 0x54, 0XE0, 0xFA, 0x11, 0x5D, 0x01, 0x57, 0x5A, 0x56,
0xA0, 0x26, 0x0A, 0X55, 0x37, 0x46,

0x6A, 0x0B, 0x23, 0x33, 0x43, 0x50, 0x13, 0x51,
0xB0, 0x07, 0xAB, 0x15, 0x90, 0xA9, 0x00, 0x98,
0x00, 0x00, 0x0E, 0x48, 0x38, 0x38, 0x03, 0x1C,
0x00, 0x40, 0x0C, 0x02, 0x03, 0x80, 0x00, 0x00,
0x0F, 0x04, 0x99, 0x7A, 0x04, 0x5E, 0xB6, 0x90,
0x5B, 0x00, 0x00, 0x67, 0x2F, 0x88, 0xFA, 0x00,
0x00, 0x17, 0x15, 0x21, 0x15, 0x05, 0x05, 0x02,
0x1B, 0x1B, 0x24, 0xF8, 0x07, 0x00, 0x00, 0x0F,
0x0F, 0x60, 0x01, 0x0A, 0x00, 0x05, 0x04, 0xFF,
0x03, 0x01, 0xA0, 0x33, 0x1B, 0x00, 0X00, 0x00,
0x00, 0x08, 0x47, 0x02, 0x02, 0xFD, 0x06, 0xf8,
0x0B, 0xF3, 0x0F, 0x70, 0x05, 0xF9, 0x0B, 0xF1,
0x11, 0x6E
},
{
/* NTSC, 800x600, bpp=32 */
0X02, 0x79, 0XE0, 0x73, 0x02, 0x80, 0x01, 0x7B, 0x7E, 0xEC,
0xA0, 0x8A, 0x0E, 0xEB, 0x8B, 0x49,

0x8B, 0x0B, 0x6B, 0x27, 0x43, 0x50, 0x12, 0x2D,
0xBC, 0x0C, 0xED, 0x15, 0x88, 0xEE, 0x00, 0x99,
0x00, 0x00, 0x0A, 0x48, 0x1C, 0x28, 0x03, 0x20,
0x00, 0x40, 0x36, 0x02, 0x03, 0x80, 0x00, 0x00,
0x0D, 0x04, 0x04, 0x7A, 0x00, 0x5D, 0xC1, 0x9B,
0x6B, 0x00, 0x00, 0xA1, 0x3F, 0x9D, 0x2F, 0x10,
0x00, 0x17, 0x15, 0x21, 0x15, 0x05, 0x05, 0x02,
0x1B, 0x1B, 0x24, 0xF8, 0x07, 0x00, 0x00, 0x0F,
0x0F, 0x60, 0x01, 0x0A, 0x00, 0x05, 0x04, 0xFF,
0x03, 0x01, 0xC6, 0x90, 0x00, 0x00, 0x00, 0x00,
0x00, 0x08, 0x46, 0x02, 0x02, 0xFD, 0x06, 0xF8,
0x0B, 0xF3, 0x0F, 0x70, 0x05, 0xF9, 0x0B, 0xF1,
0x11, 0x6E
},
{
/* PAL, 640x480, bpp=8,16 */
0X82, 0x5D, 0XE0, 0x23, 0x02, 0x64, 0x01, 0x56, 0x5A, 0x6F,
0xA0, 0x0D, 0x0F, 0x6E, 0x24, 0xC1,

0x6B, 0x0B, 0x03, 0x67, 0x40, 0x50, 0x12, 0x96,
0xCE, 0x32, 0xFF, 0x01, 0x7E, 0xF6, 0x00, 0xA8,
0x00, 0x00, 0x07, 0x48, 0x20, 0x1C, 0x44, 0x60,
0x44, 0x4F, 0x1B, 0x02, 0x03, 0x80, 0x00, 0x00,
0x0C, 0x0C, 0xD7, 0x84, 0x04, 0x68, 0x3B, 0x9C,
0x57, 0x63, 0x17, 0xAC, 0x25, 0x80, 0x29, 0x10,
0x00, 0x1A, 0x22, 0x2A, 0x22, 0x05, 0x02, 0x00,
0x1C, 0x3D, 0x14, 0xFE, 0x03, 0x54, 0x01, 0xFE,
0x7E, 0x60, 0x00, 0x08, 0x00, 0x04, 0x07, 0x55,
0x01, 0x01, 0xA0, 0x33, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0C, 0x4E, 0xFE, 0x03, 0xFB, 0x06, 0xF8,
0x0A, 0xF5, 0x0C, 0x73, 0x06, 0xF8, 0x0B, 0xF2,
0x10, 0x6F
},
{
/* PAL, 800x600, bpp=8,16 */
0X82, 0x5B, 0XE0, 0x91, 0x02, 0x73, 0x07, 0x6C, 0x70, 0xEC,
0xA0, 0xA8, 0x0B, 0xEB, 0xAD, 0xC7,

0x8B, 0x0B, 0x1A, 0x47, 0x40, 0x50, 0x12, 0x56,
0x00, 0x37, 0xF7, 0x00, 0x7D, 0xE2, 0x00, 0xB9,
0x00, 0x00, 0x0E, 0x48, 0x38, 0x38, 0x44, 0x62,
0x44, 0x4F, 0x53, 0x02, 0x07, 0x80, 0x00, 0x00,
0x0A, 0x05, 0xA2, 0x83, 0x08, 0x68, 0x46, 0x99,
0x68, 0x63, 0x17, 0xAC, 0x25, 0x80, 0x6B, 0x10,
0x00, 0x1A, 0x22, 0x2A, 0x22, 0x05, 0x02, 0x00,
0x1C, 0x3D, 0x14, 0xFE, 0x03, 0x54, 0x01, 0xFE,
0x7E, 0x60, 0x00, 0x08, 0x00, 0x04, 0x07, 0x55,
0x01, 0x01, 0xE6, 0x90, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0C, 0x4D, 0xFB, 0x04, 0xFB, 0x07, 0xF8,
0x09, 0xF6, 0x0A, 0x74, 0x06, 0xF8, 0x0B, 0xF2,
0x10, 0x6F
},
{
/* PAL, 640x480, bpp=32 */
0X82, 0x5D, 0XE0, 0x23, 0x02, 0x64, 0x01, 0x56, 0x5A, 0x6F,
0xA0, 0x0D, 0x0F, 0x6E, 0x24, 0x81,

0x6B, 0x0B, 0x02, 0x67, 0x40, 0x50, 0x12, 0x93,
0xCE, 0x32, 0xF0, 0x01, 0x88, 0xE8, 0x00, 0xA8,
0x00, 0x00, 0x07, 0x48, 0x20, 0x1C, 0x44, 0x60,
0x44, 0x4F, 0x1B, 0x02, 0x03, 0x80, 0x00, 0x00,
0x0C, 0x05, 0xE2, 0x84, 0x00, 0x68, 0x3B, 0x9C,
0x57, 0x63, 0x17, 0xAC, 0x25, 0x80, 0x29, 0x10,
0x00, 0x1A, 0x22, 0x2A, 0x22, 0x05, 0x02, 0x00,
0x1C, 0x3D, 0x14, 0xFE, 0x03, 0x54, 0x01, 0xFE,
0x7E, 0x60, 0x00, 0x08, 0x00, 0x04, 0x07, 0x55,
0x01, 0x01, 0xA0, 0x33, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0C, 0x4E, 0xFE, 0x03, 0xFB, 0x06, 0xF8,
0x0A, 0xF5, 0x0C, 0x73, 0x06, 0xF8, 0x0B, 0xF2,
0x10, 0x6F
},
{
/* PAL, 800x600, bpp=32 */
0X82, 0x5B, 0XE0, 0x91, 0x02, 0x73, 0x07, 0x6C, 0x70, 0xEC,
0xA0, 0xA8, 0x0B, 0xEB, 0xAD, 0x87,

0x8B, 0x0B, 0x1A, 0x67, 0x40, 0x50, 0x12, 0x53,
0x00, 0x37, 0xEE, 0x00, 0x83, 0xEB, 0x00, 0xB9,
0x00, 0x00, 0x0E, 0x48, 0x38, 0x38, 0x44, 0x62,
0x44, 0x4F, 0x53, 0x02, 0x07, 0x80, 0x00, 0x00,
0x0A, 0x05, 0x5E, 0x83, 0x08, 0x68, 0x46, 0x99,
0x68, 0x63, 0x17, 0xAC, 0x25, 0x80, 0x6B, 0x10,
0x00, 0x1A, 0x22, 0x2A, 0x22, 0x05, 0x02, 0x00,
0x1C, 0x3D, 0x14, 0xFE, 0x03, 0x54, 0x01, 0xFE,
0x7E, 0x60, 0x00, 0x08, 0x00, 0x04, 0x07, 0x55,
0x01, 0x01, 0xA0, 0x22, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0C, 0x4D, 0xFB, 0x04, 0xFB, 0x07, 0xF8,
0x09, 0xF6, 0x0A, 0x74, 0x06, 0xF8, 0x0B, 0xF2,
0x10, 0x6F
}
};
/* TV Parameters for CH7005C */
#define TV_CH7005C_MODE_SIZE 45
#define TV_CH7005C_CRTC_NUM 0x10
#define TV_CH7005C_TVREG_NUM 29
#define TV_CH7005C_PORT 0xEA
unsigned char TV_CH7005C_Table[TV_MODE][TV_CH7005C_MODE_SIZE]={
{
/* NTSC 640x480 bpp=8,16 */
0x02, 0x80, 0x20, 0x02, 0x00, 0x5D, 0X80, 0X57, 0X80, 0X56,
0XBA, 0X10, 0X8C, 0X50, 0XF8, 0X7F,

0X6A, 0X7A, 0X00, 0X09, 0X80, 0X66, 0X00, 0X60,
0X2E, 0XFF, 0X01, 0X0B, 0X0C, 0X03, 0X40, 0X3F,
0X7E, 0X40, 0X02, 0X00, 0X08, 0X00, 0X00, 0X00,
0X00, 0X00, 0X0A, 0X02, 0X05
},
{
/* NTSC 800X600 bpp=8,16 */
0x02, 0x80, 0x20, 0x02, 0x00, 0x7D, 0X80, 0X6E, 0X1C, 0XBA,
0XF0, 0X70, 0X8C, 0XBA, 0X78, 0X53,

0X8C, 0X4A, 0X00, 0X09, 0X80, 0XAE, 0X01, 0X80,
0X2E, 0X02, 0X01, 0X0B, 0X7E, 0X7E, 0X7E, 0X7E,
0X7E, 0X40, 0X01, 0X0C, 0X00, 0X00, 0X00, 0X00,
0X00, 0X00, 0X0A, 0X00, 0X05
},
{
/* NTSC 640x480 bpp=32 */
0x02, 0x80, 0x20, 0x02, 0x00, 0x5D, 0X80, 0X57, 0X80, 0X56,
0XBA, 0X10, 0X8C, 0X50, 0XBD, 0X57,

0X6A, 0X7A, 0X00, 0X09, 0X80, 0X67, 0X00, 0X60,
0X2E, 0XFF, 0X01, 0X0B, 0X0C, 0X03, 0X40, 0X3F,
0X7E, 0X40, 0X02, 0X00, 0X07, 0X0C, 0X0D, 0X00,
0X00, 0X00, 0X0A, 0X02, 0X05
},
{
/* NTSC 800X600 bpp=32 */
0x02, 0x80, 0x20, 0x02, 0x00, 0x7D, 0X80, 0X6E, 0X1C, 0XBA,
0XF0, 0X70, 0X8C, 0XBA, 0XF8, 0X53,

0X8C, 0X4A, 0X00, 0X09, 0X80, 0XAF, 0X01, 0X80,
0X2E, 0X02, 0X01, 0X0B, 0X0C, 0X03, 0X40, 0X3F,
0X7E, 0X40, 0X01, 0X0C, 0X00, 0X00, 0X00, 0X00,
0X00, 0X00, 0X0A, 0X00, 0X05
},
{
/* PAL 640x480 bpp=8,16 */
0x82, 0x80, 0x20, 0x02, 0x00, 0x71, 0X74, 0X62, 0X84, 0X6F,
0XF0, 0X10, 0X09, 0XEB, 0X80, 0X5F,

0X81, 0X4A, 0X00, 0X09, 0X80, 0X84, 0X00, 0X70,
0X28, 0X02, 0X01, 0X0B, 0X0C, 0X03, 0X40, 0X3F,
0X7E, 0X40, 0X02, 0X00, 0X08, 0X00, 0X00, 0X00,
0X00, 0X00, 0X0A, 0X01, 0X05
},
{
/* PAL 800x600 bpp=8,16 */
0x82, 0x80, 0x20, 0x02, 0x00, 0x73, 0X76, 0X6A, 0X8C, 0XEC,
0XF0, 0X7E, 0X09, 0XEB, 0X8F, 0X8D,

0X83, 0X4A, 0X00, 0X09, 0X80, 0X7E, 0X00, 0X70,
0X3F, 0X02, 0X01, 0X0B, 0X0C, 0X03, 0X40, 0X3F,
0X7E, 0X40, 0X02, 0X00, 0X08, 0X00, 0X00, 0X00,
0X00, 0X00, 0X0A, 0X01, 0X05
},
{
/* PAL 640x480 bpp=32 */
0x82, 0x80, 0x20, 0x02, 0x00, 0x71, 0X74, 0X62, 0X84, 0X6F,
0XF0, 0X10, 0X09, 0XEB, 0X80, 0X1F,

0X81, 0X4A, 0X00, 0X09, 0X80, 0X84, 0X00, 0X70,
0X28, 0X02, 0X01, 0X0B, 0X0C, 0X03, 0X40, 0X3F,
0X7E, 0X40, 0X02, 0X00, 0X08, 0X00, 0X00, 0X00,
0X00, 0X00, 0X0A, 0X01, 0X05
},
{
/* PAL 800X600 bpp=32 */
0x82, 0x80, 0x20, 0x02, 0x00, 0x73, 0X76, 0X6A, 0X8C, 0XEC,
0XF0, 0X7E, 0X09, 0XEB, 0X5D, 0X48,

0X83, 0X4A, 0X00, 0X09, 0X80, 0X7E, 0X00, 0X70,
0X3F, 0X02, 0X01, 0X0B, 0X0C, 0X03, 0X40, 0X3F,
0X7E, 0X40, 0X02, 0X00, 0X08, 0X00, 0X00, 0X00,
0X00, 0X00, 0X0A, 0X01, 0X05
}
};

static unsigned char smbus_read(ScrnInfoPtr pScrn, unsigned char bIndex, unsigned char devAdr)
{
    TRIDENTPtr pTrident=TRIDENTPTR(pScrn);
    unsigned short i;
    unsigned char  bData;

    /* clear host status */
    OUTB(SMBUS_BASE, 0xFF);

    /* check SMBUS ready */
    for ( i = 0; i < 0xFFFF; i++ )
        if ( (INB(SMBUS_BASE) & 0x01) == 0 )
            break;

    /* set host command */
    OUTB(SMBUS_BASE+3, bIndex);

    /* set slave address */
    OUTB(SMBUS_BASE+4, devAdr | 0x01);

    /* start */
    OUTB(SMBUS_BASE+2, 0x48);

    /* SMBUS Wait Ready */
    for ( i = 0; i < 0xFFFF; i++ )
        if ( (INB(SMBUS_BASE) & 0x01) == 0 )
            break;
    bData=INB(SMBUS_BASE+5);

    return bData;

}

static void smbus_write(ScrnInfoPtr pScrn, unsigned char bData, unsigned char bIndex, unsigned char devAdr)
{
    TRIDENTPtr pTrident=TRIDENTPTR(pScrn);
    unsigned short i;

    /* clear host status */
    OUTB(SMBUS_BASE, 0xFF);

    /* check SMBUS ready */
    for ( i = 0; i < 0xFFFF; i++ )
        if ( (INB(SMBUS_BASE) & 0x01) == 0 )
           break;

    OUTB(SMBUS_BASE+2, 0x08);

    /* set host command */
    OUTB(SMBUS_BASE+3, bIndex);

    /* set slave address */
    OUTB(SMBUS_BASE+4, devAdr & 0xFE);

    OUTB(SMBUS_BASE+5, bData);

    /* start */
    OUTB(SMBUS_BASE+2, 0x48);

    /* SMBUS Wait Ready */
    for ( i = 0; i < 0xFFFF; i++ )
    if ( (INB(SMBUS_BASE) & 0x01) == 0 )
       break;
}
void VIA_SaveTVDepentVGAReg(ScrnInfoPtr pScrn)
{
    TRIDENTPtr pTrident=TRIDENTPTR(pScrn);
    unsigned char protect;
    unsigned char bTmp;
    int i;
    unsigned char VGA_RegIdx_about_TV[VGA_REGNUM_ABOUT_TV]={
                  0xD8,0XD9,/* SR */
                  0X33,/* GR */
                  0XC0,0XD0,0XD1,0XD2,0XD3,0XE0,0XE3,0XE4,0XE5,/* CR */
                  0XE6,0XE7,0XF0,0XF1,0XF6,0XFE,0XFF
                  };
    unsigned char TV_CH7005C_RegIdx[TV_CH7005C_TVREG_NUM]={
                  0X00,0X01,0X03,0X04,0X06,0X07,0X08,0X09,
		  0X0A,0X0B,0X0D,0X0E,0X10,0X11,0X13,0X14,
		  0X15,0X17,0X18,0X19,0X1A,0X1B,0X1C,0X1D,
		  0X1E,0X1F,0X20,0X21,0X3D
                  };

    /*ErrorF("VIAB3D: VIA_SaveTVDepentVGAReg:\n");*/

    /* Unprotect */
    OUTB(0x3C4, 0x11);
    protect = INB(0x3C5);
    OUTB(0x3C5, 0x92);

    /* Set TV Hw environment */
    OUTB(0x3d4,0xc1);
    OUTB(0x3d5,0x41);

    /* SR_d8,SR_d9 */
    for (i=0; i<2; i++)
    {
        OUTB(0x3c4,VGA_RegIdx_about_TV[i]);
        bTmp=INB(0x3c5);
        pTrident->DefaultTVDependVGASetting[i]=bTmp;
    }

    /* GR_33 */
    OUTB(0x3ce,0x33);
    bTmp=INB(0x3cf);
    pTrident->DefaultTVDependVGASetting[2]=bTmp;

    /* CR_c0,d0,d1,d2,d3,e0,e3,e4,e5,e6,e7,f0,f1,f6,fe,ff */
    for (i=3; i<VGA_REGNUM_ABOUT_TV; i++)
    {
        OUTB(0x3d4,VGA_RegIdx_about_TV[i]);
        bTmp=INB(0x3d5);
        pTrident->DefaultTVDependVGASetting[i]=bTmp;
    }

    switch (pTrident->TVChipset)
    {
       case 1:
             for (i=0; i<TVX_REG_NUM; i++)
             {
                 bTmp=smbus_read(pScrn,i,TVX_VT1621_PORT);
                 pTrident->DefaultTVDependVGASetting[VGA_REGNUM_ABOUT_TV+i]=bTmp;
             }
	     break;
       case 2:
             for (i=0; i<TV_CH7005C_TVREG_NUM; i++)
	     {
                 bTmp=smbus_read(pScrn,TV_CH7005C_RegIdx[i],TV_CH7005C_PORT);
                 pTrident->DefaultTVDependVGASetting[VGA_REGNUM_ABOUT_TV+i]=bTmp;
	     }
	     break;
       default:
             ErrorF("VIAB3D: VIA_SaveTVDepentVGAReg: Wrong Chipset setting\n");
	     break;

    }
    /* protect */
    OUTB(0x3C4, 0x11);
    OUTB(0x3C5, protect);
}
void VIA_RestoreTVDependVGAReg(ScrnInfoPtr pScrn)
{
    TRIDENTPtr pTrident=TRIDENTPTR(pScrn);
    unsigned char protect;
    unsigned char bTmp;
    int i;
    unsigned char VGA_RegIdx_about_TV[VGA_REGNUM_ABOUT_TV]={
                  0xD8,0XD9,/* SR */
                  0X33,/* GR */
                  0XC0,0XD0,0XD1,0XD2,0XD3,0XE0,0XE3,0XE4,0XE5,/* CR */
                  0XE6,0XE7,0XF0,0XF1,0XF6,0XFE,0XFF
                  };
    unsigned char TV_CH7005C_RegIdx[TV_CH7005C_TVREG_NUM]={
                  0X00,0X01,0X03,0X04,0X06,0X07,0X08,0X09,
		  0X0A,0X0B,0X0D,0X0E,0X10,0X11,0X13,0X14,
		  0X15,0X17,0X18,0X19,0X1A,0X1B,0X1C,0X1D,
		  0X1E,0X1F,0X20,0X21,0X3D
                  };

    /*ErrorF("VIAB3D: VIA_RestoreTVDependVGAReg:\n");*/

    /* Unprotect */
    OUTB(0x3C4, 0x11);
    protect = INB(0x3C5);
    OUTB(0x3C5, 0x92);

    /* Set TV Hw environment */
    OUTB(0x3d4,0xc1);
    OUTB(0x3d5,0x41);

    /* SR_d8,SR_d9 */
    for (i=0; i<2; i++)
    {
        OUTB(0x3c4,VGA_RegIdx_about_TV[i]);
        bTmp=pTrident->DefaultTVDependVGASetting[i];
        OUTB(0x3c5,bTmp);
    }
    /* GR_33 */
    OUTB(0x3ce,0x33);
    bTmp=pTrident->DefaultTVDependVGASetting[2];
    OUTB(0x3cf,bTmp);

    /* CR_c0,d0,d1,d2,d3,e0,e3,e4,e5,e6,e7,f0,f1,f6,fe,ff */
    for (i=3; i<VGA_REGNUM_ABOUT_TV; i++)
    {
        OUTB(0x3d4,VGA_RegIdx_about_TV[i]);
        bTmp=pTrident->DefaultTVDependVGASetting[i];
        OUTB(0x3d5,bTmp);
    }
    switch (pTrident->TVChipset)
    {
         case 1:
                for (i=0; i<TVX_REG_NUM; i++)
                {
                    bTmp=pTrident->DefaultTVDependVGASetting[VGA_REGNUM_ABOUT_TV+i];
                    smbus_write(pScrn,bTmp,i,TVX_VT1621_PORT);
		}
		break;
         case 2:
	        for (i=0; i<TV_CH7005C_TVREG_NUM; i++)
		{
                    bTmp=pTrident->DefaultTVDependVGASetting[VGA_REGNUM_ABOUT_TV+i];
                    smbus_write(pScrn,bTmp,TV_CH7005C_RegIdx[i],TV_CH7005C_PORT);
		}
		break;
       default:
             ErrorF("VIAB3D: VIA_SaveTVDepentVGAReg: Wrong Chipset setting\n");
	     break;
    }
    /* protect */
    OUTB(0x3C4, 0x11);
    OUTB(0x3C5, protect);
}
void VIA_TVInit(ScrnInfoPtr pScrn)
{
    TRIDENTPtr pTrident=TRIDENTPTR(pScrn);
    unsigned char idx=0;
    unsigned char i;
    unsigned char protect;
    unsigned char TV_CRTC[TVX_CRTC_NUM] =
       { 0xC0,0xD0,0xD1,0xD2,0xD3,0xE0,0xE3,0xE4,0xE5,
         0xE6,0xE7,0xF0,0xF1,0xF6,0xFE,0xFF  };
    unsigned char TV_CH7005C_RegIdx[TV_CH7005C_TVREG_NUM]={
                  0X00,0X01,0X03,0X04,0X06,0X07,0X08,0X09,
		  0X0A,0X0B,0X0D,0X0E,0X10,0X11,0X13,0X14,
		  0X15,0X17,0X18,0X19,0X1A,0X1B,0X1C,0X1D,
		  0X1E,0X1F,0X20,0X21,0X3D
                  };

#ifdef DEBUG_CODE_TRACE
    ErrorF("VIAB3D: VIA_TVInit:\n");
#endif

    if (pScrn->currentMode->HDisplay==640 && pScrn->currentMode->VDisplay==480 && (pScrn->depth==8 || pScrn->depth==16) && pTrident->TVSignalMode == 0)
    {
       /* Overlay window 1 position OK */
       ErrorF("VIAB3D: VIA_TVInit: TV Params 640x480x8(16) NTSC\n");
       idx=0;
       pTrident->OverrideHsync=-71;
       pTrident->OverrideVsync=15;
     }
    else if (pScrn->currentMode->HDisplay==800 && pScrn->currentMode->VDisplay==600 && (pScrn->depth==8 || pScrn->depth==16) && pTrident->TVSignalMode == 0)
    {
       /* Overlay window 1 position OK */
       ErrorF("VIAB3D: VIA_TVInit: TV Params 800x600x8(16) NTSC\n");
       idx=1;
       pTrident->OverrideHsync=-152;
       pTrident->OverrideVsync=72;
       }
    else if (pScrn->currentMode->HDisplay==640 && pScrn->currentMode->VDisplay==480 && pScrn->depth==24 && pTrident->TVSignalMode == 0)
    {
       ErrorF("VIAB3D: VIA_TVInit: TV Params 640x480x32 NTSC\n");
       idx=2;
       pTrident->OverrideHsync=-65;
       pTrident->OverrideVsync=14;
       }
    else if (pScrn->currentMode->HDisplay==800 && pScrn->currentMode->VDisplay==600 && pScrn->depth==24 && pTrident->TVSignalMode == 0)
    {
       ErrorF("VIAB3D: VIA_TVInit: TV Params 800x600x32 NTSC\n");
       idx=3;
       pTrident->OverrideHsync=-158;
       pTrident->OverrideVsync=72;
       }
    else if (pScrn->currentMode->HDisplay==640 && pScrn->currentMode->VDisplay==480 && (pScrn->depth==8 || pScrn->depth==16) && pTrident->TVSignalMode == 1)
    {
       /* Overlay window 1 position OK */
       ErrorF("VIAB3D: VIA_TVInit: TV Params 640x480x8(16) PAL\n");
       idx=4;
       pTrident->OverrideHsync=2;
       pTrident->OverrideVsync=65;
       }
    else if (pScrn->currentMode->HDisplay==800 && pScrn->currentMode->VDisplay==600 && (pScrn->depth==8 || pScrn->depth==16) && pTrident->TVSignalMode == 1)
    {
       ErrorF("VIAB3D: VIA_TVInit: TV Params 800x600x8(16) PAL\n");
       /* patch TV screen defection */
       idx=5;
       /* patch 800x600 screen defect */
       OUTB(0x3d4,0x2f);
       OUTB(0x3d5,0xbf);
       pTrident->OverrideHsync=-145;
       pTrident->OverrideVsync=43;
       }
    else if (pScrn->currentMode->HDisplay==640 && pScrn->currentMode->VDisplay==480 && pScrn->depth==24 && pTrident->TVSignalMode == 1)
    {
       ErrorF("VIAB3D: VIA_TVInit: TV Params 640x480x32 PAL\n");
       idx=6;
       pTrident->OverrideHsync=0;
       pTrident->OverrideVsync=63;
       }
    else if (pScrn->currentMode->HDisplay==800 && pScrn->currentMode->VDisplay==600 && pScrn->depth==24 && pTrident->TVSignalMode == 1)
    {
       ErrorF("VIAB3D: VIA_TVInit: TV Params 800x600x32 PAL\n");
       idx=7;
       OUTB(0x3d4,0x2f);
       OUTB(0x3d5,0xbf);
       pTrident->OverrideHsync=-141;
       pTrident->OverrideVsync=42;
       }
    else
    {
       ErrorF("VIAB3D: VIA_TVInit: TV Params default mode\n");
       return;
    }

    /* Unprotect */
    OUTB(0x3C4, 0x11);
    protect = INB(0x3C5);
    OUTB(0x3C5, 0x92);

    /* Set TV hw environment */
    OUTB(0x3c4,0x24);
    OUTB(0x3c5,0x4f);
    OUTB(0x3d4,0xc1);
    OUTB(0x3d5,0x41);
    OUTB(0x3ce,0x23);
    OUTB(0x3cf,0x88);

    /* set CRT + TV */
    OUTB(0x3CE,0x33);
    OUTB(0x3CF,0x20);

    /* set CRTC */
    for( i = 0; i < TVX_CRTC_NUM; i++ )
    {
         OUTB(0x3D4, TV_CRTC[i]);

	 if (pTrident->TVChipset==2) {
            OUTB(0x3D5, TV_CH7005C_Table[idx][i]);
	 }
	 else {
	    OUTB(0x3D5, TVX_VT1621_Table[idx][i]);
	 }
    }


    /* Digital TV interface control */
    switch (pTrident->TVChipset)
    {
           case 1: OUTB(0x3C4,0xD8);
                   OUTB(0x3C5,0x60);
                   OUTB(0x3C4,0xD9);
                   OUTB(0x3C5,0x38);
		   break;
           case 2: OUTB(0x3c4,0xd8);
	           OUTB(0x3c5,0x24);
                   OUTB(0x3C4,0xD9);
                   OUTB(0x3C5,0x18);
		   break;
    }

    switch (pTrident->TVChipset)
    {
           case 1:
                  /* set TVX registers */
                  for (i=0; i < TVX_REG_NUM; i++ )
                  {
                      smbus_write(pScrn,TVX_VT1621_Table[idx][TVX_CRTC_NUM+i], i, TVX_VT1621_PORT);
                  }
		  break;
           case 2:
	          for (i=0; i<TV_CH7005C_TVREG_NUM; i++)
		  {
                      smbus_write(pScrn,TV_CH7005C_Table[idx][TV_CH7005C_CRTC_NUM+i], TV_CH7005C_RegIdx[i], TV_CH7005C_PORT);
		  }
	          break;
    }

    /*VIA_DumpReg(pScrn);*/

    /* protect */
    OUTB(0x3C4, 0x11);
    OUTB(0x3C5, protect);
}
void VIA_DumpReg(ScrnInfoPtr pScrn)
{
    TRIDENTPtr pTrident=TRIDENTPTR(pScrn);
    int i,j;
    unsigned char bTmp;
    unsigned char protect;

    /* Unprotect */
    OUTB(0x3C4, 0x11);
    protect = INB(0x3C5);
    OUTB(0x3C5, 0x92);

    /* SR */
    for (i=0; i<16; i++)
    {
        for (j=0; j<16; j++)
	{
            OUTB(0x3c4,(16*i+j));
	    bTmp=INB(0x3c5);

	    ErrorF("SR%02x=%02x ",(16*i+j),bTmp);
	}
	ErrorF("\n");
    }
    ErrorF("\n");
    /* CR */
    for (i=0; i<16; i++)
    {
        for (j=0; j<16; j++)
	{
            OUTB(0x3d4,(16*i+j));
	    bTmp=INB(0x3d5);

	    ErrorF("CR%02x=%02x ",(16*i+j),bTmp);
	}
	ErrorF("\n");
    }
    ErrorF("\n");
    /* GR */
    for (i=0; i<16; i++)
    {
        for (j=0; j<16; j++)
	{
            OUTB(0x3ce,(16*i+j));
	    bTmp=INB(0x3cf);

	    ErrorF("GR%02x=%02x ",(16*i+j),bTmp);
	}
	ErrorF("\n");
    }
    ErrorF("\n");
    /* SM */
    for (i=0; i<16; i++)
    {
        for (j=0; j<16; j++)
	{
	    if (pTrident->TVChipset==2)
               bTmp=smbus_read(pScrn,(16*i+j),TV_CH7005C_PORT);
	    else bTmp=smbus_read(pScrn,(16*i+j),TVX_VT1621_PORT);
	    ErrorF("SM%02x=%02x ",(16*i+j),bTmp);
	}
	ErrorF("\n");
    }
    ErrorF("\n");
    /* protect */
    OUTB(0x3C4, 0x11);
    OUTB(0x3C5, protect);

}
