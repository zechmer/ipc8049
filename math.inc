.org #0x700
; =============================================================================
; MATH SUBROUTINES
; -----------------------------------------------------------------------------
; memory SUB8: ret. A=(R1)-(R0),  keep R0/R1
; memory SUB16: ret. A=((R1+1), (R1))-((R0+1), (R0)),  keep R0/R1
; memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
; 8*16bit MUL R2*(R4, R3)->(R6, R5)
; generate RANDOM nr in #0x25, #0x26
; =============================================================================

; -----------------------------------------------------------------------------
; memory SUB8: ret. A=(R1)-(R0),  keep R0/R1
; -----------------------------------------------------------------------------
memSUB8:    MOV  A, @R0      
            JZ   #0x0707      
            CPL  A          
            INC  A          
            ADD  A, @R1      
            RET             
            MOV  A, @R1      
            CLR  C          
            CPL  C          
            RET             
; -----------------------------------------------------------------------------
; memory SUB16: ret. A=((R1+1), (R1))-((R0+1), (R0)),  keep R0/R1
; -----------------------------------------------------------------------------
memSUB16:   CALL memSUB8       ;memory SUB8: ret. A=(R1)-(R0),  keep R0/R1
            JNZ  #0x0715      
            DEC  R0         
            DEC  R1         
            CALL memSUB8       ;memory SUB8: ret. A=(R1)-(R0),  keep R0/R1
            INC  R0         
            INC  R1         
            RET             
; -----------------------------------------------------------------------------
; memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
; -----------------------------------------------------------------------------
memADD:     ADD  A, @R1      
            MOV  @R1, A      
            INC  R1         
            MOV  A, #0x00      
            ADDC A, @R1      
            MOV  @R1, A      
            DEC  R1         
            RET             
; -----------------------------------------------------------------------------
; 8*16bit MUL R2*(R4, R3)->(R6, R5)
; -----------------------------------------------------------------------------
memMUL8x16: MOV  R5, #0x00     
            MOV  R6, #0x00     
            CLR  C          
            MOV  R0, #0x08     
            MOV  A, R2       
            RRC  A          
            MOV  R2, A       
            JNC  #0x0731      
            MOV  A, R3       
            ADD  A, R5       
            MOV  R5, A       
            MOV  A, R4       
            ADDC A, R6       
            MOV  R6, A       
            MOV  A, R3       
            RLC  A          
            MOV  R3, A       
            MOV  A, R4       
            RLC  A          
            MOV  R4, A       
            DJNZ R0, #0x0726   
            RET             
; -----------------------------------------------------------------------------
; generate RANDOM nr in #0x25, #0x26
; -----------------------------------------------------------------------------
GEN_RANDOM: MOV  R1, #0x26     
            MOV  A, @R1      
            MOV  R2, A       
            MOV  R3, #0x8D     
            CALL memMUL8x16       ;8*16bit MUL R2*(R4, R3)->(R6, R5)
            MOV  A, R6       
            MOV  R1, #0x26     
            MOV  @R1, A      
            DEC  R1         
            MOV  A, R5       
            CALL memADD       ;memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
            MOV  A, #0x8B      
            CALL memADD       ;memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
            RET             
