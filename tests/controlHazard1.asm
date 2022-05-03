	lw	1 0 data
	noop
	lw	2 0 neg2
	noop
	lw	3 0 data2
	noop
	lw 	4 0 plus1
	noop
	lw	5 0 zero
	noop
	noop
	noop
start	beq	1 0 end
	beq	5 3 skip
	add	5 5 4
	noop
	noop
	noop
	beq	0 0 start
skip	add	1 1 2
	noop
	noop
	noop
	beq	0 0 start
end	halt
data	.fill	26
data2	.fill	5
zero	.fill	0
neg2	.fill	-2
plus1	.fill	1
