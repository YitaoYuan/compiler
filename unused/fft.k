decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime()

global @S1 = alloc i32, zeroinit
global @S2 = alloc [ i32 , 2097152 ], zeroinit
global @S3 = alloc [ i32 , 2097152 ], zeroinit
global @S4 = alloc [ i32 , 2097152 ], zeroinit
global @S5 = alloc [ i32 , 2097152 ], zeroinit

fun @multiply(%6 : i32, %7 : i32): i32 {
%label1:
  @S8 = alloc i32
  @S9 = alloc i32
  %10 = eq %7, 0
  store %7, @S9
  store %6, @S8
  br %10, %next2, %label3
%next2:
  ret 0
%label4:
  jump %label3
%label3:
  %11 = load @S9
  %12 = eq %11, 1
  br %12, %next5, %label6
%next5:
  %13 = load @S8
  %14 = mod %13, 998244353
  ret %14
%label7:
  jump %label6
%label6:
  %15 = load @S8
  %16 = load @S9
  %17 = div %16, 2
  %18 = call @multiply (%15, %17)
  @S19 = alloc i32
  %20 = add %18, %18
  %21 = mod %20, 998244353
  %22 = mod %16, 2
  %23 = eq %22, 1
  store %21, @S19
  br %23, %next8, %label9
%next8:
  %24 = load @S19
  %25 = load @S8
  %26 = add %24, %25
  %27 = mod %26, 998244353
  ret %27
%label11:
  jump %label10
%label9:
  %28 = load @S19
  ret %28
%label12:
  jump %label10
%label10:
  ret 0
}

fun @power(%29 : i32, %30 : i32): i32 {
%label13:
  @S31 = alloc i32
  @S32 = alloc i32
  %33 = eq %30, 0
  store %30, @S32
  store %29, @S31
  br %33, %next14, %label15
%next14:
  ret 1
%label16:
  jump %label15
%label15:
  %34 = load @S31
  %35 = load @S32
  %36 = div %35, 2
  %37 = call @power (%34, %36)
  @S38 = alloc i32
  %39 = call @multiply (%37, %37)
  %40 = mod %35, 2
  %41 = eq %40, 1
  store %39, @S38
  br %41, %next17, %label18
%next17:
  %42 = load @S38
  %43 = load @S31
  %44 = call @multiply (%42, %43)
  ret %44
%label20:
  jump %label19
%label18:
  %45 = load @S38
  ret %45
%label21:
  jump %label19
%label19:
  ret 0
}

fun @MemMove(%46 : *i32, %47 : i32, %48 : *i32, %49 : i32): i32 {
%label22:
  @S50 = alloc *i32
  @S51 = alloc i32
  @S52 = alloc *i32
  @S53 = alloc i32
  @S54 = alloc i32
  %55 = add 0, 0
  store %55, @S54
  store %49, @S53
  store %48, @S52
  store %47, @S51
  store %46, @S50
  jump %label23
%label23:
  %56 = load @S54
  %57 = load @S53
  %58 = lt %56, %57
  br %58, %next24, %label25
%next24:
  %59 = load @S54
  %60 = load @S52
  %61 = getptr %60, %59
  %62 = load %61
  %63 = load @S51
  %64 = add %63, %59
  %65 = load @S50
  %66 = getptr %65, %64
  store %62, %66
  %67 = add %59, 1
  store %67, @S54
  jump %label23
%label25:
  %68 = load @S54
  ret %68
%label26:
  ret 0
}

fun @fft(%69 : *i32, %70 : i32, %71 : i32, %72 : i32): i32 {
%label27:
  @S73 = alloc *i32
  @S74 = alloc i32
  @S75 = alloc i32
  @S76 = alloc i32
  %77 = eq %71, 1
  store %72, @S76
  store %71, @S75
  store %70, @S74
  store %69, @S73
  br %77, %next28, %label29
%next28:
  ret 1
%label30:
  jump %label29
%label29:
  @S78 = alloc i32
  %79 = add 0, 0
  store %79, @S78
  jump %label31
%label31:
  %80 = load @S78
  %81 = load @S75
  %82 = lt %80, %81
  br %82, %next32, %label33
%next32:
  %83 = load @S78
  %84 = mod %83, 2
  %85 = eq %84, 0
  br %85, %next34, %label35
%next34:
  %86 = load @S78
  %87 = load @S74
  %88 = add %86, %87
  %89 = load @S73
  %90 = getptr %89, %88
  %91 = load %90
  %92 = div %86, 2
  %93 = getptr @S2, 0
  %94 = getelemptr %93, %92
  store %91, %94
  jump %label36
%label35:
  %95 = load @S78
  %96 = load @S74
  %97 = add %95, %96
  %98 = load @S73
  %99 = getptr %98, %97
  %100 = load %99
  %101 = load @S75
  %102 = div %101, 2
  %103 = div %95, 2
  %104 = add %102, %103
  %105 = getptr @S2, 0
  %106 = getelemptr %105, %104
  store %100, %106
  jump %label36
%label36:
  %107 = load @S78
  %108 = add %107, 1
  store %108, @S78
  jump %label31
%label33:
  %109 = load @S73
  %110 = load @S74
  %111 = getptr @S2, 0
  %112 = getelemptr %111, 0
  %113 = load @S75
  %114 = call @MemMove (%109, %110, %112, %113)
  %115 = div %113, 2
  %116 = load @S76
  %117 = call @multiply (%116, %116)
  %118 = call @fft (%109, %110, %115, %117)
  %119 = div %113, 2
  %120 = add %110, %119
  %121 = div %113, 2
  %122 = call @multiply (%116, %116)
  %123 = call @fft (%109, %120, %121, %122)
  @S124 = alloc i32
  %125 = add 0, 1
  store %125, @S124
  %126 = add 0, 0
  store %126, @S78
  jump %label37
%label37:
  %127 = load @S78
  %128 = load @S75
  %129 = div %128, 2
  %130 = lt %127, %129
  br %130, %next38, %label39
%next38:
  %131 = load @S74
  %132 = load @S78
  %133 = add %131, %132
  %134 = load @S73
  %135 = getptr %134, %133
  %136 = load %135
  @S137 = alloc i32
  %138 = add %131, %132
  %139 = load @S75
  %140 = div %139, 2
  %141 = add %138, %140
  %142 = getptr %134, %141
  %143 = load %142
  @S144 = alloc i32
  %145 = load @S124
  %146 = call @multiply (%145, %143)
  %147 = add %136, %146
  %148 = mod %147, 998244353
  %149 = add %131, %132
  %150 = getptr %134, %149
  store %148, %150
  %151 = call @multiply (%145, %143)
  %152 = sub %136, %151
  %153 = add %152, 998244353
  %154 = mod %153, 998244353
  %155 = add %131, %132
  %156 = div %139, 2
  %157 = add %155, %156
  %158 = getptr %134, %157
  store %154, %158
  %159 = load @S76
  %160 = call @multiply (%145, %159)
  %161 = add %132, 1
  store %143, @S144
  store %160, @S124
  store %136, @S137
  store %161, @S78
  jump %label37
%label39:
  ret 0
%label40:
  ret 0
}

fun @main(): i32 {
%label41:
  %162 = getptr @S3, 0
  %163 = getelemptr %162, 0
  %164 = call @getarray (%163)
  @S165 = alloc i32
  %166 = getptr @S4, 0
  %167 = getelemptr %166, 0
  %168 = call @getarray (%167)
  @S169 = alloc i32
  call @starttime ()
  store %168, @S169
  store %164, @S165
  %170 = add 0, 1
  %171 = getptr @S1, 0
  store %170, %171
  jump %label42
%label42:
  %173 = getptr @S1, 0
  %172 = load %173
  %174 = load @S165
  %175 = load @S169
  %176 = add %174, %175
  %177 = sub %176, 1
  %178 = lt %172, %177
  br %178, %next43, %label44
%next43:
  %180 = getptr @S1, 0
  %179 = load %180
  %181 = mul %179, 2
  %182 = getptr @S1, 0
  store %181, %182
  jump %label42
%label44:
  %183 = getptr @S3, 0
  %184 = getelemptr %183, 0
  %186 = getptr @S1, 0
  %185 = load %186
  %187 = div 998244352, %185
  %188 = call @power (3, %187)
  %189 = call @fft (%184, 0, %185, %188)
  %190 = getptr @S4, 0
  %191 = getelemptr %190, 0
  %193 = getptr @S1, 0
  %192 = load %193
  %194 = div 998244352, %192
  %195 = call @power (3, %194)
  %196 = call @fft (%191, 0, %192, %195)
  @S197 = alloc i32
  %198 = add 0, 0
  store %198, @S197
  jump %label45
%label45:
  %199 = load @S197
  %201 = getptr @S1, 0
  %200 = load %201
  %202 = lt %199, %200
  br %202, %next46, %label47
%next46:
  %203 = load @S197
  %204 = getptr @S3, 0
  %205 = getelemptr %204, %203
  %206 = load %205
  %207 = getptr @S4, 0
  %208 = getelemptr %207, %203
  %209 = load %208
  %210 = call @multiply (%206, %209)
  %211 = getptr @S3, 0
  %212 = getelemptr %211, %203
  store %210, %212
  %213 = add %203, 1
  store %213, @S197
  jump %label45
%label47:
  %214 = getptr @S3, 0
  %215 = getelemptr %214, 0
  %217 = getptr @S1, 0
  %216 = load %217
  %218 = div 998244352, %216
  %219 = sub 998244352, %218
  %220 = call @power (3, %219)
  %221 = call @fft (%215, 0, %216, %220)
  %222 = add 0, 0
  store %222, @S197
  jump %label48
%label48:
  %223 = load @S197
  %225 = getptr @S1, 0
  %224 = load %225
  %226 = lt %223, %224
  br %226, %next49, %label50
%next49:
  %227 = load @S197
  %228 = getptr @S3, 0
  %229 = getelemptr %228, %227
  %230 = load %229
  %232 = getptr @S1, 0
  %231 = load %232
  %233 = call @power (%231, 998244351)
  %234 = call @multiply (%230, %233)
  %235 = getptr @S3, 0
  %236 = getelemptr %235, %227
  store %234, %236
  %237 = add %227, 1
  store %237, @S197
  jump %label48
%label50:
  call @stoptime ()
  %238 = load @S165
  %239 = load @S169
  %240 = add %238, %239
  %241 = sub %240, 1
  %242 = getptr @S3, 0
  %243 = getelemptr %242, 0
  call @putarray (%241, %243)
  ret 0
%label51:
  ret 0
}