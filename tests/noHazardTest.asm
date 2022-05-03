	lw	1 0 data
	noop
	noop
	noop
	lw	2 0 neg2
	noop
	noop
	noop
	sw	1 0 50
	noop
	noop
	noop
	sw	2 0 56
	noop
	noop
	noop
start	beq	1 0 end
	noop
	noop
	noop
	add	1 1 2
	noop
	noop
	noop
	nand	3 2 1
	noop
	noop
	noop
	beq	0 0 start
	noop
	noop
	noop
end	halt
data	.fill 26
neg2	.fill -2
