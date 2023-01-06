	@程序信息
	.arch armv7-a
	.fpu vfpv3-d16


	@全局变量iter，其值为1，大小为4字节，类型为object
	.text
	.global	iter
	.section	.rodata
	.align	2
	.size	iter, 4
iter:
	.word	1


	@adder函数 类型为function
	.text
	.align	1
	.global	adder
	.type	adder, %function
adder:
    add r0, r1
	bx	lr				@返回函数调用位置
    

	@LC0，LC1，LC2，三个会用到的字符串
	.section	.rodata
	.align	2
.LC0:
	.ascii	"%d\000"

	.align	2
.LC1:
	.ascii	"%d\012\000"

	.align	2
.LC2:
	.ascii	"enter the number greater than 0!\000"
	
	@main函数，类型为function
	.text
	.align	1
	.global	main
	.syntax unified
	.thumb
	.thumb_func
	.type	main, %function
main:
	push	{lr}
    sub	sp, sp, #20		@开辟栈帧
	add	r7, sp, #0  
	movs	r3, #0 
	str	r3, [r7, #4]	@变量a
	movs	r3, #1
	str	r3, [r7, #8]    @变量b
	movs	r3, #1
	str	r3, [r7, #12]   @变量i
	mov	r1, r7 
	ldr	r3, .L10
.L1:
	add	r3, pc
	mov	r0, r3 			@r0存储了LC0字符串的地址
	bl	__isoc99_scanf(PLT) 		@调用输入函数，获得变量n，存储在栈顶
	ldr	r3, [r7]
	cmp	r3, #1  		@比较n==1
	bne	.L3
	ldr	r1, [r7, #4]	@n=1，打印a，通过r1和r0传递打印参数
	ldr	r0, .L10+4
.L2:
	add	r0, pc
	bl	printf(PLT) 	@r0中存了字符串，r1中存了要输出的值a，进行输出
	b	.L9				@跳转到终止
.L3:
	cmp	r3, #2			@比较n==2
	bne	.L5
	ldr	r1, [r7, #8]	@n=2，打印b，通过r1和r0传递打印参数
	ldr	r0, .L10+8
.L4:
	add	r0, pc
	bl	printf(PLT)		@同理打印b
	b	.L9
.L5:
	cmp	r3, #0			@比较n是否小于1，不大于0
	bgt	.L7
	ldr	r0, .L10+12		@n<1，打印字符串"enter the number greater than 0!"
.L6:
	add	r0, pc
	bl	puts(PLT) 		@打印字符串
	b	.L9

.L7:
	ldr	r3, [r7]		@获取n的值，存储在r3
	ldr	r2, [r7, #12] 	@获取i的值，存储在r2，
	cmp	r2, r3			@while循环的条件判断
	blt	.L8				@n<i，进行循环体执行一次循环
	ldr	r1, [r7, #4]	@循环结束，准备打印a的值
	ldr	r0, .L10+16
    b   .L9
.L8:
	ldr	r3, [r7, #8]	@获取b的值，存储在r3
	str	r3, [r7, #16]	@把b给到t
	ldr	r1, [r7, #8]	@获取b的值，存储在r1
	ldr	r0, [r7, #4]	@获取a的值，存储在r0
	bl	adder(PLT)		@进行加法，参数通过r1，r0传入，结果通过r0返回
	str	r0, [r7, #8]	@b=a+b
	ldr	r3, [r7, #16]	@获取t的值，存储在r3
	str	r3, [r7, #4]	@a=t
	ldr	r3, [r7, #12]	@获取到i的值，存储在r3
	adds	r3, r3, #1
	str	r3, [r7, #12]	@i=i+1
    b  .L7
.L9:
	add	r0, pc
	bl	printf(PLT)
	movs	r3, #0
	mov	r0, r3
	adds	r7, r7, #20
	mov	sp, r7
	pop	{pc}
	.align	2
.L10:
	.word	.LC0-(.L1+4)
	.word	.LC1-(.L2+4)
	.word	.LC1-(.L4+4)
	.word	.LC2-(.L6+4)
	.word	.LC1-(.L9+4)