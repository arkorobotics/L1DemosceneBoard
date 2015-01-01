/* 
 * File:   main.c
 * Author: arko
 *
 * Created on September 8, 2014, 7:25 PM
 */
#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#define  FCY    16000000UL    // Instruction cycle frequency, Hz
#include <libpic30.h>

_CONFIG1(FWDTEN_OFF & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
_CONFIG2(POSCMOD_HS & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2)
_CONFIG3(ALTPMP_ALTPMPEN & SOSCSEL_EC)
        
#define HOR_RES 640
#define VER_RES 480
#define HOR_FRONT_PORCH 20
#define HOR_BACK_PORCH 51
#define HOR_PULSE_WIDTH  96
#define VER_FRONT_PORCH 12
#define VER_BACK_PORCH 36
#define VER_PULSE_WIDTH 2

#define GFX_DISPLAY_PIXEL_COUNT    90160

#define GFX_DISPLAY_BUFFER_START_ADDRESS        0x00000001ul
#define GFX_DISPLAY_BUFFER_LENGTH               0x00012C00ul

//__eds__ unsigned char GFXDisplayBuffer[GFX_DISPLAY_PIXEL_COUNT] __attribute__((eds));
//__eds__ unsigned char GFXDisplayBufferBottom[GFX_DISPLAY_PIXEL_COUNT] __attribute__((eds));


__eds__ unsigned char  __attribute__((far,section("eds1b"),space(eds) ,address(0x01000))) GFXDisplayBuffer[ GFX_DISPLAY_PIXEL_COUNT ]; // Sample data buffer
//__eds__ unsigned char  __attribute__((far,section("eds2b"),space(eds) ,address(0x09000))) GFXDisplayBufferBottom[ GFX_DISPLAY_PIXEL_COUNT ]; // Sample data buffer




void config_graphics(void) {
    
    CLKDIVbits.RCDIV = 0;

    CLKDIVbits.G1CLKSEL = 1;                         /* Use 96Mhz Clk */
    CLKDIV2bits.GCLKDIV = 26;  //56      //11        /* 56 = divide by 15 = 6.5Mhz */

    G1CON2bits.DPMODE = 1;      /* TFT */
    G1DBENbits.GDBEN = 0xFFFF;
    G1DPWbits.DPW = HOR_RES;
    G1DPHbits.DPH = VER_RES;
    G1DPWTbits.DPWT = HOR_FRONT_PORCH + HOR_PULSE_WIDTH + HOR_BACK_PORCH + HOR_RES;
    G1DPHTbits.DPHT = VER_FRONT_PORCH + VER_PULSE_WIDTH + VER_BACK_PORCH + VER_RES;
    G1CON3bits.DPCLKPOL = 1;    /* Sample on rising edge*/
    G1CON3bits.DPENOE = 0;
    G1CON3bits.DPPWROE = 0;
    G1CON3bits.DPVSOE = 1;      /* use VSYNC */
    G1CON3bits.DPHSOE = 1;      /* use HSYNC */
    G1CON3bits.DPVSPOL = 0;     /* VSYNC negative polarity */
    G1CON3bits.DPHSPOL = 0;     /* HSYNC negative polarity */
    G1ACTDAbits.ACTLINE = VER_FRONT_PORCH + VER_PULSE_WIDTH + VER_BACK_PORCH;
    G1ACTDAbits.ACTPIX = HOR_FRONT_PORCH + HOR_PULSE_WIDTH + HOR_BACK_PORCH;
    G1VSYNCbits.VSST = VER_FRONT_PORCH;
    G1HSYNCbits.HSST = HOR_FRONT_PORCH;
    G1VSYNCbits.VSLEN = VER_PULSE_WIDTH;
    G1HSYNCbits.HSLEN = HOR_PULSE_WIDTH;
    G1DBLCONbits.VENST = VER_FRONT_PORCH + VER_PULSE_WIDTH + VER_BACK_PORCH;
    G1DBLCONbits.HENST = HOR_FRONT_PORCH + HOR_PULSE_WIDTH + HOR_BACK_PORCH;
    
    G1DPADRL = (unsigned long)(GFXDisplayBuffer) & 0xFFFF;
    G1DPADRH = 0;

    G1CON2bits.DPBPP = 2;     /* 8bpp mode */
    G1CON1bits.PUBPP = 2;

    G1CON3bits.DPPINOE = 1;
    G1CON3bits.DPPOWER = 1;

    G1CON2bits.DPTEST = 2;      // DISABLE THIS LINE TO ALLOW FRAMEBUFFER TO
                                // DISPLAY!
    
    G1CON1bits.G1EN = 1;        /* Enable module */
    G1CON3bits.DPPOWER = 1;
    
    /*
    G1CLUTbits.CLUTEN = 1;

    G1CLUTbits.CLUTRWEN = 1;
    G1CLUTbits.CLUTADR = 0x00;
    G1CLUTWR = 0x001F;
    G1CLUTbits.CLUTADR = 0x01;
    G1CLUTWR = 0xF800;
    G1CLUTbits.CLUTADR = 0x02;
    G1CLUTWR = 0x07E0;

    G1CLUTbits.CLUTRWEN = 0;
    */
}


int main(void) {
    //OSCCON = 0x0000;

    CLKDIVbits.RCDIV = 0;      // 8 MHz
    CLKDIVbits.CPDIV = 0;

    ANSB = 0x0000;
    ANSC = 0x0000;
    ANSD = 0x0000;
    ANSF = 0x0000;
    ANSG = 0x0000;
    //TRISE = 0x0000;

    uint32_t y;
    for (y = 1; y < GFX_DISPLAY_PIXEL_COUNT+4000; y++)
    {
        GFXDisplayBuffer[(unsigned long)(y)] = 0x00;
    }

    config_graphics();

    uint32_t x = 0;
    y=0;

    while (1) {
        y=0;

        for (y = 7928; y < 86168; y = y + 319)
        {

                   GFXDisplayBuffer[y+1] = (unsigned char)x;
                   GFXDisplayBuffer[y+2] = (unsigned char)x;
                   GFXDisplayBuffer[y+3] = (unsigned char)x;
                   GFXDisplayBuffer[y+4] = (unsigned char)x;
                   GFXDisplayBuffer[y+5] = (unsigned char)x;
                   GFXDisplayBuffer[y+6] = (unsigned char)x;
                   GFXDisplayBuffer[y+7] = (unsigned char)x;
                   GFXDisplayBuffer[y+8] = (unsigned char)x;
                   GFXDisplayBuffer[y+9] = (unsigned char)x;
                   GFXDisplayBuffer[y+10] = (unsigned char)x;
                   GFXDisplayBuffer[y+11] = (unsigned char)x;
                   GFXDisplayBuffer[y+11] = (unsigned char)x;
                   GFXDisplayBuffer[y+12] = (unsigned char)x;
                   GFXDisplayBuffer[y+13] = (unsigned char)x;
                   GFXDisplayBuffer[y+14] = (unsigned char)x;
                   GFXDisplayBuffer[y+15] = (unsigned char)x;
                   GFXDisplayBuffer[y+16] = (unsigned char)x;
                   GFXDisplayBuffer[y+17] = (unsigned char)x;
                   GFXDisplayBuffer[y+18] = (unsigned char)x;
                   GFXDisplayBuffer[y+19] = (unsigned char)x;
                   GFXDisplayBuffer[y+20] = (unsigned char)x;
                   GFXDisplayBuffer[y+21] = (unsigned char)x;
                   GFXDisplayBuffer[y+22] = (unsigned char)x;
                   GFXDisplayBuffer[y+23] = (unsigned char)x;
                   GFXDisplayBuffer[y+24] = (unsigned char)x;
                   GFXDisplayBuffer[y+25] = (unsigned char)x;
                   GFXDisplayBuffer[y+26] = (unsigned char)x;
                   GFXDisplayBuffer[y+27] = (unsigned char)x;
                   GFXDisplayBuffer[y+28] = (unsigned char)x;
                   GFXDisplayBuffer[y+29] = (unsigned char)x;
                   GFXDisplayBuffer[y+30] = (unsigned char)x;
                   GFXDisplayBuffer[y+31] = (unsigned char)x;
                   GFXDisplayBuffer[y+32] = (unsigned char)x;
                   GFXDisplayBuffer[y+33] = (unsigned char)x;
                   GFXDisplayBuffer[y+34] = (unsigned char)x;
                   GFXDisplayBuffer[y+35] = (unsigned char)x;
                   GFXDisplayBuffer[y+36] = (unsigned char)x;
                   GFXDisplayBuffer[y+37] = (unsigned char)x;
                   GFXDisplayBuffer[y+38] = (unsigned char)x;
                   GFXDisplayBuffer[y+39] = (unsigned char)x;
                   GFXDisplayBuffer[y+40] = (unsigned char)x;
                   GFXDisplayBuffer[y+41] = (unsigned char)x;
                   GFXDisplayBuffer[y+42] = (unsigned char)x;
                   GFXDisplayBuffer[y+43] = (unsigned char)x;
                   GFXDisplayBuffer[y+44] = (unsigned char)x;
                   GFXDisplayBuffer[y+45] = (unsigned char)x;
                   GFXDisplayBuffer[y+46] = (unsigned char)x;
                   GFXDisplayBuffer[y+47] = (unsigned char)x;
                   GFXDisplayBuffer[y+48] = (unsigned char)x;
                   GFXDisplayBuffer[y+49] = (unsigned char)x;
                   GFXDisplayBuffer[y+50] = (unsigned char)x;
                   GFXDisplayBuffer[y+51] = (unsigned char)x;
                   GFXDisplayBuffer[y+52] = (unsigned char)x;
                   GFXDisplayBuffer[y+53] = (unsigned char)x;
                   GFXDisplayBuffer[y+54] = (unsigned char)x;
                   GFXDisplayBuffer[y+55] = (unsigned char)x;
                   GFXDisplayBuffer[y+56] = (unsigned char)x;
                   GFXDisplayBuffer[y+57] = (unsigned char)x;
                   GFXDisplayBuffer[y+58] = (unsigned char)x;
                   GFXDisplayBuffer[y+59] = (unsigned char)x;
                   GFXDisplayBuffer[y+60] = (unsigned char)x;
                   GFXDisplayBuffer[y+61] = (unsigned char)x;
                   GFXDisplayBuffer[y+62] = (unsigned char)x;
                   GFXDisplayBuffer[y+63] = (unsigned char)x;
                   GFXDisplayBuffer[y+64] = (unsigned char)x;
                   GFXDisplayBuffer[y+65] = (unsigned char)x;
                   GFXDisplayBuffer[y+66] = (unsigned char)x;
                   GFXDisplayBuffer[y+67] = (unsigned char)x;
                   GFXDisplayBuffer[y+68] = (unsigned char)x;
                   GFXDisplayBuffer[y+69] = (unsigned char)x;
                   GFXDisplayBuffer[y+70] = (unsigned char)x;
                   GFXDisplayBuffer[y+71] = (unsigned char)x;
                   GFXDisplayBuffer[y+72] = (unsigned char)x;
                   GFXDisplayBuffer[y+73] = (unsigned char)x;
                   GFXDisplayBuffer[y+74] = (unsigned char)x;
                   GFXDisplayBuffer[y+75] = (unsigned char)x;
                   GFXDisplayBuffer[y+76] = (unsigned char)x;
                   GFXDisplayBuffer[y+77] = (unsigned char)x;
                   GFXDisplayBuffer[y+78] = (unsigned char)x;
                   GFXDisplayBuffer[y+79] = (unsigned char)x;
                   GFXDisplayBuffer[y+80] = (unsigned char)x;
                   GFXDisplayBuffer[y+81] = (unsigned char)x;
                   GFXDisplayBuffer[y+82] = (unsigned char)x;
                   GFXDisplayBuffer[y+83] = (unsigned char)x;
                   GFXDisplayBuffer[y+84] = (unsigned char)x;
                   GFXDisplayBuffer[y+85] = (unsigned char)x;
                   GFXDisplayBuffer[y+86] = (unsigned char)x;
                   GFXDisplayBuffer[y+87] = (unsigned char)x;
                   GFXDisplayBuffer[y+88] = (unsigned char)x;
                   GFXDisplayBuffer[y+89] = (unsigned char)x;
                   GFXDisplayBuffer[y+90] = (unsigned char)x;
                   GFXDisplayBuffer[y+91] = (unsigned char)x;
                   GFXDisplayBuffer[y+92] = (unsigned char)x;
                   GFXDisplayBuffer[y+93] = (unsigned char)x;
                   GFXDisplayBuffer[y+94] = (unsigned char)x;
                   GFXDisplayBuffer[y+95] = (unsigned char)x;
                   GFXDisplayBuffer[y+96] = (unsigned char)x;
                   GFXDisplayBuffer[y+97] = (unsigned char)x;
                   GFXDisplayBuffer[y+98] = (unsigned char)x;
                   GFXDisplayBuffer[y+99] = (unsigned char)x;

                   GFXDisplayBuffer[y+100] = (unsigned char)x;
                   GFXDisplayBuffer[y+101] = (unsigned char)x;
                   GFXDisplayBuffer[y+102] = (unsigned char)x;
                   GFXDisplayBuffer[y+103] = (unsigned char)x;
                   GFXDisplayBuffer[y+104] = (unsigned char)x;
                   GFXDisplayBuffer[y+105] = (unsigned char)x;
                   GFXDisplayBuffer[y+106] = (unsigned char)x;
                   GFXDisplayBuffer[y+107] = (unsigned char)x;
                   GFXDisplayBuffer[y+108] = (unsigned char)x;
                   GFXDisplayBuffer[y+109] = (unsigned char)x;
                   GFXDisplayBuffer[y+110] = (unsigned char)x;
                   GFXDisplayBuffer[y+111] = (unsigned char)x;
                   GFXDisplayBuffer[y+111] = (unsigned char)x;
                   GFXDisplayBuffer[y+112] = (unsigned char)x;
                   GFXDisplayBuffer[y+113] = (unsigned char)x;
                   GFXDisplayBuffer[y+114] = (unsigned char)x;
                   GFXDisplayBuffer[y+115] = (unsigned char)x;
                   GFXDisplayBuffer[y+116] = (unsigned char)x;
                   GFXDisplayBuffer[y+117] = (unsigned char)x;
                   GFXDisplayBuffer[y+118] = (unsigned char)x;
                   GFXDisplayBuffer[y+119] = (unsigned char)x;
                   GFXDisplayBuffer[y+120] = (unsigned char)x;
                   GFXDisplayBuffer[y+121] = (unsigned char)x;
                   GFXDisplayBuffer[y+122] = (unsigned char)x;
                   GFXDisplayBuffer[y+123] = (unsigned char)x;
                   GFXDisplayBuffer[y+124] = (unsigned char)x;
                   GFXDisplayBuffer[y+125] = (unsigned char)x;
                   GFXDisplayBuffer[y+126] = (unsigned char)x;
                   GFXDisplayBuffer[y+127] = (unsigned char)x;
                   GFXDisplayBuffer[y+128] = (unsigned char)x;
                   GFXDisplayBuffer[y+129] = (unsigned char)x;
                   GFXDisplayBuffer[y+130] = (unsigned char)x;
                   GFXDisplayBuffer[y+131] = (unsigned char)x;
                   GFXDisplayBuffer[y+132] = (unsigned char)x;
                   GFXDisplayBuffer[y+133] = (unsigned char)x;
                   GFXDisplayBuffer[y+134] = (unsigned char)x;
                   GFXDisplayBuffer[y+135] = (unsigned char)x;
                   GFXDisplayBuffer[y+136] = (unsigned char)x;
                   GFXDisplayBuffer[y+137] = (unsigned char)x;
                   GFXDisplayBuffer[y+138] = (unsigned char)x;
                   GFXDisplayBuffer[y+139] = (unsigned char)x;
                   GFXDisplayBuffer[y+140] = (unsigned char)x;
                   GFXDisplayBuffer[y+141] = (unsigned char)x;
                   GFXDisplayBuffer[y+142] = (unsigned char)x;
                   GFXDisplayBuffer[y+143] = (unsigned char)x;
                   GFXDisplayBuffer[y+144] = (unsigned char)x;
                   GFXDisplayBuffer[y+145] = (unsigned char)x;
                   GFXDisplayBuffer[y+146] = (unsigned char)x;
                   GFXDisplayBuffer[y+147] = (unsigned char)x;
                   GFXDisplayBuffer[y+148] = (unsigned char)x;
                   GFXDisplayBuffer[y+149] = (unsigned char)x;
                   GFXDisplayBuffer[y+150] = (unsigned char)x;
                   GFXDisplayBuffer[y+151] = (unsigned char)x;
                   GFXDisplayBuffer[y+152] = (unsigned char)x;
                   GFXDisplayBuffer[y+153] = (unsigned char)x;
                   GFXDisplayBuffer[y+154] = (unsigned char)x;
                   GFXDisplayBuffer[y+155] = (unsigned char)x;
                   GFXDisplayBuffer[y+156] = (unsigned char)x;
                   GFXDisplayBuffer[y+157] = (unsigned char)x;
                   GFXDisplayBuffer[y+158] = (unsigned char)x;
                   GFXDisplayBuffer[y+159] = (unsigned char)x;

                   GFXDisplayBuffer[y+160] = (unsigned char)x;
                   GFXDisplayBuffer[y+161] = (unsigned char)x;
                   GFXDisplayBuffer[y+162] = (unsigned char)x;
                   GFXDisplayBuffer[y+163] = (unsigned char)x;
                   GFXDisplayBuffer[y+164] = (unsigned char)x;
                   GFXDisplayBuffer[y+165] = (unsigned char)x;
                   GFXDisplayBuffer[y+166] = (unsigned char)x;
                   GFXDisplayBuffer[y+167] = (unsigned char)x;
                   GFXDisplayBuffer[y+168] = (unsigned char)x;
                   GFXDisplayBuffer[y+169] = (unsigned char)x;
                   GFXDisplayBuffer[y+170] = (unsigned char)x;
                   GFXDisplayBuffer[y+171] = (unsigned char)x;
                   GFXDisplayBuffer[y+172] = (unsigned char)x;
                   GFXDisplayBuffer[y+173] = (unsigned char)x;
                   GFXDisplayBuffer[y+174] = (unsigned char)x;
                   GFXDisplayBuffer[y+175] = (unsigned char)x;
                   GFXDisplayBuffer[y+176] = (unsigned char)x;
                   GFXDisplayBuffer[y+177] = (unsigned char)x;
                   GFXDisplayBuffer[y+178] = (unsigned char)x;
                   GFXDisplayBuffer[y+179] = (unsigned char)x;
                   GFXDisplayBuffer[y+180] = (unsigned char)x;
                   GFXDisplayBuffer[y+181] = (unsigned char)x;
                   GFXDisplayBuffer[y+182] = (unsigned char)x;
                   GFXDisplayBuffer[y+183] = (unsigned char)x;
                   GFXDisplayBuffer[y+184] = (unsigned char)x;
                   GFXDisplayBuffer[y+185] = (unsigned char)x;
                   GFXDisplayBuffer[y+186] = (unsigned char)x;
                   GFXDisplayBuffer[y+187] = (unsigned char)x;
                   GFXDisplayBuffer[y+188] = (unsigned char)x;
                   GFXDisplayBuffer[y+189] = (unsigned char)x;
                   GFXDisplayBuffer[y+190] = (unsigned char)x;
                   GFXDisplayBuffer[y+191] = (unsigned char)x;

                   GFXDisplayBuffer[y+192] = (unsigned char)x;
                   GFXDisplayBuffer[y+193] = (unsigned char)x;
                   GFXDisplayBuffer[y+194] = (unsigned char)x;
                   GFXDisplayBuffer[y+195] = (unsigned char)x;
                   GFXDisplayBuffer[y+196] = (unsigned char)x;
                   GFXDisplayBuffer[y+197] = (unsigned char)x;
                   GFXDisplayBuffer[y+198] = (unsigned char)x;
                   GFXDisplayBuffer[y+199] = (unsigned char)x;
                   GFXDisplayBuffer[y+200] = (unsigned char)x;
                   GFXDisplayBuffer[y+201] = (unsigned char)x;
                   GFXDisplayBuffer[y+202] = (unsigned char)x;
                   GFXDisplayBuffer[y+203] = (unsigned char)x;
                   GFXDisplayBuffer[y+204] = (unsigned char)x;
                   GFXDisplayBuffer[y+205] = (unsigned char)x;
                   GFXDisplayBuffer[y+206] = (unsigned char)x;
                   GFXDisplayBuffer[y+207] = (unsigned char)x;
                   GFXDisplayBuffer[y+208] = (unsigned char)x;
                   GFXDisplayBuffer[y+209] = (unsigned char)x;
                   GFXDisplayBuffer[y+210] = (unsigned char)x;
                   GFXDisplayBuffer[y+211] = (unsigned char)x;
                   GFXDisplayBuffer[y+211] = (unsigned char)x;
                   GFXDisplayBuffer[y+212] = (unsigned char)x;
                   GFXDisplayBuffer[y+213] = (unsigned char)x;
                   GFXDisplayBuffer[y+214] = (unsigned char)x;
                   GFXDisplayBuffer[y+215] = (unsigned char)x;
                   GFXDisplayBuffer[y+216] = (unsigned char)x;
                   GFXDisplayBuffer[y+217] = (unsigned char)x;
                   GFXDisplayBuffer[y+218] = (unsigned char)x;
                   GFXDisplayBuffer[y+219] = (unsigned char)x;
                   GFXDisplayBuffer[y+220] = (unsigned char)x;
                   GFXDisplayBuffer[y+221] = (unsigned char)x;
                   GFXDisplayBuffer[y+222] = (unsigned char)x;
                   GFXDisplayBuffer[y+223] = (unsigned char)x;
                   GFXDisplayBuffer[y+224] = (unsigned char)x;
                   GFXDisplayBuffer[y+225] = (unsigned char)x;
                   GFXDisplayBuffer[y+226] = (unsigned char)x;
                   GFXDisplayBuffer[y+227] = (unsigned char)x;
                   GFXDisplayBuffer[y+228] = (unsigned char)x;
                   GFXDisplayBuffer[y+229] = (unsigned char)x;
                   GFXDisplayBuffer[y+230] = (unsigned char)x;
                   GFXDisplayBuffer[y+231] = (unsigned char)x;
                   GFXDisplayBuffer[y+232] = (unsigned char)x;
                   GFXDisplayBuffer[y+233] = (unsigned char)x;
                   GFXDisplayBuffer[y+234] = (unsigned char)x;
                   GFXDisplayBuffer[y+235] = (unsigned char)x;
                   GFXDisplayBuffer[y+236] = (unsigned char)x;
                   GFXDisplayBuffer[y+237] = (unsigned char)x;
                   GFXDisplayBuffer[y+238] = (unsigned char)x;
                   GFXDisplayBuffer[y+239] = (unsigned char)x;
                   GFXDisplayBuffer[y+240] = (unsigned char)x;
                   GFXDisplayBuffer[y+241] = (unsigned char)x;
                   GFXDisplayBuffer[y+242] = (unsigned char)x;
                   GFXDisplayBuffer[y+243] = (unsigned char)x;
                   GFXDisplayBuffer[y+244] = (unsigned char)x;
                   GFXDisplayBuffer[y+245] = (unsigned char)x;
                   GFXDisplayBuffer[y+246] = (unsigned char)x;
                   GFXDisplayBuffer[y+247] = (unsigned char)x;
                   GFXDisplayBuffer[y+248] = (unsigned char)x;
                   GFXDisplayBuffer[y+249] = (unsigned char)x;
                   GFXDisplayBuffer[y+250] = (unsigned char)x;
                   GFXDisplayBuffer[y+251] = (unsigned char)x;
                   GFXDisplayBuffer[y+252] = (unsigned char)x;
                   GFXDisplayBuffer[y+253] = (unsigned char)x;
                   GFXDisplayBuffer[y+254] = (unsigned char)x;
                   GFXDisplayBuffer[y+255] = (unsigned char)x;
                   GFXDisplayBuffer[y+256] = (unsigned char)x;
                   GFXDisplayBuffer[y+257] = (unsigned char)x;
                   GFXDisplayBuffer[y+258] = (unsigned char)x;
                   GFXDisplayBuffer[y+259] = (unsigned char)x;
                   GFXDisplayBuffer[y+260] = (unsigned char)x;
                   GFXDisplayBuffer[y+261] = (unsigned char)x;
                   GFXDisplayBuffer[y+262] = (unsigned char)x;
                   GFXDisplayBuffer[y+263] = (unsigned char)x;
                   GFXDisplayBuffer[y+264] = (unsigned char)x;
                   GFXDisplayBuffer[y+265] = (unsigned char)x;
                   GFXDisplayBuffer[y+266] = (unsigned char)x;
                   GFXDisplayBuffer[y+267] = (unsigned char)x;
                   GFXDisplayBuffer[y+268] = (unsigned char)x;
                   GFXDisplayBuffer[y+269] = (unsigned char)x;
                   GFXDisplayBuffer[y+270] = (unsigned char)x;
                   GFXDisplayBuffer[y+271] = (unsigned char)x;
                   GFXDisplayBuffer[y+272] = (unsigned char)x;
                   GFXDisplayBuffer[y+273] = (unsigned char)x;
                   GFXDisplayBuffer[y+274] = (unsigned char)x;
                   GFXDisplayBuffer[y+275] = (unsigned char)x;
                   GFXDisplayBuffer[y+276] = (unsigned char)x;
                   GFXDisplayBuffer[y+277] = (unsigned char)x;
                   GFXDisplayBuffer[y+278] = (unsigned char)x;
                   GFXDisplayBuffer[y+279] = (unsigned char)x;
                   GFXDisplayBuffer[y+280] = (unsigned char)x;
                   GFXDisplayBuffer[y+281] = (unsigned char)x;
                   GFXDisplayBuffer[y+282] = (unsigned char)x;
                   GFXDisplayBuffer[y+283] = (unsigned char)x;
                   GFXDisplayBuffer[y+284] = (unsigned char)x;
                   GFXDisplayBuffer[y+285] = (unsigned char)x;
                   GFXDisplayBuffer[y+286] = (unsigned char)x;
                   GFXDisplayBuffer[y+287] = (unsigned char)x;
                   GFXDisplayBuffer[y+288] = (unsigned char)x;
                   GFXDisplayBuffer[y+289] = (unsigned char)x;
                   GFXDisplayBuffer[y+290] = (unsigned char)x;
                   GFXDisplayBuffer[y+291] = (unsigned char)x;
                   GFXDisplayBuffer[y+292] = (unsigned char)x;
                   GFXDisplayBuffer[y+293] = (unsigned char)x;
                   GFXDisplayBuffer[y+294] = (unsigned char)x;
                   GFXDisplayBuffer[y+295] = (unsigned char)x;
                   GFXDisplayBuffer[y+296] = (unsigned char)x;
                   GFXDisplayBuffer[y+297] = (unsigned char)x;
                   GFXDisplayBuffer[y+298] = (unsigned char)x;
                   GFXDisplayBuffer[y+299] = (unsigned char)x;
                   GFXDisplayBuffer[y+300] = (unsigned char)x;
                   GFXDisplayBuffer[y+301] = (unsigned char)x;
                   GFXDisplayBuffer[y+302] = (unsigned char)x;
                   GFXDisplayBuffer[y+303] = (unsigned char)x;
                   GFXDisplayBuffer[y+304] = (unsigned char)x;
                   GFXDisplayBuffer[y+305] = (unsigned char)x;
                   GFXDisplayBuffer[y+306] = (unsigned char)x;
                   GFXDisplayBuffer[y+307] = (unsigned char)x;
                   GFXDisplayBuffer[y+308] = (unsigned char)x;
                   GFXDisplayBuffer[y+309] = (unsigned char)x;
                   GFXDisplayBuffer[y+310] = (unsigned char)x;
                   GFXDisplayBuffer[y+311] = (unsigned char)x;
                   GFXDisplayBuffer[y+311] = (unsigned char)x;
                   GFXDisplayBuffer[y+312] = (unsigned char)x;
                   GFXDisplayBuffer[y+313] = (unsigned char)x;
                   GFXDisplayBuffer[y+314] = (unsigned char)x;
                   GFXDisplayBuffer[y+315] = (unsigned char)x;
                   GFXDisplayBuffer[y+316] = (unsigned char)x;
                   GFXDisplayBuffer[y+317] = (unsigned char)x;
                   GFXDisplayBuffer[y+318] = (unsigned char)x;
                   GFXDisplayBuffer[y+319] = (unsigned char)x;
                   
                   //}
                   __delay_ms(1);
        }

        __delay_ms(10);
        x = x + 257;
        //G1CON2bits.DPTEST = 2;
    }

    return 0;
}







