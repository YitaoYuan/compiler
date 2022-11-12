  .data
S1:
    .word   0
S2:
    .zero   8388608
S3:
    .zero   8388608
S4:
    .zero   8388608
S5:
    .zero   8388608
  .text
  .globl main
  .globl getch
  .globl getarray
  .globl putint
  .globl putch
  .globl putarray
  .globl starttime
  .globl stoptime
multiply:
  addi  sp,  sp,  -32
  sw    ra,  28(sp)
  sw    s0,  24(sp)
  sw    s1,  20(sp)
  sw    s2,  16(sp)
  sw    s3,  12(sp)
.label1:
  xori  s0,  a1,  0
  seqz  s0,  s0
  sw    a1,  4(sp)
  sw    a0,  8(sp)
  beqz  s0,  .label3
.next2:
  li    a0,  0
  j     .ret_label1
.label3:
  lw    s0,  4(sp) ;b
  xori  s1,  s0,  1
  seqz  s1,  s1
  beqz  s1,  .label6
.next5:
  lw    s0,  8(sp) ;a
  li    s2,  998244353
  rem   s1,  s0,  s2
  mv    a0,  s1
  j     .ret_label1
.label6:
  lw    s0,  8(sp)
  lw    s1,  4(sp)
  li    s3,  2
  div   s2,  s1,  s3
  mv    a0,  s0
  mv    a1,  s2
  call  multiply
  add   s0,  a0,  a0
  li    s3,  998244353
  rem   s2,  s0,  s3
  li    s3,  2
  rem   s0,  s1,  s3
  xori  s1,  s0,  1
  seqz  s1,  s1
  sw    s2,  0(sp) ; cur
  beqz  s1,  .label9
.next8:
  lw    s0,  0(sp)
  lw    s1,  8(sp)
  add   s2,  s0,  s1
  li    s1,  998244353
  rem   s0,  s2,  s1
  mv    a0,  s0
  j     .ret_label1
.label9:
  lw    s0,  0(sp)
  mv    a0,  s0
  j     .ret_label1
.ret_label1:
  lw    ra,  28(sp)
  lw    s0,  24(sp)
  lw    s1,  20(sp)
  lw    s2,  16(sp)
  lw    s3,  12(sp)
  addi  sp,  sp,  32
  ret
power:
  addi  sp,  sp,  -32
  sw    ra,  28(sp)
  sw    s0,  24(sp)
  sw    s1,  20(sp)
  sw    s2,  16(sp)
  sw    s3,  12(sp)
.label13:
  xori  s0,  a1,  0
  seqz  s0,  s0
  sw    a1,  4(sp)
  sw    a0,  8(sp)
  beqz  s0,  .label15
.next14:
  li    a0,  1
  j     .ret_label2
.label15:
  lw    s0,  8(sp) ;a 
  lw    s1,  4(sp) ;b
  li    s3,  2
  div   s2,  s1,  s3
  mv    a0,  s0
  mv    a1,  s2
  call  power ; ??? 我的a0呢
  call  multiply
  li    s2,  2
  rem   s0,  s1,  s2
  xori  s1,  s0,  1
  seqz  s1,  s1
  sw    a0,  0(sp) ; cur
  beqz  s1,  .label18
.next17:
  lw    s0,  0(sp)
  lw    s1,  8(sp)
  mv    a0,  s0
  mv    a1,  s1
  call  multiply
  mv    a0,  a0
  j     .ret_label2
.label18:
  lw    s0,  0(sp)
  mv    a0,  s0
  j     .ret_label2
.ret_label2:
  lw    ra,  28(sp)
  lw    s0,  24(sp)
  lw    s1,  20(sp)
  lw    s2,  16(sp)
  lw    s3,  12(sp)
  addi  sp,  sp,  32
  ret
MemMove:
  addi  sp,  sp,  -20
.label22:
  li    t0,  0
  sw    t0,  0(sp)
  sw    a3,  4(sp)
  sw    a2,  8(sp)
  sw    a1,  12(sp)
  sw    a0,  16(sp)
.label23:
  lw    t0,  0(sp)
  lw    t1,  4(sp)
  slt   t2,  t0,  t1
  beqz  t2,  .label25
.next24:
  lw    t0,  0(sp)
  lw    t1,  8(sp)
  li    t2,  4
  mul   t2,  t2,  t0
  add   t2,  t1,  t2
  lw    t1,  0(t2)
  lw    t2,  12(sp)
  add   t3,  t2,  t0
  lw    t2,  16(sp)
  li    t4,  4
  mul   t4,  t4,  t3
  add   t4,  t2,  t4
  sw    t1,  0(t4)
  addi  t1,  t0,  1
  sw    t1,  0(sp)
  j     .label23
.label25:
  lw    t0,  0(sp)
  mv    a0,  t0
  j     .ret_label3
.ret_label3:
  addi  sp,  sp,  20
  ret
fft:
  addi  sp,  sp,  -76
  sw    ra,  72(sp)
  sw    s0,  68(sp)
  sw    s1,  64(sp)
  sw    s2,  60(sp)
  sw    s3,  56(sp)
  sw    s4,  52(sp)
  sw    s5,  48(sp)
  sw    s6,  44(sp)
  sw    s7,  40(sp)
  sw    s8,  36(sp)
.label27:
  xori  s0,  a2,  1
  seqz  s0,  s0
  sw    a3,  20(sp)
  sw    a2,  24(sp)
  sw    a1,  28(sp)
  sw    a0,  32(sp)
  beqz  s0,  .label29
.next28:
  li    a0,  1
  j     .ret_label4
.label29:
  li    s0,  0
  sw    s0,  16(sp)
.label31:
  lw    s0,  16(sp)
  lw    s1,  24(sp)
  slt   s2,  s0,  s1
  beqz  s2,  .label33
.next32:
  lw    s0,  16(sp)
  li    s2,  2
  rem   s1,  s0,  s2
  xori  s0,  s1,  0
  seqz  s0,  s0
  beqz  s0,  .label35
  j     .next34
.label33:
  lw    s0,  32(sp)
  lw    s1,  28(sp)
  la    s2,  S2
  addi  s3,  s2,  0
  lw    s2,  24(sp)
  mv    a0,  s0
  mv    a1,  s1
  mv    a2,  s3
  mv    a3,  s2
  call  MemMove
  li    s4,  2
  div   s3,  s2,  s4
  lw    s2,  20(sp)
  mv    a0,  s2
  mv    a1,  s2
  call  multiply
  sw    a0,  0(sp)
  mv    a0,  s0
  mv    a1,  s1
  mv    a2,  s3
  lw    a3,  0(sp)
  call  fft
  add   s4,  s1,  s3
  mv    a0,  s2
  mv    a1,  s2
  call  multiply
  sw    a0,  0(sp)
  mv    a0,  s0
  mv    a1,  s4
  mv    a2,  s3
  lw    a3,  0(sp)
  call  fft
  li    s0,  1
  sw    s0,  12(sp)
  li    s0,  0
  sw    s0,  16(sp)
  j     .label37
.next34:
  lw    s0,  16(sp)
  lw    s1,  28(sp)
  add   s2,  s0,  s1
  lw    s1,  32(sp)
  li    s3,  4
  mul   s3,  s3,  s2
  add   s3,  s1,  s3
  lw    s1,  0(s3)
  li    s3,  2
  div   s2,  s0,  s3
  la    s0,  S2
  li    s3,  4
  mul   s3,  s3,  s2
  add   s3,  s0,  s3
  sw    s1,  0(s3)
  j     .label36
.label35:
  lw    s0,  16(sp)
  lw    s1,  28(sp)
  add   s2,  s0,  s1
  lw    s1,  32(sp)
  li    s3,  4
  mul   s3,  s3,  s2
  add   s3,  s1,  s3
  lw    s1,  0(s3)
  lw    s2,  24(sp)
  li    s4,  2
  div   s3,  s2,  s4
  li    s4,  2
  div   s2,  s0,  s4
  add   s0,  s3,  s2
  la    s2,  S2
  li    s3,  4
  mul   s3,  s3,  s0
  add   s3,  s2,  s3
  sw    s1,  0(s3)
  j     .label36
.label37:
  lw    s0,  16(sp)
  lw    s1,  24(sp)
  li    s3,  2
  div   s2,  s1,  s3
  slt   s1,  s0,  s2
  beqz  s1,  .label39
  j     .next38
.label36:
  lw    s0,  16(sp)
  addi  s1,  s0,  1
  sw    s1,  16(sp)
  j     .label31
.next38:
  lw    s0,  28(sp)
  lw    s1,  16(sp)
  add   s2,  s0,  s1
  lw    s0,  32(sp)
  li    s3,  4
  mul   s3,  s3,  s2
  add   s3,  s0,  s3
  lw    s4,  0(s3)
  lw    s5,  24(sp)
  li    s7,  2
  div   s6,  s5,  s7
  add   s5,  s2,  s6
  li    s2,  4
  mul   s2,  s2,  s5
  add   s2,  s0,  s2
  lw    s0,  0(s2)
  lw    s5,  12(sp)
  mv    a0,  s5
  mv    a1,  s0
  call  multiply
  add   s6,  s4,  a0
  li    s8,  998244353
  rem   s7,  s6,  s8
  sw    s7,  0(s3)
  mv    a0,  s5
  mv    a1,  s0
  call  multiply
  sub   s3,  s4,  a0
  li    t6,  998244353
  add   s6,  s3,  t6
  li    s7,  998244353
  rem   s3,  s6,  s7
  sw    s3,  0(s2)
  lw    s2,  20(sp)
  mv    a0,  s5
  mv    a1,  s2
  call  multiply
  addi  s2,  s1,  1
  sw    s0,  4(sp)
  sw    a0,  12(sp)
  sw    s4,  8(sp)
  sw    s2,  16(sp)
  j     .label37
.label39:
  li    a0,  0
  j     .ret_label4
.ret_label4:
  lw    ra,  72(sp)
  lw    s0,  68(sp)
  lw    s1,  64(sp)
  lw    s2,  60(sp)
  lw    s3,  56(sp)
  lw    s4,  52(sp)
  lw    s5,  48(sp)
  lw    s6,  44(sp)
  lw    s7,  40(sp)
  lw    s8,  36(sp)
  addi  sp,  sp,  76
  ret
main:
  addi  sp,  sp,  -44
  sw    ra,  40(sp)
  sw    s0,  36(sp)
  sw    s1,  32(sp)
  sw    s2,  28(sp)
  sw    s3,  24(sp)
  sw    s4,  20(sp)
.label41:
  la    s0,  S3
  addi  s1,  s0,  0
  mv    a0,  s1
  call  getarray
  la    s0,  S4
  addi  s1,  s0,  0
  sw    a0,  4(sp)
  mv    a0,  s1
  call  getarray
  sw    a0,  0(sp)
  call  starttime
  lw    s0,  0(sp)
  sw    s0,  12(sp)
  lw    s0,  4(sp)
  sw    s0,  16(sp)
  li    s0,  1
  la    s1,  S1
  sw    s0,  0(s1)
.label42:
  la    s0,  S1
  lw    s1,  0(s0)
  lw    s0,  16(sp)
  lw    s2,  12(sp)
  add   s3,  s0,  s2
  addi  s0,  s3,  -1
  slt   s2,  s1,  s0
  beqz  s2,  .label44
.next43:
  la    s0,  S1
  lw    s1,  0(s0)
  li    s2,  2
  mul   s2,  s1,  s2
  sw    s2,  0(s0)
  j     .label42
.label44:
  la    s0,  S3
  addi  s1,  s0,  0
  la    s0,  S1
  lw    s2,  0(s0)
  li    s4,  998244352
  div   s3,  s4,  s2
  li    a0,  3
  mv    a1,  s3
  call  power
  sw    a0,  4(sp)
  mv    a0,  s1
  li    a1,  0
  mv    a2,  s2
  lw    a3,  4(sp)
  call  fft
  la    s1,  S4
  addi  s2,  s1,  0
  lw    s1,  0(s0)
  li    s3,  998244352
  div   s0,  s3,  s1
  li    a0,  3
  mv    a1,  s0
  call  power
  sw    a0,  4(sp)
  mv    a0,  s2
  li    a1,  0
  mv    a2,  s1
  lw    a3,  4(sp)
  call  fft
  li    s0,  0
  sw    s0,  8(sp)
.label45:
  lw    s0,  8(sp)
  la    s1,  S1
  lw    s2,  0(s1)
  slt   s1,  s0,  s2
  beqz  s1,  .label47
.next46:
  lw    s0,  8(sp)
  la    s1,  S3
  li    s2,  4
  mul   s2,  s2,  s0
  add   s2,  s1,  s2
  lw    s1,  0(s2)
  la    s3,  S4
  li    s4,  4
  mul   s4,  s4,  s0
  add   s4,  s3,  s4
  lw    s3,  0(s4)
  mv    a0,  s1
  mv    a1,  s3
  call  multiply
  sw    a0,  0(s2)
  addi  s1,  s0,  1
  sw    s1,  8(sp)
  j     .label45
.label47:
  la    s0,  S3
  addi  s1,  s0,  0
  la    s0,  S1
  lw    s2,  0(s0)
  li    s3,  998244352
  div   s0,  s3,  s2
  li    s3,  998244352
  sub   s3,  s3,  s0
  li    a0,  3
  mv    a1,  s3
  call  power
  sw    a0,  4(sp)
  mv    a0,  s1
  li    a1,  0
  mv    a2,  s2
  lw    a3,  4(sp)
  call  fft
  li    s0,  0
  sw    s0,  8(sp)
.label48:
  lw    s0,  8(sp)
  la    s1,  S1
  lw    s2,  0(s1)
  slt   s1,  s0,  s2
  beqz  s1,  .label50
.next49:
  lw    s0,  8(sp)
  la    s1,  S3
  li    s2,  4
  mul   s2,  s2,  s0
  add   s2,  s1,  s2
  lw    s1,  0(s2)
  la    s3,  S1
  lw    s4,  0(s3)
  mv    a0,  s4
  li    a1,  998244351
  call  power
  sw    a0,  4(sp)
  mv    a0,  s1
  lw    a1,  4(sp)
  call  multiply
  sw    a0,  0(s2)
  addi  s1,  s0,  1
  sw    s1,  8(sp)
  j     .label48
.label50:
  call  stoptime
  lw    s0,  16(sp)
  lw    s1,  12(sp)
  add   s2,  s0,  s1
  addi  s0,  s2,  -1
  la    s1,  S3
  addi  s2,  s1,  0
  mv    a0,  s0
  mv    a1,  s2
  call  putarray
  li    a0,  0
  j     .ret_label5
.ret_label5:
  lw    ra,  40(sp)
  lw    s0,  36(sp)
  lw    s1,  32(sp)
  lw    s2,  28(sp)
  lw    s3,  24(sp)
  lw    s4,  20(sp)
  addi  sp,  sp,  44
  ret