;************************************************************************
;*                                                                      *
;*                        l a u n c h . a s m                           *
;*                                                                      *
;*      This is the launcher for Medley on DOS.  It:                    *
;*                                                                      *
;*      * Looks for and validates any -m memsize argument on the        *
;*        command line.  Only values in the range 8 - 32 are allowed.   *
;*                                                                      *
;*      * Loads the real Medley emulator, emul.exe, from the same       *
;*        directory that the launcher came from.                        *
;*                                                                      *
;*      * Sets the Intel DOS Extender's profile to the requested        *
;*        memory size + 3Mb (to allow for the emulator, internal        *
;*        data areas, etc.)                                             *
;*                                                                      *
;*      * Set the termination address in the child process's PSP,       *
;*        so control returns to the launcher when Medley terminates.    *
;*                                                                      *
;*      * Jumps to the emulator's start address.                        *
;*                                                                      *
;*      * Upon return, just terminates cleanly.                         *
;*                                                                      *
;*      [We could perhaps do some diagnosis here of error returns?]     *
;*                                                                      *
;*                                                                      *
;*                                                                      *
;************************************************************************

;************************************************************************/
;*                                                                      */
;*      (C) Copyright 1993, 1994  Venue. All Rights Reserved.           */
;*      Manufactured in the United States of America.                   */
;*                                                                      */
;*      The contents of this file are proprietary information           */
;*      belonging to Venue, and are provided to you under license.      */
;*      They may not be further distributed or disclosed to third       */
;*      parties without the specific permission of Venue.               */
;*                                                                      */
;************************************************************************/

.model small
.386p
.stack 100h
.data 
	align 8
    ;********************************************************
    ;*  Parameter block for INT 214B, that loads medley.exe *
    ;********************************************************
envseg  DW      0       ; environment (0 = copy mine)
cmdip   DW      ?       ; command-line-tail pointer
cmdcs   DW      ?
fcb1    DD      ?       ; dummy first FCB to fill in
fcb2    DD      ?       ; " 2nd FCB, not here in DOS 4.01???
stk     DD      ?       ;  SS:SP for emul.exe, filled in by loader
csip    DD      ?       ; start addr for emul.exe, filled in by loader



retad   DD      FAR PTR myret   ; cs:ip return address, to put in
				; child PSP, so we get control back.


    ;* Error messages, misc strings, and work areas*
	align   8
memval  dd      0
errstg  DB      'ERROR:  Couldn''t free excess storage.',13,10,'$'
noload  db      'ERROR: Loading emulator failed:  $'
loaded  db      'LOAD SUCCESSFUL.',13,10,'$'
nominfo db      'ERROR: -m must be followed by a number 8 - 64.',13,10,'$'
badexe  db      'ERROR: emul.exe is corrupted.',13,10,'$'
emulpath DB     'emul.exe',0    ; name of the real emulator.
mflag   db      '-m'            ; to search for -m/-M in cmd line
mmflag  db      '-M'    
profile db      'PRO'           ; to find the DOS extender profile
cmdline db      128 dup (?)             ; hold the cmd line tail for real emulator

	;* Error-message table for failures loading emul.exe
	align   2
errtbl  dw      OFFSET ng0msg   ; 0 = unknown failure
	dw      OFFSET ng1msg   ; 1 = "invalid function"
	dw      OFFSET ng2msg   ; 2 = file not found
	dw      OFFSET ng3msg   ; 3 = path not found
	dw      OFFSET ng4msg   ; 4 = too many open files
	dw      OFFSET ng5msg   ; 5 = access denied
	dw      OFFSET ng0msg   ; 6 = not possible error
	dw      OFFSET ng0msg   ; 7 = not possible error
	dw      OFFSET ng8msg   ; 8 = insufficient storage
	dw      OFFSET ng0msg   ; 9 = not possible
	dw      OFFSET ngamsg   ; A = bad environment
	dw      OFFSET ngbmsg   ; B = bad format (corrupt .exe?)a

ng0msg  db      'Unknown problem',13,10,'$'
ng1msg  db      'Invalid Function',13,10
	db      'Make sure you are running DOS 4.0 or later.',13,10,'$'
ng2msg  db      'File not found.',13,10
	db      'CD to proper directory, or set PATH.',13,10,'$'
ng3msg  db      'Path not found.',13,10
	db      'CD to proper directory, or set PATH.',13,10,'$'
ng4msg  db      'Too many files open.',13,10
	db      'Shut down some TSRs that have file open?',13,10,'$'
ng5msg  db      'Access denied.',13,10
	db      'Make sure of your access rights to emul.exe?',13,10,'$'
ng8msg  db      'Not enough memory.',13,10
	db      'Shut down some TSR applications?',13,10,'$'
ngamsg  db      'Environment corrupt.',13,10
	db      'Check using SET; You may need to re-boot.',13,10,'$'
ngbmsg  db      'EXE file corrupted.',13,10,'$'
	db      'You may need to restore from backup or re-install.',13,10,'$'
.code



;************************************************************************
;*                                                                      *
;*                              M A C R O S                             *
;*                                                                      *
;*      prints  Given a string ptr in DX, print it to the display.      *
;*                                                                      *
;*      kill    Exit cleanly, using INT 21/4C                           *
;*                                                                      *
;************************************************************************

prints  macro   text
	mov     dx, OFFSET text
	mov     ah,9
	int     21h
	endm

kill    macro
	mov ah,4ch
	int 21h
	endm



;************************************************************************
;*                                                                      *
;*                      M A I N   E N T R Y                             *
;*                                                                      *
;*                                                                      *
;*                                                                      *
;*                                                                      *
;*                                                                      *
;************************************************************************

PUBLIC  main
main    PROC    NEAR

	mov     ax,ds           ; Save memory-block start for freeing
	mov     es,ax           ; excess memory in a bit.

	mov     ax,@data        ; DS points to start of data segment
	mov     ds,ax

	mov     cmdcs, ax       ; Copy the command line for the emulator
	mov     cmdip, OFFSET cmdline

	mov     di, OFFSET cmdline
	mov     cx,128
	mov     bx,es
	mov     dx,ds
	mov     es,dx
	mov     ds,bx
	mov     si,80h

	rep
	movsb

	mov     es,bx           ; Free the excess memory that DOS gives
	mov     ds,dx           ; us (we need it for the emulator)

	mov     ax,4a00h
	mov     bx,090h         ; We only need 900h bytes for this program
	int 21h
	jnc     freeok

	prints errstg           ; Couldn't free spare space; punt.
	kill

	;************************************************
	;*  Search the command line for -m or -M        *
	;************************************************
freeok: 
	mov     di,81h          ; start of command line tail
	mov     si, OFFSET mflag
	mov     cx, 2

	mov     bx,81h
	add     bl,es:[80h]

m1lp:   call strcmp
	je      fndm

	add     di, 1
	cmp     di, bx
	jl      m1lp

	mov     di,81h          ; start of command line tail
	mov     si, OFFSET mmflag

m2lp:   call strcmp
	je      fndm

	add     di, 1
	cmp     di, bx
	jl      m2lp

	mov     memval,02400000h        ; memory value not set--use 35MB total.
	jmp     doload

fndm:   add     di,2            ; Found "-m".  Now look for a number
	cmp     di,bx           ; (Make sure it's not end of line)
	jnl     nogoodm

ok1:
	mov     edx, 0          ; Holds the memory-requirement value
	mov     ax,0            ; holds characters as we read

	;************************************************
	;*  Skip over spaces/tabs before any number     *
	;************************************************
skiplp:
	mov     al, es:[di]
	inc     di
	cmp     al, 20h         ; spaces
	je      skiplp
	cmp     al,  09h        ; tabs
	je      skiplp
	cmp     di,bx           ; make sure we're still in the string
	jle     cnvst           ; Yup, we've got the first char, so enter
				; the conversion loop part-way down.

nogoodm:
	prints  nominfo         ; no arg to -m, or it's bad; Punt.
	kill

	;********************************************************
	;   Convert the numeric argument to -m; result in edx.  *
	;********************************************************
cnvlp:  mov     al,es:[di]
	add     di, 1
cnvst:  cmp     al, 30h
	jl      endcnv
	cmp     al, 39h
	jg      endcnv
	sub     al, 30h
	imul    dx, 10
	add     dx, ax
	jmp     cnvlp

endcnv:
	cmp     edx,0           ; if still 0, no valid chars!
	je      nogoodm
	cmp     edx, 8          ; must be in the range [8, 32]
	jl      nogoodm
	cmp     edx,64
	jg      nogoodm

	add     edx, 3          ; add 3mb for data areas, etc, and
	sal     edx, 20         ; convert to megabytes
	mov     memval, edx     ; save memory requested

	;************************************************
	;*  Load the real emulator .EXE file, emul.exe  *
	;************************************************
doload: mov     dx, OFFSET emulpath
	mov     ax, seg envseg
	mov     es, ax
	mov     bx, OFFSET envseg
	mov     ax,4b01h        ; load-don't-start
	int 21h
	jnc     loadok

	add     ax,ax
	mov     si,ax
	prints  noload
	mov     bx,OFFSET errtbl
	mov     dx,ds:[bx+si]
	mov     ah,9
	int 21h
	kill

loadok:                         ; Load succeeded.
	mov     ah,51h          ; get PSP address for child
	int     21h
	mov     es, bx          ; get segment for DI addressing

;       mov     cx,128          ; copy the command line tail
;       mov     di,80h          ; (which appears to be flaky in DOS 4)
;       mov     si, offset cmdline
;       rep
;       movsb

	mov     eax,retad
	mov     dword ptr es:[+0ah], eax        ; set up return address.

	cmp     memval, 0       ; If no -m value given, just
	je      dorun           ; go start the emulator.

	mov     di,0            ; Search for the Intel Extender's PROFILE
	mov     si, OFFSET profile      ; (see extender.h)
	mov     cx, 3           ; (length is 3 bytes)

srchlp: call    strcmp
	je      gotprof         ; found the profile; fix it.
	add     di, 1
	cmp     di, 2048
	jle     srchlp

	prints  badexe          ; No extender profile, so the emulator
	kill                    ; EXE must be corrupt.  Punt.

gotprof:
	mov     eax,memval              ; Fill in the memory requirement.
	mov     es:[di+1bch], eax

;********************************************************
;*                                                      *
;*      Set up the stack seg/pointer & start medley.    *
;*                                                      *
;********************************************************
dorun:  lss     sp,stk          ; load stack SS & SP regs
	mov     ax, es          ; copy PSP ptr to ax & ds, since some
	mov     bx, ds          ; code expects it in both places.
	mov     fs,bx           ; Also, copy DS to FS, so we still have
	mov     ds,ax           ; a base for the indirect jump . . .
	jmp     fs:[csip]       ; to start-of-medley.

myret:  kill                    ; we get back here, so quit gracefully.

main    endp



;************************************************************************/
;*                                                                      */
;*                              s t r c m p                             */
;*                                                                      */
;*      Compare [ds]di and es:[si] for (CX) characters.  If the         */
;*      strings are equal, the Zero flag is set when this returns.      */
;*                                                                      */
;*      All registers are preserved.                                    */
;*                                                                      */
;************************************************************************/

strcmp  proc    near
	cld
	push di
	push si
	push cx

	repe
	cmpsb

	pop     cx
	pop     si
	pop     di
	ret
strcmp  endp

END
