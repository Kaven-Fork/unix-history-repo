/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.proprietary.c%
 *
 *	@(#)instrs.adb	5.1 (Berkeley) %G%
 */

OP("adda",0x8e,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("addb2",0x08,2,ACCR+TYPB,ACCM+TYPB,0,0,0,0),
OP("addb3",0x18,3,ACCR+TYPB,ACCR+TYPB,ACCW+TYPB,0,0,0),
OP("addd",0xc7,1,ACCR+TYPD,0,0,0,0,0),
OP("addf",0xc6,1,ACCR+TYPF,0,0,0,0,0),
OP("addl2",0x0c,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("addl3",0x1c,3,ACCR+TYPL,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("addw2",0x0a,2,ACCR+TYPW,ACCM+TYPW,0,0,0,0),
OP("addw3",0x1a,3,ACCR+TYPW,ACCR+TYPW,ACCW+TYPW,0,0,0),
OP("adwc",0x8d,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("andb2",0xa8,2,ACCR+TYPB,ACCM+TYPB,0,0,0,0),
OP("andb3",0xb8,3,ACCR+TYPB,ACCR+TYPB,ACCW+TYPB,0,0,0),
OP("andl2",0xac,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("andl3",0xbc,3,ACCR+TYPL,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("andw2",0xaa,2,ACCR+TYPW,ACCM+TYPW,0,0,0,0),
OP("andw3",0xba,3,ACCR+TYPW,ACCR+TYPW,ACCW+TYPW,0,0,0),
OP("aobleq",0x3f,3,ACCR+TYPL,ACCM+TYPL,ACCB+TYPW,0,0,0),
OP("aoblss",0x2f,3,ACCR+TYPL,ACCM+TYPL,ACCB+TYPW,0,0,0),
OP("atanf",0x25,0,0,0,0,0,0,0),
OP("bbc",0x1e,3,ACCR+TYPL,ACCR+TYPL,ACCB+TYPW,0,0,0),
OP("bbs",0x0e,3,ACCR+TYPL,ACCR+TYPL,ACCB+TYPW,0,0,0),
OP("bbssi",0x5f,3,ACCR+TYPL,ACCM+TYPL,ACCB+TYPW,0,0,0),
OP("bcc",0xf1,1,ACCB+TYPB,0,0,0,0,0),
OP("bcs",0xe1,1,ACCB+TYPB,0,0,0,0,0),
OP("beql",0x31,1,ACCB+TYPB,0,0,0,0,0),
OP("beqlu",0x31,1,ACCB+TYPB,0,0,0,0,0),
OP("bgeq",0x81,1,ACCB+TYPB,0,0,0,0,0),
OP("bgequ",0xe1,1,ACCB+TYPB,0,0,0,0,0),
OP("bgtr",0x41,1,ACCB+TYPB,0,0,0,0,0),
OP("bgtru",0xa1,1,ACCB+TYPB,0,0,0,0,0),
OP("bicpsw",0x9b,1,ACCR+TYPW,0,0,0,0,0),
OP("bispsw",0x8b,1,ACCR+TYPW,0,0,0,0,0),
OP("bitb",0x39,2,ACCR+TYPB,ACCR+TYPB,0,0,0,0),
OP("bitl",0x3d,2,ACCR+TYPL,ACCR+TYPL,0,0,0,0),
OP("bitw",0x3b,2,ACCR+TYPW,ACCR+TYPW,0,0,0,0),
OP("bleq",0x51,1,ACCB+TYPB,0,0,0,0,0),
OP("blequ",0xb1,1,ACCB+TYPB,0,0,0,0,0),
OP("blss",0x91,1,ACCB+TYPB,0,0,0,0,0),
OP("blssu",0xf1,1,ACCB+TYPB,0,0,0,0,0),
OP("bneq",0x21,1,ACCB+TYPB,0,0,0,0,0),
OP("bnequ",0x21,1,ACCB+TYPB,0,0,0,0,0),
OP("bpt",0x30,0,0,0,0,0,0,0),
OP("brb",0x11,1,ACCB+TYPB,0,0,0,0,0),
OP("brw",0x13,1,ACCB+TYPW,0,0,0,0,0),
OP("btcs",0xce,1,ACCR+TYPB,0,0,0,0,0),
OP("bvc",0xc1,1,ACCB+TYPB,0,0,0,0,0),
OP("bvs",0xd1,1,ACCB+TYPB,0,0,0,0,0),
OP("callf",0xfe,2,ACCR+TYPB,ACCA+TYPB,0,0,0,0),
OP("calls",0xbf,2,ACCR+TYPB,ACCA+TYPB,0,0,0,0),
OP("casel",0xfc,3,ACCR+TYPL,ACCR+TYPL,ACCR+TYPL,0,0,0),
OP("clrb",0x49,1,ACCW+TYPB,0,0,0,0,0),
OP("clrl",0x4d,1,ACCW+TYPL,0,0,0,0,0),
OP("clrw",0x4b,1,ACCW+TYPW,0,0,0,0,0),
OP("cmpb",0x19,2,ACCR+TYPB,ACCR+TYPB,0,0,0,0),
OP("cmpd",0x37,1,ACCR+TYPD,0,0,0,0,0),
OP("cmpd2",0x47,2,ACCR+TYPD,ACCR+TYPD,0,0,0,0),
OP("cmpf",0x36,1,ACCR+TYPF,0,0,0,0,0),
OP("cmpf2",0x46,2,ACCR+TYPF,ACCR+TYPF,0,0,0,0),
OP("cmpl",0x1d,2,ACCR+TYPL,ACCR+TYPL,0,0,0,0),
OP("cmps2",0x92,0,0,0,0,0,0,0),
OP("cmps3",0xd2,0,0,0,0,0,0,0),
OP("cmpw",0x1b,2,ACCR+TYPW,ACCR+TYPW,0,0,0,0),
OP("cosf",0x15,0,0,0,0,0,0,0),
OP("cvdf",0xa6,0,0,0,0,0,0,0),
OP("cvdl",0x87,1,ACCW+TYPL,0,0,0,0,0),
OP("cvfl",0x86,1,ACCW+TYPL,0,0,0,0,0),
OP("cvld",0x77,1,ACCR+TYPL,0,0,0,0,0),
OP("cvlf",0x76,1,ACCR+TYPL,0,0,0,0,0),
OP("cvtbl",0x89,2,ACCR+TYPB,ACCW+TYPL,0,0,0,0),
OP("cvtbw",0x99,2,ACCR+TYPB,ACCW+TYPW,0,0,0,0),
OP("cvtlb",0x6f,2,ACCR+TYPL,ACCW+TYPB,0,0,0,0),
OP("cvtlw",0x7f,2,ACCR+TYPL,ACCW+TYPW,0,0,0,0),
OP("cvtwb",0x33,2,ACCR+TYPW,ACCW+TYPB,0,0,0,0),
OP("cvtwl",0x23,2,ACCR+TYPW,ACCW+TYPL,0,0,0,0),
OP("decb",0x79,1,ACCM+TYPB,0,0,0,0,0),
OP("decl",0x7d,1,ACCM+TYPL,0,0,0,0,0),
OP("decw",0x7b,1,ACCM+TYPW,0,0,0,0,0),
OP("divd",0xf7,1,ACCR+TYPD,0,0,0,0,0),
OP("divf",0xf6,1,ACCR+TYPF,0,0,0,0,0),
OP("divl2",0x6c,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("divl3",0x7c,3,ACCR+TYPL,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("ediv",0x3e,4,ACCR+TYPL,ACCR+TYPQ,ACCW+TYPL,ACCW+TYPL,0,0),
OP("emul",0x2e,4,ACCR+TYPL,ACCR+TYPL,ACCR+TYPL,ACCW+TYPQ,0,0),
OP("expf",0x55,0,0,0,0,0,0,0),
OP("ffc",0xbe,2,ACCR+TYPL,ACCW+TYPL,0,0,0,0),
OP("ffs",0xae,2,ACCR+TYPL,ACCW+TYPL,0,0,0,0),
OP("halt",0x00,0,0,0,0,0,0,0),
OP("incb",0x69,1,ACCM+TYPB,0,0,0,0,0),
OP("incl",0x6d,1,ACCM+TYPL,0,0,0,0,0),
OP("incw",0x6b,1,ACCM+TYPW,0,0,0,0,0),
OP("insque",0xe0,2,ACCA+TYPL,ACCA+TYPL,0,0,0,0),
OP("jmp",0x71,1,ACCA+TYPB,0,0,0,0,0),
OP("kcall",0xcf,1,ACCR+TYPW,0,0,0,0,0),
OP("ldd",0x07,1,ACCR+TYPD,0,0,0,0,0),
OP("ldf",0x06,1,ACCR+TYPF,0,0,0,0,0),
OP("ldfd",0x97,1,ACCR+TYPF,0,0,0,0,0),
OP("ldpctx",0x60,0,0,0,0,0,0,0),
OP("lnd",0x17,1,ACCR+TYPD,0,0,0,0,0),
OP("lnf",0x16,1,ACCR+TYPF,0,0,0,0,0),
OP("loadr",0xab,2,ACCR+TYPW,ACCA+TYPL,0,0,0,0),
OP("logf",0x35,0,0,0,0,0,0,0),
OP("mcomb",0x29,2,ACCR+TYPB,ACCW+TYPB,0,0,0,0),
OP("mcoml",0x2d,2,ACCR+TYPL,ACCW+TYPL,0,0,0,0),
OP("mcomw",0x2b,2,ACCR+TYPW,ACCW+TYPW,0,0,0,0),
OP("mfpr",0xbd,2,ACCR+TYPL,ACCW+TYPL,0,0,0,0),
OP("mnegb",0xe8,2,ACCR+TYPB,ACCW+TYPB,0,0,0,0),
OP("mnegl",0xec,2,ACCR+TYPL,ACCW+TYPL,0,0,0,0),
OP("mnegw",0xea,2,ACCR+TYPW,ACCW+TYPW,0,0,0,0),
OP("movab",0xe9,2,ACCA+TYPB,ACCW+TYPL,0,0,0,0),
OP("moval",0xed,2,ACCA+TYPL,ACCW+TYPL,0,0,0,0),
OP("movaw",0xeb,2,ACCA+TYPW,ACCW+TYPL,0,0,0,0),
OP("movb",0x09,2,ACCR+TYPB,ACCW+TYPB,0,0,0,0),
OP("movblk",0xf8,0,0,0,0,0,0,0),
OP("movl",0x0d,2,ACCR+TYPL,ACCW+TYPL,0,0,0,0),
OP("movob",0xc9,2,ACCR+TYPB,ACCW+TYPB,0,0,0,0),
OP("movow",0xcb,2,ACCR+TYPW,ACCW+TYPW,0,0,0,0),
OP("movpsl",0xcd,1,ACCW+TYPL,0,0,0,0,0),
OP("movs2",0x82,0,0,0,0,0,0,0),
OP("movs3",0xc2,0,0,0,0,0,0,0),
OP("movw",0x0b,2,ACCR+TYPW,ACCW+TYPW,0,0,0,0),
OP("movzbl",0xa9,2,ACCR+TYPB,ACCW+TYPL,0,0,0,0),
OP("movzbw",0xb9,2,ACCR+TYPB,ACCW+TYPW,0,0,0,0),
OP("movzwl",0xc3,2,ACCR+TYPW,ACCW+TYPL,0,0,0,0),
OP("mtpr",0xad,2,ACCR+TYPL,ACCR+TYPL,0,0,0,0),
OP("muld",0xe7,1,ACCR+TYPD,0,0,0,0,0),
OP("mulf",0xe6,1,ACCR+TYPF,0,0,0,0,0),
OP("mull2",0x4c,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("mull3",0x5c,3,ACCR+TYPL,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("negd",0xb7,0,0,0,0,0,0,0),
OP("negf",0xb6,0,0,0,0,0,0,0),
OP("nop",0x10,0,0,0,0,0,0,0),
OP("orb2",0x88,2,ACCR+TYPB,ACCM+TYPB,0,0,0,0),
OP("orb3",0x98,3,ACCR+TYPB,ACCR+TYPB,ACCW+TYPB,0,0,0),
OP("orl2",0x8c,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("orl3",0x9c,3,ACCR+TYPL,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("orw2",0x8a,2,ACCR+TYPW,ACCM+TYPW,0,0,0,0),
OP("orw3",0x9a,3,ACCR+TYPW,ACCR+TYPW,ACCW+TYPW,0,0,0),
OP("prober",0xc0,3,ACCR+TYPB,ACCA+TYPB,ACCR+TYPL,0,0,0),
OP("probew",0xd0,3,ACCR+TYPB,ACCA+TYPB,ACCR+TYPL,0,0,0),
OP("pushab",0xf9,1,ACCA+TYPB,0,0,0,0,0),
OP("pushal",0xfd,1,ACCA+TYPL,0,0,0,0,0),
OP("pushaw",0xfb,1,ACCA+TYPW,0,0,0,0,0),
OP("pushb",0xd9,1,ACCR+TYPB,0,0,0,0,0),
OP("pushd",0x67,0,0,0,0,0,0,0),
OP("pushl",0xdd,1,ACCR+TYPL,0,0,0,0,0),
OP("pushw",0xdb,1,ACCR+TYPW,0,0,0,0,0),
OP("rei",0x20,0,0,0,0,0,0,0),
OP("remque",0xf0,1,ACCA+TYPL,0,0,0,0,0),
OP("ret",0x40,0,0,0,0,0,0,0),
OP("sbwc",0x9d,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("shal",0x4e,3,ACCR+TYPB,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("shar",0x5e,3,ACCR+TYPB,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("shll",0x48,3,ACCR+TYPB,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("shlq",0x4a,3,ACCR+TYPB,ACCR+TYPQ,ACCW+TYPQ,0,0,0),
OP("shrl",0x58,3,ACCR+TYPB,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("shrq",0x5a,3,ACCR+TYPB,ACCR+TYPQ,ACCW+TYPQ,0,0,0),
OP("sinf",0x05,0,0,0,0,0,0,0),
OP("sqrtf",0x45,0,0,0,0,0,0,0),
OP("std",0x27,1,ACCW+TYPD,0,0,0,0,0),
OP("stf",0x26,1,ACCW+TYPF,0,0,0,0,0),
OP("storer",0xbb,2,ACCR+TYPW,ACCA+TYPL,0,0,0,0),
OP("suba",0x9e,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("subb2",0x28,2,ACCR+TYPB,ACCM+TYPB,0,0,0,0),
OP("subb3",0x38,3,ACCR+TYPB,ACCR+TYPB,ACCW+TYPB,0,0,0),
OP("subd",0xd7,1,ACCR+TYPD,0,0,0,0,0),
OP("subf",0xd6,1,ACCR+TYPF,0,0,0,0,0),
OP("subl2",0x2c,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("subl3",0x3c,3,ACCR+TYPL,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("subw2",0x2a,2,ACCR+TYPW,ACCM+TYPW,0,0,0,0),
OP("subw3",0x3a,3,ACCR+TYPW,ACCR+TYPW,ACCW+TYPW,0,0,0),
OP("svpctx",0x70,0,0,0,0,0,0,0),
OP("tstb",0x59,1,ACCR+TYPB,0,0,0,0,0),
OP("tstd",0x57,0,0,0,0,0,0,0),
OP("tstf",0x56,0,0,0,0,0,0,0),
OP("tstl",0x5d,1,ACCR+TYPL,0,0,0,0,0),
OP("tstw",0x5b,1,ACCR+TYPW,0,0,0,0,0),
OP("xorb2",0xc8,2,ACCR+TYPB,ACCM+TYPB,0,0,0,0),
OP("xorb3",0xd8,3,ACCR+TYPB,ACCR+TYPB,ACCW+TYPB,0,0,0),
OP("xorl2",0xcc,2,ACCR+TYPL,ACCM+TYPL,0,0,0,0),
OP("xorl3",0xdc,3,ACCR+TYPL,ACCR+TYPL,ACCW+TYPL,0,0,0),
OP("xorw2",0xca,2,ACCR+TYPW,ACCM+TYPW,0,0,0,0),
OP("xorw3",0xda,3,ACCR+TYPW,ACCR+TYPW,ACCW+TYPW,0,0,0),
