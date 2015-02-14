#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <string.h>
#include <math.h>
#include "fonts.h"

#define  FCY    16000000UL    // Instruction cycle frequency, Hz
#include <libpic30.h>

_CONFIG1(FWDTEN_OFF & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
_CONFIG2(POSCMOD_HS & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2)
_CONFIG3(ALTPMP_ALTPMPEN & SOSCSEL_EC)

/*
   To change resolutions, use an X modeline generator like:
   http://xtiming.sourceforge.net/cgi-bin/xtiming.pl
   For example, 80x480@60Hz produces
   Modeline "80x480@61" 4.93 80 112 128 160 480 490 495 505
   Then we convert it to the values we need:

   4.93 is the pixel clock in MHz, we need to set CLOCKDIV to match.  To pick a
   divisor, use page 147 of the data sheet or, uh, this perl one-liner to
   generate all of them and pick one that's close:
   perl -e '$d = 1;  $x = 0; while($x < 128) { printf "%s %.2f\n",$x,96/$d;if($x < 64) { $d += 0.25; } elsif($x < 96) { $d += 0.50 } else { $d += 1 }; $x++}'

   The differences between: 80 112 128 160 480
   are H-porch, H-Pulse, H-width
   The differences between: 480 490 495 505
   are V-porch, V-Pulse, V-width

   Once these values are in, the monitor will display it but it will probably
   be misaligned (especially vertically).  Play with VENST_FUDGE and
   HENST_FUDGE to fix this.
*/

#define HOR_RES 80UL
#define VER_RES 480UL
#define HOR_FRONT_PORCH 32
#define HOR_PULSE_WIDTH 8
#define HOR_BACK_PORCH  32
#define VER_FRONT_PORCH 10
#define VER_PULSE_WIDTH 5
#define VER_BACK_PORCH  10
#define BPP 8
#define CLOCKDIV 69
#define VENST_FUDGE 0
#define HENST_FUDGE 0
#define VSPOL 0 /* sync polarities */
#define HSPOL 0

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
#define IPU_SRCADDR		0x7100
#define IPU_DESTADDR	 	0x7200
#define IPU_DECOMPRESS	 	0x7400

#define GFX_BUFFER_SIZE (HOR_RES * VER_RES / (8/BPP))
//__eds__ uint8_t GFXDisplayBuffer[2][GFX_BUFFER_SIZE] __attribute__((eds, section("DISPLAY"), address(0x1000)));
__eds__ uint8_t GFXDisplayBuffer[2][GFX_BUFFER_SIZE] __attribute__((far, section("DISPLAY"),space(eds)));


void config_graphics(void) {
	_G1CLKSEL = 1;
	_GCLKDIV = CLOCKDIV;

	G1DPADRL = (unsigned long)(GFXDisplayBuffer) & 0xFFFF;
	G1DPADRH = (unsigned long)(GFXDisplayBuffer) >>16 & 0xFF;
	G1W1ADRL = (unsigned long)(GFXDisplayBuffer) & 0xFFFF;
	G1W1ADRH = (unsigned long)(GFXDisplayBuffer) >>16 & 0xFF;
	G1W2ADRL = (unsigned long)(GFXDisplayBuffer) & 0xFFFF;
	G1W2ADRH = (unsigned long)(GFXDisplayBuffer) >>16 & 0xFF;

	_GDBEN = 0xFFFF;

	// Using PIC24F manual section 43 page 37-38
	_DPMODE = 1;      /* TFT */
	_GDBEN = 0xFFFF;
	_DPW = _PUW = HOR_RES; // Work area and FB size so GPU works
	_DPH = _PUH = VER_RES;
	_DPWT = HOR_FRONT_PORCH + HOR_PULSE_WIDTH + HOR_BACK_PORCH + HOR_RES;
	_DPHT = VER_FRONT_PORCH + VER_PULSE_WIDTH + VER_BACK_PORCH + VER_RES + 25;//VER_RES;// + 35; // centered but shaky
	_DPCLKPOL = 0;
	_DPENOE = 0;
	_DPENPOL = 0;
	_DPVSOE = 1;
	_DPHSOE = 1;
	_DPVSPOL = VSPOL;
	_DPHSPOL = HSPOL;
	_ACTLINE = _VENST = VER_FRONT_PORCH + VER_PULSE_WIDTH + VER_BACK_PORCH - VENST_FUDGE;
	_ACTPIX = _HENST = HOR_FRONT_PORCH + HOR_PULSE_WIDTH + HOR_BACK_PORCH - HENST_FUDGE;
	_VSST = VER_FRONT_PORCH;
	_HSST = HOR_FRONT_PORCH;
	_VSLEN = VER_PULSE_WIDTH;
	_HSLEN = HOR_PULSE_WIDTH;
	_DPPWROE = 0;
	_DPPINOE = 1;
	_DPPOWER = 1;
	_DPBPP = _PUBPP = 3;


//	_DPTEST = 2; /* Uncomment for test patterns */

	_G1EN = 1;
	__delay_ms(1);
}

void config_chr(void) {
    while(_CMDFUL) continue;
    G1CMDL = 0xFFFF;
    G1CMDH = CHR_FGCOLOR;
    Nop();

    while(_CMDFUL) continue;
    G1CMDL = 0;
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

void chr_print(unsigned char *c) {
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

void rcc_color(unsigned int color) {
	while(_CMDFUL) continue;
	G1CMDL = color;
	G1CMDH = RCC_COLOR;
	Nop();
}

void rcc_setdest(__eds__ uint8_t *buf) {
	while(!_CMDMPT) continue; // Wait for GPU to finish drawing
	G1W2ADRL = (unsigned long)(buf);
	G1W2ADRH = (unsigned long)(buf);
}

void gpu_setfb(__eds__ uint8_t *buf) {
	G1DPADRL = (unsigned long)(buf);
	G1DPADRH = (unsigned long)(buf);
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
	rcc_color(0xffff);
	//rcc_color(0x0000);
	rcc_draw(0, 0, HOR_RES-1, VER_RES-1);
}

volatile int fb_ready = 0;
void __attribute__((interrupt, auto_psv))_GFX1Interrupt(void) {
	static int lines = 0;
	static int syncs = 0;
	static int next_fb = 1;
	if(_VMRGNIF) { /* on a vertical sync, flip buffers if it's ready */
		lines = 0;
		syncs++;
		if(fb_ready) {
			gpu_setfb(GFXDisplayBuffer[next_fb]);
			next_fb = !next_fb;
		}
		fb_ready = 0;
		_VMRGNIF = 0;
	} else if(_HMRGNIF) { /* on each horizontal sync, ...? */
		lines++;
		_HMRGNIF = 0;
	}
	_GFX1IF = 0;
}

#define PIX_W 1
#define PIX_H 6

#define MAX_SPRITES 10
#define SPR_HEAD 5

struct Sprite {
	uint8_t width;  // Width (in pixels)
	uint8_t height; // Height (in pixels)
	uint8_t bitres; // Bits per Pixel
	uint8_t trans;  // Transparency
	uint8_t rotate; // Rotation, 0: none, 1: 90 cw, 2: 180, 3: 90 ccw
	uint8_t *data;  // Pointer to sprite pixel data
};

struct Sprite s[MAX_SPRITES];

uint8_t SpriteMap[] __attribute__((space(eds)))= {
	0x08, 0x08, 0x08, 0x00, 0x00, /* data */
/*	0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,
	0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,
	0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,
	0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,
	0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,
	0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,
	0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,
	0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,
*/
	0xe0,0x00,0xff,0xff,0xff,0xff,0x00,0x1c,
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0xe0,0x00,0xff,0xff,0xff,0xff,0x00,0x1c,
/* // NSL Logo rotation test black background
	0xe0,0xff,0x00,0x00,0x00,0x00,0xff,0x1c,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xe0,0xff,0x00,0x00,0x00,0x00,0xff,0x03,
*/

};

/*
	0xe0 pure red
	0x1c pure green
	0x03 pure blue
*/

void loadSprite(uint8_t id) {
	// TODO: make this actually address multiple sprites
	//uint8_t off;
	//off = (uint16_t)(SpriteMap) & 0xFFFF;
	s[id].width  =  SpriteMap[0];
	s[id].height =  SpriteMap[1];
	s[id].bitres =  SpriteMap[2];
	s[id].trans  =  SpriteMap[3];
	s[id].rotate =  SpriteMap[4];
	s[id].data   = &SpriteMap[5];

	// next = s[id].width * s[id].height + SPR_HEAD
}

void drawSprite(uint16_t x, uint16_t y, uint8_t id, uint8_t rotation) {

	unsigned int w,h;
	unsigned int x1,y1;

	for (h=0; h < s[id].height; h++) {
		for (w=0; w < s[id].width; w++) {
			rcc_color(*(s[id].data + w + s[id].width*h));
			//rcc_color(rand()); tv screen effect
			switch(rotation) {
				//  00 deg	0,0 1,0 2,0 ... 0,1
				//  90 deg CCW	7,0 7,1 7,2 ... 6,0
				// 180 deg	7,7 6,7 5,7 ... 7,6
				//  90 deg CW	0,7 0,6 0,5 ... 1,6
				case 0: // 0 degree
					x1 = x+w;
					y1 = y + (h<<2) + (h<<1);//y+(PIX_H*h);
					if (x1 >= HOR_RES-2) break;
					if (y1 >= VER_RES-PIX_H) return;
					rcc_draw(x1, y1, 1,PIX_H);
					break;
				case 1: // 90 degree CW
					x1 = x+(s[id].width-h-1);
					y1 = y+(PIX_H*(w));
					if (x1 >= HOR_RES-1 || x1 <= 0) continue;
					if (y1 >= VER_RES-PIX_H || y1 <= 0) continue;
					rcc_draw(x1, y1, 1,PIX_H);
					break;
				case 2: // 180 degree
					if (x+(s[id].width-w-1) >= HOR_RES-1) continue;
					if (y+(PIX_H*(s[id].height-h-1)) >= VER_RES-PIX_H) continue;
					rcc_draw(x1, y1, 1,PIX_H);
					break;
				case 3: // 90 degree CCW
				default:
					break;
			}
		}
	}
}

void drawSpriteRotation(uint16_t x, uint16_t y, uint8_t id, uint16_t rotation) {
	int x1,y1;

        unsigned int w,h;

        for (h=0; h < s[id].height; h++) {
                for (w=0; w < s[id].width; w++) {
                        rcc_color(*(s[id].data + w + s[id].width*h));
			x1 = x * cos(rotation) - y * sin(rotation);
			y1 = x * sin(rotation) - y * cos(rotation);
			if (x1 >= HOR_RES-1) continue;
			rcc_draw(x1, y1, 1, PIX_H);
		}
	}
}

int main(void) {

	ANSB = 0x0000;
	ANSC = 0x0000;
	ANSD = 0x0000;
	ANSF = 0x0000;
	ANSG = 0x0000;
	//OSCCON = 0x0000;
	//TRISE = 0x0000;
	TRISB = 0x0000;

	config_graphics();
	config_chr();

	// clear buffers
	rcc_setdest(GFXDisplayBuffer[0]);
	blank_background();
	rcc_setdest(GFXDisplayBuffer[1]);
	blank_background();


	_VMRGNIF = 0;
	_HMRGNIF = 0;
	_HMRGNIE = 1;
	_VMRGNIE = 1;
	_GFX1IE = 1;

	loadSprite(0);
	unsigned int lol = 0;

	int next_fb = 1;
	while (1) {
		rcc_setdest(GFXDisplayBuffer[next_fb]);
		next_fb = !next_fb;

		blank_background();

		//rcc_color(0x8888);
		//rcc_draw(50,50, 20, 20*PIX_H);

		
		drawSprite(((5+lol)%HOR_RES-1),10,0, (lol)%2);
		drawSprite(((10+lol)%HOR_RES-1),60,0, (lol)%2);
		drawSprite(((15+lol)%HOR_RES-1),110,0, (lol)%2);
		drawSprite(((20+lol)%HOR_RES-1),170,0, (lol)%2);
		drawSprite(((25+lol)%HOR_RES-1),220,0, (lol)%2);
		drawSprite(((30+lol)%HOR_RES-1),270,0, (lol)%2);
		drawSprite(((35+lol)%HOR_RES-1),320,0, (lol)%2);
		drawSprite(((40+lol)%HOR_RES-1),370,0, (lol)%2);
		drawSprite(((45+lol)%HOR_RES-1),420,0, (lol)%2);
		drawSprite(((50+lol)%HOR_RES-1),470,0, (lol)%2);
		
                if(lol > 80) lol=0;
		//drawSprite(((20+lol)%HOR_RES-1),200,0, 1);//(lol/100)%3);
		//drawSprite(((5+lol)%(HOR_RES-1)),300,0, 0);
		//drawSprite(40,300,0, 0);
		//drawSprite(40,300,0, (lol)%2);
		//drawSprite(40,10,0, 0);
		lol++;

		//rcc_color(0);
		//rcc_draw(0,0,20,20*PIX_H);

		//char message[50];
		//sprintf(message, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ndisp: %04x\ndraw: %04x", GFXDisplayBuffer[0][15+50*HOR_RES], GFXDisplayBuffer[1][15+50*HOR_RES]);
		//chr_print(message);

                // screen border
                //rcc_color(0x0000);
                rcc_color(0xffff);
                rcc_draw(0,0, 1, VER_RES); // left
                rcc_draw(HOR_RES-2,0,1,VER_RES); // HOR_RES-1 // right
                rcc_draw(0,0, HOR_RES-2, PIX_H); // top
                rcc_draw(0,VER_RES-PIX_H,HOR_RES-2,PIX_H); // bottom

                // Clean right column
		rcc_color(0x0000);
                //rcc_color(0xffff);
                rcc_draw(HOR_RES-1,0, 1,VER_RES);

		while(!_CMDMPT) continue; // Wait for GPU to finish drawing
		fb_ready = 1;
		while(fb_ready) continue; // wait for gfx interrupt to flip
                __delay_ms(100);
	}

	return 0;
}