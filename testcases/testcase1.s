MOV R0,#0
B	bb2  /*Branch to finish basic block*/
bb2:
LDR	R1,[PC,#-16]  /*Load MOV R0,#0 to register*/
LDR	R2,[PC,#-16]  /*Load B #-4 to register*/
ADD	R1,R1,R2
loop:      /*Add them and store them in R1*/
CMP	R0,#0
BEQ loop
