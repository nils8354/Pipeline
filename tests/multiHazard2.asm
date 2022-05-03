	lw 1 0 data
	lw 2 0 neg2
	noop
start	beq 1 0 end
	add 1 1 2
	beq 0 0 start
end	halt
data	.fill 26
neg2	.fill -2
