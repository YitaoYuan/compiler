decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime()


fun @add(%1 : i32, %2 : i32): i32 {
%label1:
  @S3 = alloc i32
  @S4 = alloc i32
  %5 = add %1, %2
  ret %5
%label2:
  ret 0
}

fun @sub(%6 : i32, %7 : i32): i32 {
%label3:
  @S8 = alloc i32
  @S9 = alloc i32
  %10 = sub %6, %7
  ret %10
%label4:
  ret 0
}

fun @mul(%11 : i32, %12 : i32): i32 {
%label5:
  @S13 = alloc i32
  @S14 = alloc i32
  %15 = mul %11, %12
  ret %15
%label6:
  ret 0
}

fun @div(%16 : i32, %17 : i32): i32 {
%label7:
  @S18 = alloc i32
  @S19 = alloc i32
  %20 = div %16, %17
  ret %20
%label8:
  ret 0
}

fun @main(): i32 {
%label9:
  %21 = call @sub (1, 2)
  %22 = call @div (4, 5)
  %23 = call @mul (3, %22)
  %24 = call @add (%21, %23)
  @S25 = alloc i32 // x
  %T26 = alloc i32
  %T27 = alloc i32
  %28 = ne 0, 0
  %29 = eq 0, 0
  store %28, %T27
  store %24, @S25
  store %17, @S19
  store %16, @S18
  store %12, @S14
  store %11, @S13
  store %7, @S9
  store %6, @S8
  store %2, @S4
  store %1, @S3
  br %29, %label11, %label10
%label10:
  %30 = load @S25
  %31 = call @sub (1, %30)
  %32 = ne %31, 0
  store %32, %T27
  jump %label11
%label11:
  %33 = load %T27
  %34 = ne %33, 0
  store %34, %T26
  br %34, %label13, %label12
%label12:
  %T35 = alloc i32
  %36 = load @S25
  %37 = ne %36, 0
  store %37, %T35
  br %37, %label15, %label14
%label14:
  %38 = call @add (1, 2)
  %39 = lt 10, %38
  %40 = ne %39, 0
  store %40, %T35
  jump %label15
%label15:
  %41 = load %T35
  %42 = call @div (%41, 5)
  %43 = call @mul (3, %42)
  %44 = ne %43, 0
  store %44, %T26
  jump %label13
%label13:
  %45 = load %T26
  %46 = call @add (1, %45)
  @S47 = alloc i32
  %48 = load @S25
  %49 = add %48, %46
  ret %49
%label16:
  ret 0
}