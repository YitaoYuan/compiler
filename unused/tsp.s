decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime()

global @S1 = alloc [ i32 , 20000000 ], zeroinit
global @S2 = alloc [ i32 , 100000 ], zeroinit

fun @transpose(%3 : i32, @S4 : *i32, %5 : i32): i32 {
				// n      matrix[]    rowsize
%label1:
  @S6 = alloc i32
  store %3, @S6  // n
  @S7 = alloc i32
  store %5, @S7  // rowsize
  %8 = load @S6
  %9 = load @S7
  %10 = div %8, %9
  @S11 = alloc i32
  store %10, @S11 // colsize = n / rowsize
  @S12 = alloc i32
  store 0, @S12 // i = 0;
  @S13 = alloc i32
  store 0, @S13 // j = 0;
  jump %label2
%label2:
  %14 = load @S12
  %15 = load @S11
  %16 = lt %14, %15 // i < colsize
  br %16, %label3, %label4
%label3:
  store 0, @S13
  jump %label5 // j = 0;
%label5:
  %17 = load @S13
  %18 = load @S7
  %19 = lt %17, %18 // j < rowsize
  br %19, %label6, %label7
%label6:
  %20 = load @S12
  %21 = load @S13
  %22 = lt %20, %21 // i < j
  br %22, %label8, %label9
%label8:
  %23 = load @S13
  %24 = add %23, 1
  store %24, @S13 // j = j + 1
  jump %label5
%label10:
  jump %label9
%label9:
  %25 = load @S12 // i
  %26 = load @S7 // rowsize
  %27 = mul %25, %26
  %28 = load @S13 // j
  %29 = add %27, %28
  %30 = getptr @S4, %29 // &matrix[i * rowsize + j]
  %31 = load %30
  @S32 = alloc i32
  store %31, @S32 // curr = matrix[i * rowsize + j]
  %33 = load @S12
  %34 = load @S7
  %35 = mul %33, %34
  %36 = load @S13
  %37 = add %35, %36
  %38 = getptr @S4, %37
  %39 = load %38 // matrix[i * rowsize + j]
  %40 = load @S13
  %41 = load @S11
  %42 = mul %40, %41
  %43 = load @S12
  %44 = add %42, %43
  %45 = getptr @S4, %44 // &matrix[j * colsize + i]
  store %39, %45 // ... = ...
  %46 = load @S32 // curr
  %47 = load @S12
  %48 = load @S7
  %49 = mul %47, %48
  %50 = load @S13
  %51 = add %49, %50
  %52 = getptr @S4, %51 // &matrix[i * rowsize + j]
  store %46, %52 // matrix[i * rowsize + j] = curr;
  %53 = load @S13
  %54 = add %53, 1
  store %54, @S13 // j = j + 1;
  jump %label5
%label7:
  %55 = load @S12
  %56 = add %55, 1
  store %56, @S12 // i = i + 1;
  jump %label2
%label4:
  ret -1
%label11:
  ret 0
}

fun @main(): i32 {
%label12:
  %57 = call @getint ()
  @S58 = alloc i32
  store %57, @S58 // n
  %59 = getelemptr @S2, 0
  %60 = call @getarray (%59)
  @S61 = alloc i32
  store %60, @S61 // len
  call @starttime ()
  @S62 = alloc i32
  store 0, @S62  // i
  jump %label13
%label13:
  %63 = load @S62
  %64 = load @S58
  %65 = lt %63, %64
  br %65, %label14, %label15
%label14:
  %66 = load @S62
  %67 = load @S62
  %68 = getelemptr @S1, %67 // matrix[i]
  store %66, %68
  %69 = load @S62
  %70 = add %69, 1
  store %70, @S62 // i = i + 1
  jump %label13
%label15:
  store 0, @S62 // i = 0;
  jump %label16
%label16:
  %71 = load @S62
  %72 = load @S61
  %73 = lt %71, %72 // i < len
  br %73, %label17, %label18
%label17:
  %74 = load @S58 // n 
  %75 = getelemptr @S1, 0 // matrix
  %76 = load @S62 
  %77 = getelemptr @S2, %76
  %78 = load %77 // a[i]
  %79 = call @transpose (%74, %75, %78)
  %80 = load @S62
  %81 = add %80, 1
  store %81, @S62 // i = i + 1;
  jump %label16
%label18:
  @S82 = alloc i32
  store 0, @S82 // ans = 0;
  store 0, @S62 // i = 0;
  jump %label19
%label19:
  %83 = load @S62
  %84 = load @S61
  %85 = lt %83, %84 // i < len
  br %85, %label20, %label21
%label20:
  %86 = load @S82
  %87 = load @S62
  %88 = load @S62
  %89 = mul %87, %88 // i * i
  %90 = load @S62
  %91 = getelemptr @S1, %90 // &matrix[i]
  %92 = load %91
  %93 = mul %89, %92 // i * i * matrix[i]
  %94 = add %86, %93 // ans + ...
  store %94, @S82 // ans = ...
  %95 = load @S62
  %96 = add %95, 1
  store %96, @S62 // i = i + 1
  jump %label19
%label21:
  %97 = load @S82
  %98 = lt %97, 0 // ans < 0
  br %98, %label22, %label23
%label22:
  %99 = load @S82
  %100 = sub 0, %99
  store %100, @S82 // ans = -ans;
  jump %label23
%label23:
  call @stoptime ()
  %101 = load @S82
  call @putint (%101)
  call @putch (10)
  ret 0
%label24:
  ret 0
}
