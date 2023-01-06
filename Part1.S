	@程序信息
	.arch armv7-a
	.fpu vfpv3-d16
	@常量声明
	.section	.rodata

	.align	2
.LC0:
	.ascii	"%d%d\000"			@ \000字符串结束
 
	.align	2
.LC1:
	.ascii	"input: x=%d,y=%d\012\000"		@ \012\000是换行加字符串结束

	.align	2
.LC2:
	.ascii	"doubleAlu: add=%d,sub=%d,mul=%d,div=%d,mod=%d\012\000"

	.align	2
.LC3:
	.ascii	"singleAlu: singleAdd=%d,singleSub=%d,x=%d\012\000"

	.align	2
.LC4:
	.ascii	"relation: eq=%d,gt=%d,lt=%d,ge=%d,le=%d,ne=%d\012\000"

	.align	2
.LC5:
	.ascii	"logic: b1=%d,b2=%d,b3=%d\012\000"

	@除法取模调用
  	.global __aeabi_idiv			@idiv除法
	.global	__aeabi_idivmod			@idivmod取模
	@代码段
	.text
	.align	1
	.global	main
	.syntax unified					@不可删
	.thumb					    	@不可删
	.thumb_func				  		@不可删
	
main:
 	@栈帧调整
	push	{lr}			
	sub	sp, sp, #56
	add	r7, sp, #0

.L1:
  	@输入x,y
	add	r2, r7, #20					@r2存y的地址（scanf第三个参数）
	add	r1, r7, #16					@r1存x的地址（scanf第二个参数）
	ldr	r0, .L7					  	@r0是.LC0（scanf第一个参数）
	bl	__isoc99_scanf(PLT)			@scanf调用

  	@说明：r8是x，r9是y，r10用来存放结果
  	ldr	r8, [r7, #16]       		@r8是x
	ldr	r9, [r7, #20]      			@r9是y

	@加法
	add	r10, r8, r9
	str	r10, [r7, #24]
	@减法
	subs r10, r8, r9
	str	r10, [r7, #28]
	@乘法
	mul	r10, r8, r9
	str	r10, [r7, #32]
	@除法
	mov	r1, r9
	mov	r0, r8
	bl	__aeabi_idiv(PLT)		 	@arm没有除法和取模运算
	mov	r10, r0             		@r0是商
	str	r10, [r7, #36]
	@模运算
	mov	r1, r9
	mov	r0, r8
	bl	__aeabi_idivmod(PLT)
	mov	r10, r1						@r0是商，r1是余数
	str	r10, [r7, #40]

	@++x	可调序
	adds	r10, r8, #1
	str	r10, [r7, #16]
  	ldr	r8,  [r7, #16]
	str	r10, [r7, #44]
	@-y
	rsbs	r10, r9, #0				@逆向减法 后减前
	str	r10, [r7, #48]

	@等于
	cmp	r8, r9
	ite	eq
	moveq	r10, #1
	movne	r10, #0
	strb	r10, [r7, #7]
	@大于
	cmp	r8, r9
	ite	gt
	movgt	r10, #1
	movle	r10, #0
	strb	r10, [r7, #8]
	@小于
	cmp	r8, r9
	ite	lt
	movlt	r10, #1
	movge	r10, #0
	strb	r10, [r7, #9]
	@大于等于
	cmp	r8, r9
	ite	ge
	movge	r10, #1
	movlt	r10, #0
	strb	r10, [r7, #10]
	@小于等于
	cmp	r8, r9
	ite	le
	movle	r10, #1
	movgt	r10, #0
	strb	r10, [r7, #11]
	@不等于
	cmp	r8, r9
	ite	ne
	movne	r10, #1
	moveq	r10, #0
	strb	r10, [r7, #12]

	@算b1结果
	cmp	r8, #1
	bgt	.L2		
	cmp	r8, #-1       				@x<=1 或运算左边为0，b1为x< -1的bool值
	bge	.L3
.L2:								@ x>1 或运算左边为1，b1确定为1;或运算左边为0且右边为1，b1确定为1
	movs r10, #1
	b	.L4
.L3:								@ x>=-1 或运算右边为0，b1确定为0
	movs	r10, #0
.L4:
	strb	r10, [r7, #13]			@存储b1结果

	@算b2结果
	cmp	r9, #0
	ite	le							@机智，避开非运算 !>直接用<=
	movle	r10, #1
	movgt	r10, #0
	strb	r10, [r7, #14]

	@算b3结果
	ldrb	r3, [r7, #13]	
	cmp	r3, #0
	beq	.L5
	ldrb	r3, [r7, #14]			@与运算左边为1，还得看右边b2值
	cmp	r3, #0
	beq	.L5				
	ldrb	r3, [r7, #13]			@与运算左边为1，右边b2值为1，b3确定为b1；
	cmp	r3, #0						@看b1为0还是1
	ite	ne
	movne	r3, #1					@b3为b1值1
	moveq	r3, #0					@b3为b1值0
	b	.L6
.L5:								@与运算左边为0，b3确定为b2；与运算左边为1，右边b2值为0，b3确定为b2；
	ldrb	r3, [r7, #14]	
	cmp	r3, #0						@看b2为0还是1
	ite	ne
	movne	r3, #1					@b3为b2值1
	moveq	r3, #0					@b3为b2值0
.L6:
	strb	r3, [r7, #15]			

	@输出x-1与y
	ldr	r2, [r7, #20]				@r2是y	（prinf1第三个参数）
	subs    r1,r8,#1				@r1是x-1（prinf1第二个参数）
	ldr	r0, .L7+4					@r0是.LC1（prinf1第一个参数）
	bl	printf(PLT)

	@输出双目运算结果
	ldr	r3, [r7, #40]				@mod结果，压栈（prinf2第六个参数）
	str	r3, [sp, #4]
	ldr	r3, [r7, #36]				@div结果，压栈（prinf2第五个参数）
	str	r3, [sp]
	ldr	r3, [r7, #32]				@r3是mul结果（prinf2第四个参数）（四个寄存器存参数）其他参数需要压栈！
	ldr	r2, [r7, #28]				@r2是sub结果（prinf2第三个参数）
	ldr	r1, [r7, #24]				@r1是add结果（prinf2第二个参数）
	ldr	r0, .L7+8					@r0是.LC2（prinf2第一个参数）
	bl	printf(PLT)

	@输出单目运算结果
	ldr	r3, [r7, #16]				@r3是x结果  （prinf3第四个参数）
	ldr	r2, [r7, #48]				@r2是-y结果 （prinf3第三个参数）
	ldr	r1, [r7, #44]				@r1是++x结果（prinf3第二个参数）
	ldr	r0, .L7+12					@r0是.LC3	（prinf3第一个参数）
	bl	printf(PLT)

	@输出关系运算结果
	ldrb	r1, [r7, #7]			@r1是EQ结果  （prinf4第二个参数）
	ldrb	r2, [r7, #8]			@r2是GT结果  （prinf4第三个参数）
	ldrb	r3, [r7, #9]			@r3是LT结果  （prinf4第四个参数）（四个寄存器存参数）其他参数需要压栈！
	ldrb	r4, [r7, #12]			@NE
  	ldrb	r5, [r7, #11]			@LE
  	ldrb	r6, [r7, #10]			@GE
	str	r4, [sp, #8]				@NE结果，压栈（prinf4第七个参数）
	str	r5, [sp, #4]				@LE结果，压栈（prinf4第六个参数）
	str	r6, [sp]					@GE结果，压栈（prinf4第五个参数）
	ldr	r0, .L7+16					@r0是.LC4   （prinf4第一个参数）
	bl	printf(PLT)

	@输出逻辑运算结果
	ldrb	r1, [r7, #13]			@r1是b1结果（prinf5第二个参数）
	ldrb	r2, [r7, #14]			@r2是b2结果（prinf5第三个参数）
	ldrb	r3, [r7, #15]			@r3是b3结果（prinf5第四个参数）
	ldr	r0, .L7+20					@r0是.LC5（prinf5第一个参数）
	bl	printf(PLT)
 
	@栈帧调整
	adds	r7, r7, #56				@r7到旧栈顶
	mov	sp, r7						@恢复sp
	pop	{pc}			            @弹栈							

	.align	2

	@ 桥接全局变量的地址
.L7:
	.word	.LC0
	.word	.LC1
	.word	.LC2
	.word	.LC3
	.word	.LC4
	.word	.LC5
