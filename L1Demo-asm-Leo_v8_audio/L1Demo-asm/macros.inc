.macro		GPU_QUEUE_EMPTY_WAIT				; wait until GFX queue is empty
			btsc		G1STAT, #PUBUSY
			bra			$-2
.endm

.macro		CHECK_GPU_QUEUE
			nop									; need one instruction cycle delay if immediate instruction before was writing to G1CMDH
			btsc		G1STAT, #CMDFUL			; check if GFX queue
			bra			$-2
.endm

.macro		CHR_PRINT_POSITION	x, y
			CHECK_GPU_QUEUE						; macro inside macro
			mov			#(((\y)&0xF)<<12)+(\x), w0
			mov			w0,G1CMDL
			mov			#CHR_PRINTPOS+(\y)>>4, w0
			mov			w0,G1CMDH 
.endm
