	lw	1 0 data
	lw	2 0 neg1
start	add	1 1 2
	beq	1 0 end
	beq	0 0 start
end	halt
data	.fill 5
neg1	.fill -1
