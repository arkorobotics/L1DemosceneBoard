// Example Code by bldewolf
// https://github.com/bldewolf/l1demo

#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <string.h>
#include "fonts.h"

#define  FCY    16000000UL    // Instruction cycle frequency, Hz
#include <libpic30.h>

_CONFIG1(FWDTEN_OFF & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
_CONFIG2(POSCMOD_HS & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2)
_CONFIG3(ALTPMP_ALTPMPEN & SOSCSEL_EC)

#define HOR_RES 640UL
#define VER_RES 480UL
#define HOR_FRONT_PORCH 16
#define HOR_PULSE_WIDTH  96
#define HOR_BACK_PORCH 48
#define VER_FRONT_PORCH 11
#define VER_PULSE_WIDTH 2
#define VER_BACK_PORCH 31
#define BPP 2

#define CHR_FGCOLOR		0x5000
#define CHR_BGCOLOR		0x5100
#define CHR_FONTBASE		0x5200
#define CHR_PRINTCHAR		0x5300
#define CHR_TXTAREASTART	0x5800
#define CHR_TXTAREAEND		0x5900
#define CHR_PRINTPOS		0x5A00
#define RCC_SRCADDR	 	0x6200
#define RCC_DESTADDR	 	0x6300
#define RCC_RECTSIZE	 	0x6400
#define RCC_COLOR	 	0x6600
#define RCC_STARTCOPY	 	0x6700

#define GFX_BUFFER_SIZE (HOR_RES * VER_RES / (8/BPP))
__eds__ uint8_t GFXDisplayBuffer[GFX_BUFFER_SIZE] __attribute__((eds, section("DISPLAY"), address(0x1000)));

void config_graphics(void) {
	_G1CLKSEL = 1;
	_GCLKDIV = 11;

	G1DPADRL = (unsigned long)(GFXDisplayBuffer) & 0xFFFF;
	G1DPADRH = 0;
	G1W1ADRL = (unsigned long)(GFXDisplayBuffer) & 0xFFFF;
	G1W1ADRH = 0;
	G1W2ADRL = (unsigned long)(GFXDisplayBuffer) & 0xFFFF;
	G1W2ADRH = 0;

	_GDBEN = 0xFFFF;

	// Using PIC24F manual section 43 page 37-38
	_DPMODE = 1;      /* TFT */
	_GDBEN = 0xFFFF;
	_DPW = _PUW = HOR_RES; // Work area and FB size so GPU works
	_DPH = _PUH = VER_RES;
	_DPWT = HOR_FRONT_PORCH + HOR_PULSE_WIDTH + HOR_BACK_PORCH + HOR_RES;
	_DPHT = VER_FRONT_PORCH + VER_PULSE_WIDTH + VER_BACK_PORCH + VER_RES;
	_DPCLKPOL = 0;
	_DPENOE = 0;
	_DPENPOL = 0;
	_DPVSOE = 1;      /* use VSYNC */
	_DPHSOE = 1;      /* use HSYNC */
	_DPVSPOL = 0;     /* VSYNC negative polarity */
	_DPHSPOL = 0;     /* HSYNC negative polarity */
	_ACTLINE = _VENST = VER_FRONT_PORCH + VER_PULSE_WIDTH + VER_BACK_PORCH;
	_ACTPIX = _HENST = HOR_FRONT_PORCH + HOR_PULSE_WIDTH + HOR_BACK_PORCH;
	_VSST = VER_FRONT_PORCH;
	_HSST = HOR_FRONT_PORCH;
	_VSLEN = VER_PULSE_WIDTH;
	_HSLEN = HOR_PULSE_WIDTH;
	_DPPWROE = 0;
	_DPPINOE = 1;
	_DPPOWER = 1;
	_DPBPP = _PUBPP = BPP / 2;


//	_DPTEST = 2;

	_G1EN = 1;
	__delay_ms(1);
}

void config_chr(void) {
	while(_CMDFUL) continue;
	G1CMDL = 0xFFFF;
	G1CMDH = CHR_FGCOLOR;
	Nop();

	while(_CMDFUL) continue;
	G1CMDL = 2;
	G1CMDH = CHR_BGCOLOR;
	Nop();

	while(_CMDFUL) continue;
	G1CMDL = (uint16_t)(FontStart) & 0xFFFF;
	G1CMDH = CHR_FONTBASE;
	Nop();

	while(_CMDFUL) continue;
	G1CMDL = 0;
	G1CMDH = CHR_TXTAREASTART;
	Nop();

	while(_CMDFUL) continue;
	G1CMDL = (HOR_RES & 0xF)<<12 | VER_RES;
	G1CMDH = CHR_TXTAREAEND | (HOR_RES >>4);
	Nop();
}

void chr_print(char *c) {
	while(_CMDFUL) continue;
	G1CMDL = 0;
	G1CMDH = CHR_PRINTPOS;
	Nop();

	while(*c != NULL) {
		while(_CMDFUL) continue;
		G1CMDL = *c;
		G1CMDH = CHR_PRINTCHAR;
		Nop();

		c++;
	}
}

void rcc_color(char color) {
	while(_CMDFUL) continue;
	G1CMDL = color;
	G1CMDH = RCC_COLOR;
	Nop();
}

void rcc_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	// destination
	while(_CMDFUL) continue;
	G1CMDL = x + y*HOR_RES;
	G1CMDH = RCC_DESTADDR | (x + y*HOR_RES)>>16;
	Nop();

	// size
	while(_CMDFUL) continue;
	G1CMDL = (w<<12) | h;
	G1CMDH = RCC_RECTSIZE | (w>>4);
	Nop();

	// go!
	while(_CMDFUL) continue;
	G1CMDL = 0xC<<3;
	G1CMDH = RCC_STARTCOPY;
	Nop();
}

void blank_background() {
	rcc_color(0);
	rcc_draw(0, 0, HOR_RES, VER_RES);
}

int main(void) {
	//OSCCON = 0x0000;

	ANSB = 0x0000;
	ANSC = 0x0000;
	ANSD = 0x0000;
	ANSF = 0x0000;
	ANSG = 0x0000;
	//TRISE = 0x0000;

	config_graphics();
	config_chr();

	blank_background();
	chr_print("Hello");

	uint8_t c = 0;
	while (1) {
		rcc_color(c++ % 4);
		rcc_draw(100, 0, 200, 200);
//		rcc_draw(0, 0, HOR_RES, VER_RES);
		__delay_ms(1000);
	}

	return 0;
}
