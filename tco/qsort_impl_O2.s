	.file	"qsort_impl.c"
# GNU C17 (Ubuntu 13.3.0-6ubuntu2~24.04.1) version 13.3.0 (x86_64-linux-gnu)
#	compiled by GNU C version 13.3.0, GMP version 6.3.0, MPFR version 4.2.1, MPC version 1.3.1, isl version isl-0.26-GMP

# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed: -mtune=generic -march=x86-64 -O2 -fasynchronous-unwind-tables -fstack-protector-strong -fstack-clash-protection -fcf-protection
	.text
	.p2align 4
	.globl	qsort_original
	.type	qsort_original, @function
qsort_original:
.LFB35:
	.cfi_startproc
	endbr64	
	pushq	%r12	#
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp	#
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx	#
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	subq	$32, %rsp	#,
	.cfi_def_cfa_offset 64
# qsort_impl.c:16:     if (!*list)
	movq	(%rdi), %rbx	# *list_11(D), _1
# qsort_impl.c:15: {
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp108
	movq	%rax, 24(%rsp)	# tmp108, D.3981
	xorl	%eax, %eax	# tmp108
# qsort_impl.c:16:     if (!*list)
	testq	%rbx, %rbx	# _1
	je	.L1	#,
# qsort_impl.c:21:     node_t *p = pivot->next;
	movq	8(%rbx), %rax	# _1->next, p
	movq	%rdi, %r12	# tmp107, list
	leaq	8(%rsp), %rbp	#, tmp103
# qsort_impl.c:20:     int value = pivot->value;
	movl	(%rbx), %edi	# _1->value, value
# qsort_impl.c:22:     pivot->next = NULL;
	movq	$0, 8(%rbx)	#, _1->next
	movq	%rsp, %r8	#, tmp104
# qsort_impl.c:24:     node_t *left = NULL, *right = NULL;
	movq	$0, (%rsp)	#, left
# qsort_impl.c:24:     node_t *left = NULL, *right = NULL;
	movq	$0, 8(%rsp)	#, right
# qsort_impl.c:25:     while (p) {
	testq	%rax, %rax	# p
	jne	.L6	#,
	jmp	.L3	#
	.p2align 4,,10
	.p2align 3
.L25:
# qsort_impl.c:28:         list_add(n->value > value ? &right : &left, n);
	movq	8(%rsp), %rsi	# right, _8
	movq	%rbp, %rcx	# tmp103, iftmp.0_49
# qsort.h:20:     n->next = *list;
	movq	%rsi, 8(%rdx)	# _8, p_52->next
# qsort.h:21:     *list = n;
	movq	%rdx, (%rcx)	# p, *iftmp.0_49
# qsort_impl.c:25:     while (p) {
	testq	%rax, %rax	# p
	je	.L3	#,
.L6:
	movq	%rax, %rdx	# p, p
# qsort_impl.c:27:         p = p->next;
	movq	8(%rax), %rax	# p_27->next, p
# qsort_impl.c:28:         list_add(n->value > value ? &right : &left, n);
	cmpl	(%rdx), %edi	# p_52->value, value
	jl	.L25	#,
# qsort_impl.c:28:         list_add(n->value > value ? &right : &left, n);
	movq	(%rsp), %rsi	# left, _8
	movq	%r8, %rcx	# tmp104, iftmp.0_49
# qsort.h:20:     n->next = *list;
	movq	%rsi, 8(%rdx)	# _8, p_52->next
# qsort.h:21:     *list = n;
	movq	%rdx, (%rcx)	# p, *iftmp.0_49
# qsort_impl.c:25:     while (p) {
	testq	%rax, %rax	# p
	jne	.L6	#,
.L3:
# qsort_impl.c:31:     qsort_original(&left);
	movq	%r8, %rdi	# tmp104,
	call	qsort_original	#
# qsort_impl.c:32:     qsort_original(&right);
	movq	%rbp, %rdi	# tmp103,
	call	qsort_original	#
# qsort_impl.c:35:     list_concat(&result, left);
	movq	(%rsp), %rax	# left, _42
# qsort.h:28:     *left = right;
	movq	%rax, 16(%rsp)	# _42, result
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _42
	je	.L12	#,
	.p2align 4,,10
	.p2align 3
.L8:
	movq	%rax, %rdx	# _42, _25
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_55 + 8B], _42
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _42
	jne	.L8	#,
# qsort.h:27:         left = &(*left)->next;
	addq	$8, %rdx	#, left
.L7:
# qsort.h:28:     *left = right;
	movq	%rbx, (%rdx)	# _1, *left_56
	movq	16(%rsp), %rax	# result, _46
# qsort_impl.c:37:     list_concat(&result, right);
	movq	8(%rsp), %rcx	# right, right.2_4
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _46
	je	.L13	#,
	.p2align 4,,10
	.p2align 3
.L10:
	movq	%rax, %rdx	# _46, _22
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_53 + 8B], _46
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _46
	jne	.L10	#,
# qsort.h:27:         left = &(*left)->next;
	addq	$8, %rdx	#, left
.L9:
# qsort.h:28:     *left = right;
	movq	%rcx, (%rdx)	# right.2_4, *left_54
# qsort_impl.c:38:     *list = result;
	movq	16(%rsp), %rax	# result, result
	movq	%rax, (%r12)	# result, *list_11(D)
.L1:
# qsort_impl.c:39: }
	movq	24(%rsp), %rax	# D.3981, tmp109
	subq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp109
	jne	.L26	#,
	addq	$32, %rsp	#,
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	popq	%rbx	#
	.cfi_def_cfa_offset 24
	popq	%rbp	#
	.cfi_def_cfa_offset 16
	popq	%r12	#
	.cfi_def_cfa_offset 8
	ret	
	.p2align 4,,10
	.p2align 3
.L12:
	.cfi_restore_state
# qsort.h:26:     while (*left)
	leaq	16(%rsp), %rdx	#, left
	jmp	.L7	#
	.p2align 4,,10
	.p2align 3
.L13:
	leaq	16(%rsp), %rdx	#, left
	jmp	.L9	#
.L26:
# qsort_impl.c:39: }
	call	__stack_chk_fail@PLT	#
	.cfi_endproc
.LFE35:
	.size	qsort_original, .-qsort_original
	.p2align 4
	.globl	qsort_tco
	.type	qsort_tco, @function
qsort_tco:
.LFB36:
	.cfi_startproc
	endbr64	
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushq	%rbx	#
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$40, %rsp	#,
	.cfi_def_cfa_offset 64
# qsort_impl.c:52:     if (!*list)
	movq	(%rdi), %rbx	# *list_18(D), prephitmp_43
# qsort_impl.c:51: {
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp114
	movq	%rax, 24(%rsp)	# tmp114, D.3992
	xorl	%eax, %eax	# tmp114
# qsort_impl.c:52:     if (!*list)
	testq	%rbx, %rbx	# prephitmp_43
	je	.L27	#,
# qsort_impl.c:57:     node_t *p = pivot->next;
	movq	8(%rbx), %rax	# _1->next, p
	movq	%rdi, %rbp	# tmp113, list
# qsort_impl.c:56:     int value = pivot->value;
	movl	(%rbx), %edi	# _1->value, value
# qsort_impl.c:58:     pivot->next = NULL;
	movq	$0, 8(%rbx)	#, _1->next
# qsort_impl.c:60:     node_t *left = NULL, *right = NULL;
	movq	$0, (%rsp)	#, left
# qsort_impl.c:60:     node_t *left = NULL, *right = NULL;
	movq	$0, 8(%rsp)	#, right
# qsort_impl.c:61:     while (p) {
	testq	%rax, %rax	# p
	je	.L35	#,
# qsort_impl.c:64:         list_add(n->value > value ? &right : &left, n);
	movq	%rsp, %r9	#, iftmp.4_41
# qsort_impl.c:64:         list_add(n->value > value ? &right : &left, n);
	leaq	8(%rsp), %r8	#, iftmp.4_41
	jmp	.L32	#
	.p2align 4,,10
	.p2align 3
.L63:
	movq	8(%rsp), %rsi	# right, _19
	movq	%r8, %rcx	# iftmp.4_41, iftmp.4_41
# qsort.h:20:     n->next = *list;
	movq	%rsi, 8(%rdx)	# _19, p_78->next
# qsort.h:21:     *list = n;
	movq	%rdx, (%rcx)	# p, *iftmp.4_41
# qsort_impl.c:61:     while (p) {
	testq	%rax, %rax	# p
	je	.L62	#,
.L32:
	movq	%rax, %rdx	# p, p
# qsort_impl.c:63:         p = p->next;
	movq	8(%rax), %rax	# p_45->next, p
# qsort_impl.c:64:         list_add(n->value > value ? &right : &left, n);
	cmpl	(%rdx), %edi	# p_78->value, value
	jl	.L63	#,
# qsort_impl.c:64:         list_add(n->value > value ? &right : &left, n);
	movq	(%rsp), %rsi	# left, _19
	movq	%r9, %rcx	# iftmp.4_41, iftmp.4_41
# qsort.h:20:     n->next = *list;
	movq	%rsi, 8(%rdx)	# _19, p_78->next
# qsort.h:21:     *list = n;
	movq	%rdx, (%rcx)	# p, *iftmp.4_41
# qsort_impl.c:61:     while (p) {
	testq	%rax, %rax	# p
	jne	.L32	#,
.L62:
# qsort_impl.c:68:     if (!left && right) {
	cmpq	$0, (%rsp)	#, left
# qsort_impl.c:68:     if (!left && right) {
	movq	8(%rsp), %rax	# right, pretmp_81
# qsort_impl.c:67:     node_t *result = NULL;
	movq	$0, 16(%rsp)	#, result
# qsort_impl.c:68:     if (!left && right) {
	je	.L64	#,
# qsort_impl.c:73:         qsort_tco(&left);
	movq	%rsp, %rdi	#, tmp107
# qsort_impl.c:72:     } else if (left && right) {
	testq	%rax, %rax	# pretmp_81
	je	.L36	#,
# qsort_impl.c:73:         qsort_tco(&left);
	call	qsort_tco	#
# qsort_impl.c:74:         list_concat(&result, left);
	movq	(%rsp), %rax	# left, _69
# qsort.h:28:     *left = right;
	movq	%rax, 16(%rsp)	# _69, result
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _69
	je	.L44	#,
	.p2align 4,,10
	.p2align 3
.L38:
	movq	%rax, %rdx	# _69, _35
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_84 + 8B], _69
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _69
	jne	.L38	#,
# qsort.h:27:         left = &(*left)->next;
	addq	$8, %rdx	#, left
.L37:
# qsort.h:28:     *left = right;
	movq	%rbx, (%rdx)	# prephitmp_43, *left_85
# qsort_impl.c:76:         qsort_tco(&right);
	leaq	8(%rsp), %rdi	#, tmp108
	call	qsort_tco	#
	movq	16(%rsp), %rax	# result, _73
# qsort_impl.c:77:         list_concat(&result, right);
	movq	8(%rsp), %rcx	# right, right.11_8
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _73
	je	.L45	#,
	.p2align 4,,10
	.p2align 3
.L40:
	movq	%rax, %rdx	# _73, _79
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_82 + 8B], _73
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _73
	jne	.L40	#,
# qsort.h:27:         left = &(*left)->next;
	addq	$8, %rdx	#, left
.L39:
# qsort.h:28:     *left = right;
	movq	%rcx, (%rdx)	# right.11_8, *left_83
# qsort_impl.c:85:     *list = result;
	movq	16(%rsp), %rbx	# result, prephitmp_43
.L35:
	movq	%rbx, 0(%rbp)	# prephitmp_43, *list_18(D)
.L27:
# qsort_impl.c:86: }
	movq	24(%rsp), %rax	# D.3992, tmp115
	subq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp115
	jne	.L65	#,
	addq	$40, %rsp	#,
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbx	#
	.cfi_def_cfa_offset 16
	popq	%rbp	#
	.cfi_def_cfa_offset 8
	ret	
	.p2align 4,,10
	.p2align 3
.L36:
	.cfi_restore_state
# qsort_impl.c:79:         qsort_tco(&left);
	call	qsort_tco	#
# qsort_impl.c:80:         list_concat(&result, left);
	movq	(%rsp), %rax	# left, _71
# qsort.h:28:     *left = right;
	movq	%rax, 16(%rsp)	# _71, result
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _71
	je	.L46	#,
.L42:
	movq	%rax, %rdx	# _71, _63
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_86 + 8B], _71
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _71
	jne	.L42	#,
# qsort.h:27:         left = &(*left)->next;
	addq	$8, %rdx	#, left
.L41:
# qsort.h:28:     *left = right;
	movq	%rbx, (%rdx)	# prephitmp_43, *left_87
# qsort_impl.c:85:     *list = result;
	movq	16(%rsp), %rbx	# result, prephitmp_43
# qsort.h:29: }
	jmp	.L35	#
	.p2align 4,,10
	.p2align 3
.L64:
# qsort_impl.c:68:     if (!left && right) {
	testq	%rax, %rax	# pretmp_81
	je	.L35	#,
# qsort_impl.c:70:         qsort_tco(&right);
	leaq	8(%rsp), %rdi	#, tmp106
	call	qsort_tco	#
# qsort_impl.c:71:         list_concat(&result, right);
	movq	8(%rsp), %rcx	# right, right.7_5
	movq	%rbx, %rax	# prephitmp_43, _67
	.p2align 4,,10
	.p2align 3
.L34:
	movq	%rax, %rdx	# _67, _56
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_80 + 8B], _67
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _67
	jne	.L34	#,
# qsort.h:28:     *left = right;
	movq	%rcx, 8(%rdx)	# right.7_5, MEM[(struct node_t * *)_56 + 8B]
# qsort_impl.c:85:     *list = result;
	movq	%rbx, 0(%rbp)	# prephitmp_43, *list_18(D)
	jmp	.L27	#
.L46:
# qsort.h:26:     while (*left)
	leaq	16(%rsp), %rdx	#, left
	jmp	.L41	#
.L45:
	leaq	16(%rsp), %rdx	#, left
	jmp	.L39	#
.L44:
	leaq	16(%rsp), %rdx	#, left
	jmp	.L37	#
.L65:
# qsort_impl.c:86: }
	call	__stack_chk_fail@PLT	#
	.cfi_endproc
.LFE36:
	.size	qsort_tco, .-qsort_tco
	.p2align 4
	.globl	qsort_method2
	.type	qsort_method2, @function
qsort_method2:
.LFB37:
	.cfi_startproc
	endbr64	
	pushq	%r14	#
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	pushq	%r13	#
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
# qsort_impl.c:102:     node_t *suffix = NULL;
	xorl	%r13d, %r13d	# suffix
# qsort_impl.c:101: {
	pushq	%r12	#
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	movq	%rdi, %r12	# tmp119, list
	pushq	%rbp	#
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	pushq	%rbx	#
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	subq	$32, %rsp	#,
	.cfi_def_cfa_offset 80
# qsort_impl.c:104:     while (*list) {
	movq	(%rdi), %r14	# *list_26(D), _81
# qsort_impl.c:101: {
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp122
	movq	%rax, 24(%rsp)	# tmp122, D.4007
	xorl	%eax, %eax	# tmp122
# qsort_impl.c:104:     while (*list) {
	testq	%r14, %r14	# _81
	je	.L67	#,
# qsort_impl.c:126:             qsort_method2(&left);
	movq	%rsp, %rbp	#, tmp115
.L82:
# qsort_impl.c:107:         node_t *p = pivot->next;
	movq	8(%r14), %rbx	# _81->next, p
# qsort_impl.c:106:         int value = pivot->value;
	movl	(%r14), %r9d	# _81->value, value
# qsort_impl.c:108:         pivot->next = NULL;
	movq	$0, 8(%r14)	#, _81->next
# qsort_impl.c:110:         node_t *left = NULL, *right = NULL;
	movq	$0, (%rsp)	#, left
# qsort_impl.c:110:         node_t *left = NULL, *right = NULL;
	movq	$0, 8(%rsp)	#, right
# qsort_impl.c:112:         while (p) {
	testq	%rbx, %rbx	# p
	je	.L68	#,
	xorl	%r10d, %r10d	# left_lsm_flag.67
	xorl	%edi, %edi	# left_lsm.66
	xorl	%r11d, %r11d	# right_lsm_flag.65
	xorl	%r8d, %r8d	# right_lsm.64
# qsort_impl.c:111:         int lc = 0, rc = 0;
	xorl	%ecx, %ecx	# rc
# qsort_impl.c:111:         int lc = 0, rc = 0;
	xorl	%esi, %esi	# lc
	jmp	.L71	#
	.p2align 4,,10
	.p2align 3
.L105:
# qsort.h:20:     n->next = *list;
	movq	%r8, %rdx	# right_lsm.64, cstore_104
# qsort_impl.c:117:                 rc++;
	addl	$1, %ecx	#, rc
# qsort.h:21:     *list = n;
	movq	%rax, %r8	# p, right_lsm.64
# qsort_impl.c:117:                 rc++;
	movl	$1, %r11d	#, right_lsm_flag.65
	movq	%rdx, 8(%rax)	# cstore_104, p_25->next
# qsort_impl.c:112:         while (p) {
	testq	%rbx, %rbx	# p
	je	.L104	#,
.L71:
	movq	%rbx, %rax	# p, p
# qsort_impl.c:114:             p = p->next;
	movq	8(%rbx), %rbx	# p_51->next, p
# qsort_impl.c:115:             if (n->value > value) {
	cmpl	%r9d, (%rax)	# value, p_25->value
	jg	.L105	#,
# qsort.h:20:     n->next = *list;
	movq	%rdi, %rdx	# left_lsm.66, cstore_104
# qsort_impl.c:120:                 lc++;
	addl	$1, %esi	#, lc
# qsort.h:21:     *list = n;
	movq	%rax, %rdi	# p, left_lsm.66
# qsort_impl.c:120:                 lc++;
	movl	$1, %r10d	#, left_lsm_flag.67
	movq	%rdx, 8(%rax)	# cstore_104, p_25->next
# qsort_impl.c:112:         while (p) {
	testq	%rbx, %rbx	# p
	jne	.L71	#,
.L104:
	testb	%r10b, %r10b	# left_lsm_flag.67
	je	.L72	#,
	movq	%rdi, (%rsp)	# left_lsm.66, left
.L72:
	testb	%r11b, %r11b	# right_lsm_flag.65
	je	.L73	#,
	movq	%r8, 8(%rsp)	# right_lsm.64, right
.L73:
# qsort_impl.c:124:         if (lc <= rc) {
	cmpl	%ecx, %esi	# rc, lc
	jle	.L106	#,
# qsort_impl.c:140:             qsort_method2(&right);
	leaq	8(%rsp), %rdi	#, tmp108
	call	qsort_method2	#
# qsort_impl.c:143:             pivot->next = right;
	movq	8(%rsp), %rax	# right, prephitmp_55
	movq	%rax, 8(%r14)	# prephitmp_55, _81->next
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# prephitmp_55
	je	.L87	#,
	.p2align 4,,10
	.p2align 3
.L81:
	movq	%rax, %rdx	# prephitmp_55, _23
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)prephitmp_55 + 8B], prephitmp_55
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# prephitmp_55
	jne	.L81	#,
.L80:
# qsort_impl.c:148:             *list = left;
	movq	(%rsp), %rax	# left, left.25_13
# qsort.h:28:     *left = right;
	movq	%r13, 8(%rdx)	# suffix, MEM[(struct node_t * *)_23 + 8B]
# qsort_impl.c:145:             suffix = pivot;
	movq	%r14, %r13	# _81, suffix
# qsort_impl.c:148:             *list = left;
	movq	%rax, (%r12)	# left.25_13, *list_82
	movq	%rax, %r14	# left.25_13, _81
.L79:
# qsort_impl.c:104:     while (*list) {
	testq	%r14, %r14	# _81
	jne	.L82	#,
.L67:
# qsort_impl.c:153:     *list = suffix;
	movq	%r13, (%r12)	# suffix, *list_83
# qsort_impl.c:154: }
	movq	24(%rsp), %rax	# D.4007, tmp123
	subq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp123
	jne	.L107	#,
	addq	$32, %rsp	#,
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	popq	%rbx	#
	.cfi_def_cfa_offset 40
	popq	%rbp	#
	.cfi_def_cfa_offset 32
	popq	%r12	#
	.cfi_def_cfa_offset 24
	popq	%r13	#
	.cfi_def_cfa_offset 16
	popq	%r14	#
	.cfi_def_cfa_offset 8
	ret	
	.p2align 4,,10
	.p2align 3
.L106:
	.cfi_restore_state
# qsort_impl.c:134:             *tail = right;
	movq	8(%rsp), %rbx	# right, p
.L68:
# qsort_impl.c:126:             qsort_method2(&left);
	movq	%rbp, %rdi	# tmp115,
	call	qsort_method2	#
# qsort_impl.c:129:             list_concat(&result, left);
	movq	(%rsp), %rax	# left, _40
# qsort.h:28:     *left = right;
	movq	%rax, 16(%rsp)	# _40, result
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _40
	je	.L85	#,
	.p2align 4,,10
	.p2align 3
.L76:
	movq	%rax, %rdx	# _40, _52
# qsort.h:27:         left = &(*left)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_96 + 8B], _40
# qsort.h:26:     while (*left)
	testq	%rax, %rax	# _40
	jne	.L76	#,
# qsort.h:27:         left = &(*left)->next;
	addq	$8, %rdx	#, left
.L75:
# qsort.h:28:     *left = right;
	movq	%r14, (%rdx)	# _81, *left_97
# qsort_impl.c:131:             node_t **tail = &result;
	movq	16(%rsp), %rax	# result, _71
# qsort_impl.c:132:             while (*tail)
	testq	%rax, %rax	# _71
	je	.L86	#,
	.p2align 4,,10
	.p2align 3
.L78:
	movq	%rax, %rdx	# _71, _12
# qsort_impl.c:133:                 tail = &(*tail)->next;
	movq	8(%rax), %rax	# MEM[(struct node_t * *)_80 + 8B], _71
# qsort_impl.c:132:             while (*tail)
	testq	%rax, %rax	# _71
	jne	.L78	#,
# qsort_impl.c:133:                 tail = &(*tail)->next;
	addq	$8, %rdx	#, tail
.L77:
# qsort_impl.c:134:             *tail = right;
	movq	%rbx, (%rdx)	# p, *tail_91
# qsort_impl.c:136:             *list = result;
	movq	16(%rsp), %rax	# result, result.22_10
	movq	%rax, (%r12)	# result.22_10, *list_82
# qsort_impl.c:137:             list = tail;
	movq	%rdx, %r12	# tail, list
# qsort_impl.c:104:     while (*list) {
	movq	(%rdx), %r14	# *tail_91, _81
	jmp	.L79	#
.L86:
# qsort_impl.c:131:             node_t **tail = &result;
	leaq	16(%rsp), %rdx	#, tail
	jmp	.L77	#
.L87:
# qsort.h:26:     while (*left)
	movq	%r14, %rdx	# _81, _23
	jmp	.L80	#
.L85:
	leaq	16(%rsp), %rdx	#, left
	jmp	.L75	#
.L107:
# qsort_impl.c:154: }
	call	__stack_chk_fail@PLT	#
	.cfi_endproc
.LFE37:
	.size	qsort_method2, .-qsort_method2
	.p2align 4
	.globl	qsort_iterative
	.type	qsort_iterative, @function
qsort_iterative:
.LFB38:
	.cfi_startproc
	endbr64	
	pushq	%r12	#
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp	#
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx	#
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	subq	$2096, %rsp	#,
	.cfi_def_cfa_offset 2128
# qsort_impl.c:170:     if (!*list)
	movq	(%rdi), %r8	# *list_21(D), prephitmp_32
# qsort_impl.c:169: {
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp114
	movq	%rax, 2088(%rsp)	# tmp114, D.4014
	xorl	%eax, %eax	# tmp114
# qsort_impl.c:170:     if (!*list)
	testq	%r8, %r8	# prephitmp_32
	je	.L108	#,
# qsort_impl.c:184:         if (!sub->next) {
	movq	8(%r8), %rax	# prephitmp_32->next, p
# qsort_impl.c:175:     stack[top++] = *list;
	movq	%r8, 32(%rsp)	# prephitmp_32, stack[0]
	movq	%rdi, %rbp	# tmp113, list
	xorl	%r9d, %r9d	# top
# qsort_impl.c:178:     node_t **rtail = &result;
	leaq	8(%rsp), %r12	#, rtail
# qsort_impl.c:175:     stack[top++] = *list;
	movl	$1, %r10d	#, top
# qsort_impl.c:201:             list_add(n->value > value ? &right : &left, n);
	leaq	16(%rsp), %rbx	#, iftmp.27_56
# qsort_impl.c:177:     node_t *result = NULL;
	movq	$0, 8(%rsp)	#, result
# qsort_impl.c:201:             list_add(n->value > value ? &right : &left, n);
	leaq	24(%rsp), %r11	#, iftmp.27_56
# qsort_impl.c:184:         if (!sub->next) {
	testq	%rax, %rax	# p
	je	.L127	#,
	.p2align 4,,10
	.p2align 3
.L110:
# qsort_impl.c:195:         pivot->next = NULL;
	movq	$0, 8(%r8)	#, prephitmp_32->next
# qsort_impl.c:193:         int value = pivot->value;
	movl	(%r8), %edi	# prephitmp_32->value, value
# qsort_impl.c:197:         node_t *left = NULL, *right = NULL;
	movq	$0, 16(%rsp)	#, left
# qsort_impl.c:197:         node_t *left = NULL, *right = NULL;
	movq	$0, 24(%rsp)	#, right
	jmp	.L115	#
	.p2align 4,,10
	.p2align 3
.L129:
# qsort_impl.c:201:             list_add(n->value > value ? &right : &left, n);
	movq	24(%rsp), %rsi	# right, _54
	movq	%r11, %rcx	# iftmp.27_56, iftmp.27_56
# qsort.h:20:     n->next = *list;
	movq	%rsi, 8(%rdx)	# _54, p_47->next
# qsort.h:21:     *list = n;
	movq	%rdx, (%rcx)	# p, *iftmp.27_56
# qsort_impl.c:198:         while (p) {
	testq	%rax, %rax	# p
	je	.L128	#,
.L115:
	movq	%rax, %rdx	# p, p
# qsort_impl.c:200:             p = p->next;
	movq	8(%rax), %rax	# p_25->next, p
# qsort_impl.c:201:             list_add(n->value > value ? &right : &left, n);
	cmpl	%edi, (%rdx)	# value, p_47->value
	jg	.L129	#,
# qsort_impl.c:201:             list_add(n->value > value ? &right : &left, n);
	movq	16(%rsp), %rsi	# left, _54
	movq	%rbx, %rcx	# iftmp.27_56, iftmp.27_56
# qsort.h:20:     n->next = *list;
	movq	%rsi, 8(%rdx)	# _54, p_47->next
# qsort.h:21:     *list = n;
	movq	%rdx, (%rcx)	# p, *iftmp.27_56
# qsort_impl.c:198:         while (p) {
	testq	%rax, %rax	# p
	jne	.L115	#,
.L128:
# qsort_impl.c:206:         if (right)
	movq	24(%rsp), %rax	# right, right.28_5
# qsort_impl.c:206:         if (right)
	testq	%rax, %rax	# right.28_5
	je	.L120	#,
# qsort_impl.c:207:             stack[top++] = right;
	movslq	%r9d, %r9	# top, top
	movq	%rax, 32(%rsp,%r9,8)	# right.28_5, stack[top_27]
# qsort_impl.c:208:         stack[top++] = pivot;
	leal	1(%r10), %eax	#, prephitmp_29
.L116:
# qsort_impl.c:208:         stack[top++] = pivot;
	movslq	%r10d, %rdx	# top, top
	movq	%r8, 32(%rsp,%rdx,8)	# prephitmp_32, stack[top_9]
# qsort_impl.c:209:         if (left)
	movq	16(%rsp), %rdx	# left, left.32_7
# qsort_impl.c:209:         if (left)
	testq	%rdx, %rdx	# left.32_7
	je	.L121	#,
# qsort_impl.c:210:             stack[top++] = left;
	cltq
# qsort_impl.c:210:             stack[top++] = left;
	addl	$2, %r10d	#, top
# qsort_impl.c:210:             stack[top++] = left;
	movq	%rdx, 32(%rsp,%rax,8)	# left.32_7, stack[prephitmp_29]
.L112:
# qsort_impl.c:181:         node_t *sub = stack[--top];
	leal	-1(%r10), %r9d	#, top
	movslq	%r9d, %rax	# top, top
	movq	32(%rsp,%rax,8), %r8	# stack[_63], prephitmp_32
# qsort_impl.c:184:         if (!sub->next) {
	movq	8(%r8), %rax	# prephitmp_32->next, p
# qsort_impl.c:184:         if (!sub->next) {
	testq	%rax, %rax	# p
	jne	.L110	#,
.L127:
# qsort_impl.c:186:             *rtail = sub;
	movq	%r8, (%r12)	# prephitmp_32, *rtail_60
# qsort_impl.c:180:     while (top > 0) {
	testl	%r9d, %r9d	# top
	je	.L111	#,
# qsort_impl.c:187:             rtail = &sub->next;
	leaq	8(%r8), %r12	#, rtail
	movl	%r9d, %r10d	# top, top
	jmp	.L112	#
	.p2align 4,,10
	.p2align 3
.L121:
# qsort_impl.c:208:         stack[top++] = pivot;
	movl	%eax, %r10d	# prephitmp_29, top
	jmp	.L112	#
	.p2align 4,,10
	.p2align 3
.L120:
	movl	%r10d, %eax	# top, prephitmp_29
# qsort_impl.c:181:         node_t *sub = stack[--top];
	movl	%r9d, %r10d	# top, top
	jmp	.L116	#
	.p2align 4,,10
	.p2align 3
.L111:
# qsort_impl.c:213:     *list = result;
	movq	8(%rsp), %rax	# result, result
	movq	%rax, 0(%rbp)	# result, *list_21(D)
.L108:
# qsort_impl.c:214: }
	movq	2088(%rsp), %rax	# D.4014, tmp115
	subq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp115
	jne	.L130	#,
	addq	$2096, %rsp	#,
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	popq	%rbx	#
	.cfi_def_cfa_offset 24
	popq	%rbp	#
	.cfi_def_cfa_offset 16
	popq	%r12	#
	.cfi_def_cfa_offset 8
	ret	
.L130:
	.cfi_restore_state
	call	__stack_chk_fail@PLT	#
	.cfi_endproc
.LFE38:
	.size	qsort_iterative, .-qsort_iterative
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
