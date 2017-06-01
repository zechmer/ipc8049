; *****************************************************************************
;  IPC 8049,  Sinclair QL,  JV 970715 
; *****************************************************************************
; This a new, restructured version of the original IPC 8049 ROM disassembly
; No function has been changed
; Bernhard Fechner 
; -----------------------------------------------------------------------------

.include "definitions.inc"         
JMP  IPCInit

.org #0x3
; EXT Interrupt
JMP  EXT_INT	; ISR external interrupt = SER2RXD & SER1TXD (either H->L)

.org #0x7
; TIMER interrupt
JMP  TIMER		; ISR timer; get here when counter++ overflows from #0xFF->#0x00
; -----------------------------------------------------------------------------
.include "ipcinit.inc"           
.include "external_int.inc"
.include "timer.inc"            

.include "ipcom.inc"
.include "keyboard.inc"
.include "math.inc"
.include "comm.inc"            
.exit