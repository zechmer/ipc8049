; -----------------------------------------------------------------------------
; ISR external interrupt = SER2RXD & SER1TXD (either H->L)
; -----------------------------------------------------------------------------
EXT_INT:    JF0  EXT_INT_2      
            RETR
            
EXT_INT_2:  SEL  RB1        ; bank 1
            MOV  R7, A       
            MOV  A, #0xFF    ;  init for 1 event
            MOV  T, A        
            EN   TCNTI      
            STRT CNT        ; start counter: will interrupt at next H->L on T1 input
            DIS  I          ; dis ext. interrupt
            MOV  A, R7       
            RETR            ; RETR also restores bank 0??