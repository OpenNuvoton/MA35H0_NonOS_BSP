;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* Copyright(c) 2017 Nuvoton Technology Corp. All rights reserved.                                         */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/
;// .syntax unified

	.globl  VectorDataBase
	.globl  VectorDataLimit

	.align  4

	.text

VectorDataBase:
	.incbin "../../sha_test_vectors.rsp"
VectorDataLimit:
	.space   4

	.end

