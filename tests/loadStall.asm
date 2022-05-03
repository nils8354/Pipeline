	lw 	1 0 data
	noop
	noop
	noop
	lw	2 0 neg1
start	add	1 1 2
	noop
	noop
	noop
	beq	1 0 end
	noop
	noop
	noop
	beq	0 0 start
	noop
	noop
	noop
end	halt
data	.fill 5
neg1	.fill -1
