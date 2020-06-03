.section	.text

.macro isr_noerror no
.global	_isr\no
.type	_isr\no, @function
_isr\no:
	cli
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	movq	$\no, %rdi
	jmp	isr_common
.endm

.macro isr_error no
.global	_isr\no
.type	_isr\no, @function
_isr\no:
	cli
	pushq	%rsi
	pushq	%rdx
	movq	0x10(%rsp), %rsi
	movq	%rdi, 0x10(%rsp)
	movq	$\no, %rdi
	jmp	isr_common
.endm

isr_noerror	0
isr_noerror	1
isr_noerror	2
isr_noerror	3
isr_noerror	4
isr_noerror	5
isr_noerror	6
isr_noerror	7
isr_error	8
isr_noerror	9
isr_error	10
isr_error	11
isr_error	12
isr_error	13
isr_error	14
isr_noerror	15
isr_noerror	16
isr_error	17
isr_noerror	18
isr_noerror	19
isr_noerror	20
isr_noerror	21
isr_noerror	22
isr_noerror	23
isr_noerror	24
isr_noerror	25
isr_noerror	26
isr_noerror	27
isr_noerror	28
isr_noerror	29
isr_noerror	30
isr_noerror	31

isr_common:
	movq	0x20(%rsp), %rdx
	cmpq	$0x08, %rdx
	je	_noswapgs_1
	swapgs

_noswapgs_1:
	movq	0x18(%rsp), %rdx
	pushq	%rax
	pushq	%rcx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11


	call	isr_handler

	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rdx
	popq	%rcx
	popq	%rax

	popq	%rdx
	popq	%rsi

	movq	0x10(%rsp), %rdi
	cmpq	$0x08, %rdi
	je	_noswapgs_2
	swapgs

_noswapgs_2:
	popq	%rdi
	sti
	iretq

.macro irq no
.global	_irq\no
.type	_irq\no, @function
_irq\no:
	cli
	pushq	%rdi
	movq	$\no, %rdi
	jmp	irq_common
.endm

irq 32
irq 33
irq 34
irq 35
irq 36
irq 37
irq 38
irq 39
irq 40
irq 41
irq 42
irq 43
irq 44
irq 45
irq 46
irq 47
irq 48
irq 49
irq 50
irq 51
irq 52
irq 53
irq 54
irq 55
irq 56
irq 57
irq 58
irq 59
irq 60
irq 61
irq 62
irq 63
irq 64
irq 65
irq 66
irq 67
irq 68
irq 69
irq 70
irq 71
irq 72
irq 73
irq 74
irq 75
irq 76
irq 77
irq 78
irq 79
irq 80
irq 81
irq 82
irq 83
irq 84
irq 85
irq 86
irq 87
irq 88
irq 89
irq 90
irq 91
irq 92
irq 93
irq 94
irq 95
irq 96
irq 97
irq 98
irq 99
irq 100
irq 101
irq 102
irq 103
irq 104
irq 105
irq 106
irq 107
irq 108
irq 109
irq 110
irq 111
irq 112
irq 113
irq 114
irq 115
irq 116
irq 117
irq 118
irq 119
irq 120
irq 121
irq 122
irq 123
irq 124
irq 125
irq 126
irq 127
irq 128
irq 129
irq 130
irq 131
irq 132
irq 133
irq 134
irq 135
irq 136
irq 137
irq 138
irq 139
irq 140
irq 141
irq 142
irq 143
irq 144
irq 145
irq 146
irq 147
irq 148
irq 149
irq 150
irq 151
irq 152
irq 153
irq 154
irq 155
irq 156
irq 157
irq 158
irq 159
irq 160
irq 161
irq 162
irq 163
irq 164
irq 165
irq 166
irq 167
irq 168
irq 169
irq 170
irq 171
irq 172
irq 173
irq 174
irq 175
irq 176
irq 177
irq 178
irq 179
irq 180
irq 181
irq 182
irq 183
irq 184
irq 185
irq 186
irq 187
irq 188
irq 189
irq 190
irq 191
irq 192
irq 193
irq 194
irq 195
irq 196
irq 197
irq 198
irq 199
irq 200
irq 201
irq 202
irq 203
irq 204
irq 205
irq 206
irq 207
irq 208
irq 209
irq 210
irq 211
irq 212
irq 213
irq 214
irq 215
irq 216
irq 217
irq 218
irq 219
irq 220
irq 221
irq 222
irq 223
irq 224
irq 225
irq 226
irq 227
irq 228
irq 229
irq 230
irq 231
irq 232
irq 233
irq 234
irq 235
irq 236
irq 237
irq 238
irq 239
irq 240
irq 241
irq 242
irq 243
irq 244
irq 245
irq 246
irq 247
irq 248
irq 249
irq 250
irq 251
irq 252
irq 253
irq 254
irq 255

irq_common:
	pushq	%rsi

	movq	0x18(%rsp), %rsi
	cmpq	$0x08, %rsi
	jne	_noswapgs_3
	swapgs

_noswapgs_3:
	pushq	%rax
	pushq	%rcx
	pushq	%rdx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11

	call	irq_handler

	movq	0x50(%rsp), %rdi
	cmpq	$0x08, %rdi
	jne	_usermode_ret

_kernel_ret:
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rdx
	popq	%rcx
	popq	%rax
	popq	%rsi
	popq	%rdi

	iretq

_usermode_ret:
	call	__process_on_user_iret

	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rdx
	popq	%rcx
	popq	%rax
	popq	%rsi
	popq	%rdi

	swapgs
	iretq
