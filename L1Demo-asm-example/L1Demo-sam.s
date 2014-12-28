;------------------------------------------------------------------------------
; Simple assembly project for L1Demo board
; paint random rectangles in 640x480x2bpp mode
; performance is about 6500 rect/sec for 1bpp mode and 6100 rect/sec for 2bpp
; (C)2014 Leo Bodnar
; subject to WTFPL licence terms
;------------------------------------------------------------------------------

				.include	"macros.inc"
				.include	"p24FJ256DA206.inc"

;------------------------------------------------------------------------------
; Configuration bits:
;------------------------------------------------------------------------------

				config	__CONFIG1	FWDTEN_OFF & GWRP_OFF & GCP_OFF & JTAGEN_OFF
				config	__CONFIG2 	POSCMOD_HS & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2
				config	__CONFIG3   ALTPMP_ALTPMPEN & SOSCSEL_EC

;------------------------------------------------------------------------------
; VGA timing
; http://martin.hinner.info/vga/timing.html
;------------------------------------------------------------------------------
				.equiv		H_ACTIVE,	640
				.equiv		H_TOTAL,	800
				.equiv		HS_START,	0
				.equiv		HS_WIDTH,	96
				.equiv		HENST,		96 + 37	; I had to increase front porch to get better picture on my LCD screen

				.equiv		V_ACTIVE,	480
				.equiv		V_TOTAL,	525
				.equiv		VS_START,	0
				.equiv		VS_WIDTH,	2
				.equiv		VENST,		10

				.equiv		GPU_H,		H_ACTIVE
				.equiv		GPU_V,		V_ACTIVE

				.equiv		FONT_LEN,	1544	; length of the font structure in bytes
;------------------------------------------------------------------------------
; GFX commands
;------------------------------------------------------------------------------

				.equiv		CHR_FGCOLOR,	0x5000
				.equiv		CHR_BGCOLOR,	0x5100
				.equiv		CHR_FONTBASE,	0x5200
				.equiv		CHR_PRINTCHAR,	0x5300
				.equiv		CHR_TXTAREASTART,0x5800
				.equiv		CHR_TXTAREAEND,	0x5900
				.equiv		CHR_PRINTPOS,	0x5A00
				.equiv		RCC_SRCADDR, 	0x6200
				.equiv		RCC_DESTADDR, 	0x6300
				.equiv		RCC_RECTSIZE, 	0x6400
				.equiv		RCC_COLOR, 		0x6600
				.equiv		RCC_STARTCOPY, 	0x6700

;========================================================= USB =========================================================================
; General RAM
;=======================================================================================================================================
					.section	RAM, bss

Counter:			.space	4					; 4 byte counter
Random:				.space	4

STACK:				.space	128					; each "call" uses 4 bytes and "push" 2 bytes 

Font:				.space	FONT_LEN

;========================================================= USB =========================================================================
; Video buffer RAM
;=======================================================================================================================================
					.section	DISPLAY, bss, eds

VRAM:				.space	(GPU_H * GPU_V / 4)	; 1 byte is 4 pixels in 2bpp mode

;------------------------------------------------------------------------------
; Main code
; Operating at 16 MIPS
;------------------------------------------------------------------------------
			.text								; Start of Code section
			.global 	__reset						; main entry
__reset:

        	mov 		#STACK,    w15			; Initalize Stack Pointer (w15)


			clr			ANSB					; switch all pins to digital
			clr			ANSC
			clr			ANSD
			clr			ANSF
			clr			ANSG

			call		InitGFX					; init GFX controller 

												; copy font from ROM to RAM
			mov			#Font, w1				; destination address in RAM
			mov			#tblpage(FontStart), w0	; Font source in program memory
			mov			w0, TBLPAG
			mov			#tbloffset(FontStart), w0

			repeat		#FONT_LEN / 2 - 1		; copy ROM to RAM
			tblrdl		[w0++], [w1++]

			CHECK_GPU_QUEUE						; inside "macros.inc" - check if GFX queue is full

			mov			#0xFFFF, w0				; font colour is white
			mov			w0,G1CMDL
			mov			#CHR_FGCOLOR, w0
			mov			w0,G1CMDH 

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			#0x0000, w0				; font background is black
			mov			w0,G1CMDL
			mov			#CHR_BGCOLOR, w0
			mov			w0,G1CMDH

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			#Font, w0				; tell GPU where our Font is in RAM
			mov			w0,G1CMDL
			mov			#CHR_FONTBASE, w0
			mov			w0,G1CMDH 

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			#0x0000, w0				; text area starts at 0,0
			mov			w0,G1CMDL
			mov			#CHR_TXTAREASTART, w0
			mov			w0,G1CMDH 

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			#(H_ACTIVE&0xF)<<12|V_ACTIVE, w0
			mov			w0,G1CMDL
			mov			#CHR_TXTAREAEND|H_ACTIVE>>4, w0
			mov			w0,G1CMDH				; text area is the full screen 

			mov			#1, w0
			mov			w0, Random + 0			; seed = 0x00000001
			clr			Random + 2

			clr			Counter + 0				; counter = 0
			clr			Counter + 2


;/-------------- main loop -------------------------------\
mainLoop:
            com.b     LATBH
            inc.b     TRISBH

			inc			Counter + 0				; counter++		
			bra			NZ, 1f
			inc			Counter + 2
1:			
			; paint a random rectangle

			call		GetRandom				; get 32 bit random number into w1 w2
			mov			w0, w3
			mov			w1,	w2

			call		GetRandom				; get 32 bit random number into w1 w2

			and			#255, w0				; limit rectangle start Y to 0..255
			and			#511, w1				; limit rectangle start X to 0..511

			and			#127, w2
			add			#80,  w2				; limit rectangle height to 80..207

			and			#127, w3
			add			#1,   w3				; limit rectangle width to 1..128

			mov			Counter, w4
			and			#3, w4					; colour is alternating between black and white

			call		Rectangle_Fill

			; print a text line at the bottom

			CHR_PRINT_POSITION	470, 190		; inside "macros.inc"

			mov			#edspage  (strHello), w1
			mov			#edsoffset(strHello), w0
			call		PrintString

			mov			Counter + 2, w0			; print 32bit hex number
			call		prn0000
			mov			Counter + 0, w0
			call		prn0000

			mov			#edspage  (strRect), w1
			mov			#edsoffset(strRect), w0
			call		PrintString

			bra 		mainLoop
;\-------------- main loop -------------------------------/


;---------------------------------------------------------------------------------------------------------------
; configure graphics display
;---------------------------------------------------------------------------------------------------------------
InitGFX:
												
			mov			#VRAM, w0				; tell video system and GFX where our videobuffer is
			mov			w0, G1DPADRL
			mov			w0, G1W1ADRL
			mov			w0, G1W2ADRL
			clr			G1DPADRH				; VRAM high word = 0 if our buffer start is below 64K 
			clr			G1W1ADRH
			clr			G1W2ADRH
 
			mov			#0xFFFF, w0				; enable all 16 video data pins GD0..15
			mov			w0, G1DBEN

			bset		CLKDIV, #G1CLKSEL		; set GFX clock to 96MHz 

			mov			#11<<GCLKDIV0, w0		; set pixel clock to 96MHz / 3.75 = 25.600MHz
			mov			w0, CLKDIV2

			mov			#H_ACTIVE, w0
			mov			w0, G1DPW				; DPW = display width
			mov			#GPU_H, w0
			mov			w0, G1PUW				; GFX width

			mov			#V_ACTIVE, w0
			mov			w0, G1DPH				; DPH = display height
			mov			#GPU_V, w0
			mov			w0, G1PUH				; GFX height

			mov			#H_TOTAL, w0
			mov			w0, G1DPWT				; DPWT = width total
			mov			#V_TOTAL, w0
			mov			w0, G1DPHT				; DPHT = height total

			mov			#HS_WIDTH, w0
			mov.b		WREG, G1HSYNCH			; HSLEN = HS width
			mov			#HS_START, w0
			mov.b		WREG, G1HSYNCL			; HSST = HS start delay

			mov			#VS_WIDTH, w0
			mov.b		WREG, G1VSYNCH			; VSLEN = VS width
			mov			#VS_START, w0
			mov.b		WREG, G1VSYNCL			; VSST = VS start delay

			mov			#VENST, w0
			mov.b		WREG, G1DBLCONH			; VENST = V blank to active
			mov.b		WREG, G1ACTDAH			; ACTLINE = lines before 1st active line
			mov			#HENST, w0
			mov.b		WREG, G1DBLCONL			; HENST = H blank to active
			mov.b		WREG, G1ACTDAL			; ACTPIX = pixels before 1st active pixel

			mov			#(1<<PUBPP0), w0		; PUBPP<2:0>: GPU bits-per-pixel (bpp) Setting bits
			mov			w0, G1CON1				; 4 = 16 bits-per-pixel
												; 3 = 8 bits-per-pixel
												; 2 = 4 bits-per-pixel
												; 1 = 2 bits-per -pixel
												; 0 = 1 bit-per-pixel

			mov			#(1<<DPBPP0 | 1<<DPMODE0 | 0<<DPTEST0), w0	; 1bpp, TFT mode
			mov			w0, G1CON2				; DPBPP<2:0>: Display bits-per-pixel Setting bits
												;	This setting must match the GPU bits-per-pixel set in PUBPP<2:0> (G1CON1<7:5>).
												;	4 = 16 bits-per-pixel
												;	3 = 8 bits-per-pixel
												;	2 = 4 bits-per-pixel
												;	1 = 2 bits-per-pixel
												;	0 = 1 bit-per-pixel
												; DPTEST<1:0>: Display Test Pattern Generator bits
												; 	3 = Borders
												; 	2 = Bars
												; 	1 = Black screen
												; 	0 = Normal Display mode; test patterns are off
												; DPMODE<2:0>: Display Glass Type bits
												; 	1 = TFT type

			mov			#1<<DPPINOE|1<<DPPOWER|0<<DPVSPOL|0<<DPHSPOL|1<<DPVSOE|1<<DPHSOE, w0
			mov			w0, G1CON3				; DPVSPOL: Display Vertical Synchronization (VSYNC) Polarity bit
												;	1 = Active-high (VSYNC)
												;	0 = Active-low (-VSYNC)
												; DPHSPOL: Display Horizontal Synchronization (HSYNC) Polarity bit
												;	1 = Active-high (HSYNC)
												;	0 = Active-low (-HSYNC)

												; init CLUT here if needed

			bset		G1CON1, #G1EN			; enable display

			repeat #16000						; 1 msec delay to let GFX start up			
			nop
												; clear screen
			mov			#0, w0					; y start
			mov			#0, w1					; x start
			mov			#V_ACTIVE, w2			; height
			mov			#H_ACTIVE, w3			; width
			mov			#0x0000, w4				; colour = black
			call		Rectangle_Fill			; clear screen

			return


;
; w0 Y position	top left corner
; w1 X position	top left corner
; w2 width
; w3 height
; w4 colour

Rectangle_Fill:

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			w4, G1CMDL
			mov			#RCC_COLOR, w4
			mov			w4, G1CMDH				; Colour 

			CHECK_GPU_QUEUE						; check if GFX queue is full

			sl			w3, #12, w4				; H << 12
			add			w4,  w2, w4				; H << 12 + W
			mov			w4, G1CMDL
			lsr			w3, #4,  w3				; H >> 4
			mov			#RCC_RECTSIZE, w4
			add			w3, w4, w4
			mov			w4, G1CMDH				; Size

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			#H_ACTIVE, w2
			mul.uu		w2, w0, w2				; w3:w2 = 640 * x
			add			w2, w1, w1
			mov			w1, G1CMDL
			mov			#RCC_DESTADDR, w1
			add			w3, w1, w1
			mov			w1, G1CMDH				; Position

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			#0b0001100000, w0		; OPER=000(solid fill), ROP=0xC, DT=0
			mov			w0,G1CMDL
			mov			#RCC_STARTCOPY, w0
			mov			w0,G1CMDH 

			return


;------------------------------------------------------
; print zero-terminated string stored in program memory 
; w0 string EDS offset
; w1 string EDS page
;------------------------------------------------------
PrintString:

			push		DSRPAG					; save current EDS read setting

			mov			w1, DSRPAG
1:
			ze			[w0++], w1				; get new byte, zero extend to full word
			bra			Z, 2f					; char ==0, end of string, "2f" means go to the first label "2" forward

			CHECK_GPU_QUEUE						; check if GFX queue is full

			mov			w1, G1CMDL				; print CHAR
			mov			#CHR_PRINTCHAR, w1
			mov			w1, G1CMDH 

			cp0			w0						; check for [a very unlikely event of] the string rolling over 32K boundary
			bra			NZ, 1b					; no, get new char. "1b" means go to the first label "1" back
												; well, we have rolled over to new EDS page
			mov			#0x8000, w0				; Reset pointer to 0x8000, EDS page acceess window start
			inc			DSRPAG					; EDS page += 1
			bra			1b
2:
			pop			DSRPAG					; restore EDS
			return

;------------------------------------
; print w0 in hex
;------------------------------------
prn0000:
			push		w1						; save w1

			CHECK_GPU_QUEUE						; check if GFX queue is full

			lsr			w0, #12, w1				; shift right 0x1234 -> 0x0001
			cp			w1, #10					; compare to 10
			bra			LTU, 1f					; branch if Less Than, Unsigned
			add			#'A'-'0'-10, w1			; if > 10 add up to "A"
1:			add			#'0',w1					; add up to "0"
			mov			w1,G1CMDL
			mov			#CHR_PRINTCHAR, w1
			mov			w1,G1CMDH				; GFX print

			CHECK_GPU_QUEUE						; check if GFX queue is full

			lsr			w0, #8, w1				; shift right 0x1234 -> 0x0012
			and			#0xF, w1				; 0x0012 -> 0x0002
			cp			w1, #10
			bra			LTU, 1f
			add			#'A'-'0'-10, w1
1:			add			#'0',w1
			mov			w1,G1CMDL
			mov			#CHR_PRINTCHAR, w1
			mov			w1,G1CMDH 

			CHECK_GPU_QUEUE						; check if GFX queue is full

			lsr			w0, #4, w1				; shift right 0x1234 -> 0x0123
			and			#0xF, w1				; 0x0123 -> 0x0003
			cp			w1, #10
			bra			LTU, 1f
			add			#'A'-'0'-10, w1
1:			add			#'0',w1
			mov			w1,G1CMDL
			mov			#CHR_PRINTCHAR, w1
			mov			w1,G1CMDH 

			CHECK_GPU_QUEUE						; check if GFX queue is full

			and			w0, #0xF, w1			; 0x1234 -> 0x0004
			cp			w1, #10
			bra			LTU, 1f
			add			#'A'-'0'-10, w1
1:			add			#'0',w1
			mov			w1,G1CMDL
			mov			#CHR_PRINTCHAR, w1
			mov			w1,G1CMDH 

			pop			w1						; restore w1

			return

;-------------------------------
; get random number into w1:w0
; http://www.firstpr.com.au/dsp/rand31/
;-------------------------------

GetRandom:
			push		w2
			push		w3
			push		w12

			mov			#16807, w0
			mov			#Random + 2, w12

			mul.uu		w0, [w12--], w2
			mul.uu		w0, [w12], w0

			sl			w2, w2
			rlc			w3, w3
			lsr			w2, w2

			addc		w0, w3, w0
			addc		w2, w1, w1

			bra			NN, 1f

			bclr		w1, #15
			inc			w0, w0
			addc		#0, w1
1:
			mov			w1, Random + 2
			mov			w0, Random + 0

			pop			w12
			pop			w3
			pop			w2

			return


strHello:	.asciz		" Hello world! L1Demo "
strRect:	.asciz		" rectangles "

.include 	"Fonts.inc"

.end                               ;End of program code in this file
