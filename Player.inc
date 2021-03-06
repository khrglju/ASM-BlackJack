.data
	
	playerCount DWORD 1
	countPrompt BYTE "Enter the number of players: ", 0
	namePrompt BYTE "Enter the name of player ", 0
	betPrompt BYTE "Enter the bet for ", 0
	betPrompt2 BYTE ": ", 0

	tooFewPeople BYTE "Too few players (1 min).",0Ah,0
	tooManyPeople BYTE "Too many players (5 max).",0Ah,0
	NameTooLong BYTE "Name is too long, try again: ",0
	NameTooShort BYTE "You must enter a name: ",0
	TenDollarMinimum BYTE "There is $10 bet minimum. Try again: ",0
	ProspectiveBet DWORD 0 ; http://support.microsoft.com/kb/74666
	NotEnoughInBank BYTE "Not enough money in the bank. Bet less: ",0
	CantBet BYTE "Can't bet, not enough money in the bank.",0Ah,0
	CantDoubleDown BYTE "Can't double down, not enought money in the bank. ",0Ah,0

.code

;--------------------------------------------------------------------
;displays all players in game
outputPlayers PROC
	mov ecx, playerCount
	mov eax, 1
	outloop:
		call display
		call Testhand
		call crlf
		inc eax
	loop outloop

	RET
outputPlayers ENDP

;--------------------------------------------------------------------
;Start of game input number of players and names of each
nameInput PROC USES EAX EBX ECX EDX ESI
	getHeadCount:
	mov edx, offset countPrompt
	call WriteString
	call ReadInt
	mov playerCount, eax
	.IF SDWORD PTR eax < 1
		mov edx,OFFSET tooFewPeople
		call WriteString
		jmp getHeadCount
	.ELSEIF eax > 5
		mov edx,OFFSET tooManyPeople
		call WriteString
		jmp getHeadCount
	.ENDIF

	mov ebx, 1

	mov ecx, playercount

	MOV edi, 0

	NameLoop:
			mov edx, offset namePrompt
			call WriteString
			mov eax, ebx
			mov eax, ' '
			call WriteChar
		push ecx
		lea EDX, (Playerstructure PTR playerarray[edi]).pname
		mov ecx, SIZEOF (Playerstructure PTR playerarray[edi]).pname
		tryNameAgain:
		call ReadString
		.IF eax > 21
			push edx
			push ecx
			mov edx,OFFSET NameTooShort
			call WriteString
			pop ecx
			pop edx
			jmp tryNameAgain
		.ELSEIF SDWORD PTR eax < 1
			push edx
			push ecx
			mov edx,OFFSET NameTooShort
			call WriteString
			pop ecx
			pop edx
			jmp tryNameAgain
		.ENDIF
		inc ebx
		pop ecx
		add edi, TYPE Playerstructure
	loop NameLoop
			

	RET
nameInput ENDP

;--------------------------------------------------------------------
;displays player name, bank, and bet | pass eax player number 1-5
display PROC USES EBX ECX EDX ESI EAX EDI
		
		CALL GetPlayerOffset
		lea EDX, (Playerstructure PTR playerarray[esi]).pname
		Mov EAX, (Playerstructure PTR playerarray[esi]).bankacc
		Mov EBX, (Playerstructure PTR playerarray[esi]).bet
			
		printinfo:
			CALL WriteString
			CALL Crlf
			CALL WriteDec
			CALL Crlf
			MOV EAX, EBX
			CALL WriteDec
			CALL Crlf


	RET
display ENDP

;--------------------------------------------------------------------
;update bank pass eax player number (1-5) and ebx amount to change update bank
deposit PROC USES ESI EAX EBX

	CALL GetPlayerOffset
	;add ebx to players bank account
	ADD (Playerstructure PTR playerarray[esi]).bankacc, EBX
	
	RET
deposit ENDP

;--------------------------------------------------------------------
;update bank pass eax player number and ebx amount to change update bank
withdraw PROC USES ESI EAX EBX

	CALL GetPlayerOffset
	;sub ebx to players bank account
	SUB (Playerstructure PTR playerarray[esi]).bankacc, EBX
	
	RET
withdraw ENDP

;--------------------------------------------------------------------
;pass player number in EAX(1-5) withdraws money from account and places in bet
PlaceBet PROC USES ECX EDX ESI
	
	CALL GetPlayerOffset

	;betting prompt for players
	mov edx, offset betprompt
	call writeString
	lea edx, (Playerstructure PTR playerarray[esi]).Pname
	call writeString
	mov edx, offset betprompt2
	call writestring
	push eax
	TryBetAgain:
	mov eax,0
	call readInt
	mov ebx,eax
	add eax,((PlayerStructure PTR PlayerArray[ESI]).bet) ; Needed?
	mov ProspectiveBet,eax
	mov eax,0
	mov eax,((PlayerStructure PTR PlayerArray[ESI]).BankAcc)
	.IF (ProspectiveBet > SDWORD PTR eax) && (SDWORD PTR ebx < 10)
		mov edx,OFFSET CantBet
		call WriteString
		;; FIXME go back to menu
		ret
	.ELSEIF SDWORD PTR ebx < 10
		mov edx,OFFSET TenDollarMinimum
		call WriteString
		jmp TryBetAgain
	.ELSEIF ProspectiveBet > SDWORD PTR eax ; if current bet + prospective bet is greater than bank, try again.
		mov edx,OFFSET NotEnoughInBank
		call WriteString
		jmp TryBetAgain
	.ENDIF
	pop eax
	;---------------------------

	;add ebx to players bet
	ADD (Playerstructure PTR playerarray[esi]).bet, EBX
	;withdraw amount from players bank | pops player number from eax so it withdraws from correct player	
	CALL withdraw
	
	RET
PlaceBet ENDP

;--------------------------------------------------------------------
;for double down send player number in EAX
doubleBet PROC USES EAX ESI EBX
	CALL GetPlayerOffset
	PUSH EAX
	;double down doubles your bet, but checks that you have enough money first.
	MOV EBX,(Playerstructure PTR playerarray[esi]).bet
	add ebx,(Playerstructure PTR playerarray[esi]).bet
	mov eax,(Playerstructure PTR playerarray[esi]).bankacc
	.IF eax < ebx
		MOV ShouldShowDoubleError, 1
		pop eax
	.ELSE
		mov (Playerstructure PTR playerarray[esi]).bet, EBX
		;withdraw amount from players bank | pops player number from eax so it withdraws from correct player
		pop eax
		CALL withdraw
	.ENDIF

	RET
doubleBet ENDP

;--------------------------------------------------------------------
;for double down send player number in EAX
doubleSplitBet PROC USES EAX ESI EBX
	CALL GetPlayerOffset
	;double down doubles your bet 
	MOV EBX, (Playerstructure PTR playerarray[esi]).Splitbet
	ADD (Playerstructure PTR playerarray[esi]).bet, EBX
	;withdraw amount from players bank | pops player number from eax so it withdraws from correct player
	CALL withdraw

	RET
doubleSplitBet ENDP

;--------------------------------------------------------------------
;send the winning player number in EAX
winBet PROC USES ESI EAX EBX
	
	
	CALL GetPlayerOffset
	;sub ebx to players bank account
	MOV EBX, (Playerstructure PTR playerarray[esi]).bet
	;calls deposit twice so it replaces the initial bet and then the amount won
		CALL deposit
		CALL deposit
	;sets bet to 0
	MOV (Playerstructure PTR playerarray[esi]).bet, 0
	RET
winBet ENDP

;--------------------------------------------------------------------
;send the winning player number in EAX
winSplitBet PROC USES ESI EAX EBX
	
	
	CALL GetPlayerOffset
	;sub ebx to players bank account
	MOV EBX, (Playerstructure PTR playerarray[esi]).Splitbet
	;calls deposit twice so it replaces the initial bet and then the amount won
		
		CALL deposit
		CALL deposit
	;sets bet to 0
	MOV (Playerstructure PTR playerarray[esi]).Splitbet, 0
	RET
winSplitBet ENDP

;--------------------------------------------------------------------
;send the losing player number in EAX
loseBet PROC USES ESI EAX EBX
	
	CALL GetPlayerOffset
	;sets bet to 0
	MOV (Playerstructure PTR playerarray[esi]).bet, 0

	RET
loseBet ENDP

;--------------------------------------------------------------------
;send the losing player number in EAX
loseSplitBet PROC USES ESI EAX EBX
	
	CALL GetPlayerOffset
	;sets bet to 0
	MOV (Playerstructure PTR playerarray[esi]).splitbet, 0

	RET
loseSplitBet ENDP

;--------------------------------------------------------------------
;pass player number in EAX returns the bet to that player
pushBet PROC USES ESI EAX EBX
	CALL GetPlayerOffset

	;sub ebx to players bank account
	MOV EBX, (Playerstructure PTR playerarray[esi]).bet
	;calls deposit twice so it replaces the initial bet and then the amount won
		CALL deposit
	;sets bet to 0
	MOV (Playerstructure PTR playerarray[esi]).bet, 0
	RET
pushBet ENDP

;--------------------------------------------------------------------
;pass player number in EAX returns the bet to that player
pushSplitBet PROC USES ESI EAX EBX
	CALL GetPlayerOffset

	;sub ebx to players bank account
	MOV EBX, (Playerstructure PTR playerarray[esi]).Splitbet
	;calls deposit twice so it replaces the initial bet and then the amount won
		
		CALL deposit
	;sets bet to 0
	MOV (Playerstructure PTR playerarray[esi]).Splitbet, 0
	RET
pushSplitBet ENDP

;Takes in EAX the player number
;Returns in ESI the player offset
GetPlayerOffset PROC USES EAX
	mov esi, 0
	;this will add the type of playerstructure to esi, making it go to the corresponding player in the array	
	.while (eax > 1)
		add esi, TYPE Playerstructure
		dec eax
	.endw
	RET
GetPlayerOffset ENDP

;Takes in player number in EAX
;Sets CurrentPlayerMessage to the player's message
GetPlayerMessage PROC USES EAX ESI
	CALL GetPlayerOffset
	MOV AL, (Playerstructure PTR playerarray[ESI]).Message
	MOV CurrentPlayerMessage, AL
	RET
GetPlayerMessage ENDP

;Takes in player number in EAX, Message number in CurrentPlayerMessage
;Sets CurrentPlayerMessage to the player's message
SetPlayerMessage PROC USES EAX ESI
	CALL GetPlayerOffset
	MOV AL, CurrentPlayerMessage
	MOV (Playerstructure PTR playerarray[ESI]).Message, AL
	RET
SetPlayerMessage ENDP