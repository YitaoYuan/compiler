  .data
S1:
    .word   0
S2:
    .word   0
  .text
  .globl main
  .globl getch
  .globl getarray
  .globl putint
  .globl putch
  .globl putarray
  .globl starttime
  .globl stoptime
t:
  addi  sp,  sp,  0
.label1:
  la    t0,  S1
  lw    t1,  0(t0)
  addi  t2,  t1,  1
  sw    t2,  0(t0)
  li    a0,  1
  j     .ret_label1
.ret_label1:
  addi  sp,  sp,  0
  ret
f:
  addi  sp,  sp,  0
.label3:
  la    t0,  S2
  lw    t1,  0(t0)
  addi  t2,  t1,  1
  sw    t2,  0(t0)
  li    a0,  0
  j     .ret_label2
.ret_label2:
  addi  sp,  sp,  0
  ret
main:
  addi  sp,  sp,  -100
  sw    ra,  96(sp)
  sw    s0,  92(sp)
  sw    s1,  88(sp)
  sw    s2,  84(sp)
.label5:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  76(sp)
  li    s1,  0
  sw    s1,  80(sp)
  bnez  s0,  .label7
  j     .next6
.label7:
  lw    s0,  76(sp)
  addi  s1,  s0,  0
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  72(sp)
  sw    s1,  80(sp)
  bnez  s0,  .label9
  j     .next8
.next6:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  76(sp)
  j     .label7
.label9:
  lw    s0,  72(sp)
  add   s1,  s2,  s0
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  68(sp)
  sw    s1,  80(sp)
  bnez  s0,  .label11
  j     .next10
.next8:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  72(sp)
  j     .label9
.label11:
  lw    s0,  68(sp)
  add   s1,  s2,  s0
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  64(sp)
  sw    s1,  80(sp)
  bnez  s0,  .label13
  j     .next12
.next10:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  68(sp)
  j     .label11
.label13:
  lw    s0,  64(sp)
  add   s1,  s2,  s0
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  60(sp)
  sw    s1,  80(sp)
  beqz  s0,  .label15
  j     .next14
.next12:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  64(sp)
  j     .label13
.next14:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  60(sp)
.label15:
  lw    s0,  60(sp)
  add   s1,  s2,  s0
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  56(sp)
  sw    s1,  80(sp)
  beqz  s0,  .label17
.next16:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  56(sp)
.label17:
  lw    s0,  56(sp)
  add   s1,  s2,  s0
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  52(sp)
  sw    s1,  80(sp)
  beqz  s0,  .label19
.next18:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  52(sp)
.label19:
  lw    s0,  52(sp)
  add   s1,  s2,  s0
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  48(sp)
  sw    s1,  80(sp)
  beqz  s0,  .label21
.next20:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  48(sp)
.label21:
  lw    s0,  48(sp)
  add   s1,  s2,  s0
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  44(sp)
  sw    s1,  80(sp)
  bnez  s0,  .label23
  j     .next22
.label23:
  lw    s0,  44(sp)
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  40(sp)
  bnez  s0,  .label27
  j     .next26
.next22:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  36(sp)
  beqz  s0,  .label25
  j     .next24
.label27:
  lw    s0,  40(sp)
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  32(sp)
  bnez  s0,  .label31
  j     .next30
.next26:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  28(sp)
  beqz  s0,  .label29
  j     .next28
.next24:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  36(sp)
.label25:
  lw    s0,  36(sp)
  xori  s1,  s0,  0
  snez  s1,  s1
  sw    s1,  44(sp)
  j     .label23
.label31:
  lw    s0,  32(sp)
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  20(sp)
  beqz  s0,  .label35
  j     .next34
.next30:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  16(sp)
  beqz  s0,  .label33
  j     .next32
.next28:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  28(sp)
.label29:
  lw    s0,  28(sp)
  xori  s1,  s0,  0
  snez  s1,  s1
  sw    s1,  40(sp)
  j     .label27
.next34:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  20(sp)
.label35:
  lw    s0,  20(sp)
  xori  s1,  s0,  0
  snez  s1,  s1
  sw    s1,  24(sp)
  bnez  s1,  .label37
  j     .next36
.next32:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  16(sp)
.label33:
  lw    s0,  16(sp)
  xori  s1,  s0,  0
  snez  s1,  s1
  sw    s1,  32(sp)
  j     .label31
.label37:
  lw    s0,  24(sp)
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  8(sp)
  beqz  s0,  .label39
  j     .next38
.next36:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  24(sp)
  j     .label37
.next38:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  8(sp)
.label39:
  lw    s0,  8(sp)
  xori  s1,  s0,  0
  snez  s1,  s1
  sw    s1,  12(sp)
  bnez  s1,  .label41
  j     .next40
.label41:
  lw    s0,  12(sp)
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  0(sp)
  beqz  s0,  .label43
  j     .next42
.next40:
  call  t
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  12(sp)
  j     .label41
.next42:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  0(sp)
.label43:
  lw    s0,  0(sp)
  xori  s1,  s0,  0
  snez  s1,  s1
  sw    s1,  4(sp)
  bnez  s1,  .label45
  j     .next44
.label45:
  lw    s0,  4(sp)
  la    s0,  S1
  lw    s1,  0(s0)
  mv    a0,  s1
  call  putint
  li    a0,  32
  call  putch
  la    s0,  S2
  lw    s1,  0(s0)
  mv    a0,  s1
  call  putint
  li    a0,  10
  call  putch
  lw    s0,  80(sp)
  mv    a0,  s0
  j     .ret_label3
.next44:
  call  f
  xori  s0,  a0,  0
  snez  s0,  s0
  sw    s0,  4(sp)
  j     .label45
.ret_label3:
  lw    ra,  96(sp)
  lw    s0,  92(sp)
  lw    s1,  88(sp)
  lw    s2,  84(sp)
  addi  sp,  sp,  100
  ret