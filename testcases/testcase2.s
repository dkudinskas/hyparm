MOV R0, #125   /*Beginadres: 80300000*/
MOV R1, #5
MOV R2, #5
multiply:
MUL R2, R1, R2
CMP R0, R2
BNE multiply

MOV	R0,#0       /*R0 is counter*/
MOV R4,#24        /*(The number of instructions that have to be copied +1)times 4 */
LDR R1,start_address /*Start reading from address 80300000*/
LDR R2,start_address	/*Destination of copied instructions 0x80300040*/
LDR R5,address_in_first_bb
ADD	R2,R2,#0x4C /* MOVT cannot be used because it isn't implemented in the hypervisors*/
next_word:
LDR	R3,[R1,+R0]
STR	R3,[R2,+R0]
ADD	R0, R0,#4
CMP	R0,R4
BNE	next_word
STR	R15,[R5]          /*Write the PC in first basisblok (so that that block gets invalidated*/
B	start_address
start_address:
.word	0x80008000
address_in_first_bb:
.word	0x8000800c
