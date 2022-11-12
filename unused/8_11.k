decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime()

global @S1 = alloc i32, zeroinit
global @S2 = alloc i32, zeroinit

fun @t(): i32 {
%label1:
  %4 = getptr @S1, 0
  %3 = load %4
  %5 = add %3, 1
  %6 = getptr @S1, 0
  store %5, %6
  ret 1
%label2:
  ret 0
}

fun @f(): i32 {
%label3:
  %8 = getptr @S2, 0
  %7 = load %8
  %9 = add %7, 1
  %10 = getptr @S2, 0
  store %9, %10
  ret 0
%label4:
  ret 0
}

fun @main(): i32 {
%label5:
  @S11 = alloc i32
  %T12 = alloc i32
  %13 = call @f ()
  %14 = ne 0, %13
  store %14, %T12
  %15 = add 0, 0
  store %15, @S11
  br %14, %label7, %next6
%next6:
  %16 = call @f ()
  %17 = ne 0, %16
  store %17, %T12
  jump %label7
%label7:
  %18 = load %T12
  %19 = add 0, %18
  %T20 = alloc i32
  %21 = call @f ()
  %22 = ne 0, %21
  store %22, %T20
  store %19, @S11
  br %22, %label9, %next8
%next8:
  %23 = call @t ()
  %24 = ne 0, %23
  store %24, %T20
  jump %label9
%label9:
  %25 = load %T20
  %26 = add %19, %25
  %T27 = alloc i32
  %28 = call @t ()
  %29 = ne 0, %28
  store %29, %T27
  store %26, @S11
  br %29, %label11, %next10
%next10:
  %30 = call @f ()
  %31 = ne 0, %30
  store %31, %T27
  jump %label11
%label11:
  %32 = load %T27
  %33 = add %26, %32
  %T34 = alloc i32
  %35 = call @t ()
  %36 = ne 0, %35
  store %36, %T34
  store %33, @S11
  br %36, %label13, %next12
%next12:
  %37 = call @t ()
  %38 = ne 0, %37
  store %38, %T34
  jump %label13
%label13:
  %39 = load %T34
  %40 = add %33, %39
  %T41 = alloc i32
  %42 = call @f ()
  %43 = ne 0, %42
  store %43, %T41
  store %40, @S11
  br %43, %next14, %label15
%next14:
  %44 = call @f ()
  %45 = ne 0, %44
  store %45, %T41
  jump %label15
%label15:
  %46 = load %T41
  %47 = add %40, %46
  %T48 = alloc i32
  %49 = call @f ()
  %50 = ne 0, %49
  store %50, %T48
  store %47, @S11
  br %50, %next16, %label17
%next16:
  %51 = call @t ()
  %52 = ne 0, %51
  store %52, %T48
  jump %label17
%label17:
  %53 = load %T48
  %54 = add %47, %53
  %T55 = alloc i32
  %56 = call @t ()
  %57 = ne 0, %56
  store %57, %T55
  store %54, @S11
  br %57, %next18, %label19
%next18:
  %58 = call @f ()
  %59 = ne 0, %58
  store %59, %T55
  jump %label19
%label19:
  %60 = load %T55
  %61 = add %54, %60
  %T62 = alloc i32
  %63 = call @t ()
  %64 = ne 0, %63
  store %64, %T62
  store %61, @S11
  br %64, %next20, %label21
%next20:
  %65 = call @t ()
  %66 = ne 0, %65
  store %66, %T62
  jump %label21
%label21:
  %67 = load %T62
  %68 = add %61, %67
  %T69 = alloc i32
  %70 = call @t ()
  %71 = ne 0, %70
  store %71, %T69
  store %68, @S11
  br %71, %label23, %next22
%next22:
  %T72 = alloc i32
  %73 = call @t ()
  %74 = ne 0, %73
  store %74, %T72
  br %74, %next24, %label25
%next24:
  %75 = call @t ()
  %76 = ne 0, %75
  store %76, %T72
  jump %label25
%label25:
  %77 = load %T72
  %78 = ne 0, %77
  store %78, %T69
  jump %label23
%label23:
  %79 = load %T69
  %T80 = alloc i32
  %81 = call @f ()
  %82 = ne 0, %81
  store %82, %T80
  br %82, %label27, %next26
%next26:
  %T83 = alloc i32
  %84 = call @t ()
  %85 = ne 0, %84
  store %85, %T83
  br %85, %next28, %label29
%next28:
  %86 = call @t ()
  %87 = ne 0, %86
  store %87, %T83
  jump %label29
%label29:
  %88 = load %T83
  %89 = ne 0, %88
  store %89, %T80
  jump %label27
%label27:
  %90 = load %T80
  %T91 = alloc i32
  %92 = call @f ()
  %93 = ne 0, %92
  store %93, %T91
  br %93, %label31, %next30
%next30:
  %T94 = alloc i32
  %95 = call @f ()
  %96 = ne 0, %95
  store %96, %T94
  br %96, %next32, %label33
%next32:
  %97 = call @t ()
  %98 = ne 0, %97
  store %98, %T94
  jump %label33
%label33:
  %99 = load %T94
  %100 = ne 0, %99
  store %100, %T91
  jump %label31
%label31:
  %101 = load %T91
  %T102 = alloc i32
  %T103 = alloc i32
  %104 = call @t ()
  %105 = ne 0, %104
  store %105, %T103
  br %105, %next34, %label35
%next34:
  %106 = call @t ()
  %107 = ne 0, %106
  store %107, %T103
  jump %label35
%label35:
  %108 = load %T103
  %109 = ne 0, %108
  store %109, %T102
  br %109, %label37, %next36
%next36:
  %110 = call @t ()
  %111 = ne 0, %110
  store %111, %T102
  jump %label37
%label37:
  %112 = load %T102
  %T113 = alloc i32
  %T114 = alloc i32
  %115 = call @f ()
  %116 = ne 0, %115
  store %116, %T114
  br %116, %next38, %label39
%next38:
  %117 = call @t ()
  %118 = ne 0, %117
  store %118, %T114
  jump %label39
%label39:
  %119 = load %T114
  %120 = ne 0, %119
  store %120, %T113
  br %120, %label41, %next40
%next40:
  %121 = call @t ()
  %122 = ne 0, %121
  store %122, %T113
  jump %label41
%label41:
  %123 = load %T113
  %T124 = alloc i32
  %T125 = alloc i32
  %126 = call @f ()
  %127 = ne 0, %126
  store %127, %T125
  br %127, %next42, %label43
%next42:
  %128 = call @f ()
  %129 = ne 0, %128
  store %129, %T125
  jump %label43
%label43:
  %130 = load %T125
  %131 = ne 0, %130
  store %131, %T124
  br %131, %label45, %next44
%next44:
  %132 = call @f ()
  %133 = ne 0, %132
  store %133, %T124
  jump %label45
%label45:
  %134 = load %T124
  %136 = getptr @S1, 0
  %135 = load %136
  call @putint (%135)
  call @putch (32)
  %138 = getptr @S2, 0
  %137 = load %138
  call @putint (%137)
  call @putch (10)
  %139 = load @S11
  ret %139
%label46:
  ret 0
}