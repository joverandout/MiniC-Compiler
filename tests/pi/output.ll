; ModuleID = 'mini-c'
source_filename = "mini-c"

define float @pi() {
block:
  %i = alloca i32, align 4
  %PI = alloca float, align 4
  %flag = alloca i1, align 1
  %flag1 = load i1, i1* %flag, align 1
  store i1 true, i1* %flag, align 1
  %PI2 = load float, float* %PI, align 4
  store float 3.000000e+00, float* %PI, align 4
  %i3 = load i32, i32* %i, align 4
  store i32 2, i32* %i, align 4
  br label %condition

condition:                                        ; preds = %"after if block", %block
  %i4 = load i32, i32* %i, align 4
  %cmptemp = icmp ult i32 %i4, 100
  %booltmp = uitofp i1 %cmptemp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %flag5 = load i1, i1* %flag, align 1
  %ifconditionS = icmp ne i1 %flag5, false
  br i1 %ifconditionS, label %then, label %"else bock"

"after loop":                                     ; preds = %condition
  %PI30 = load float, float* %PI, align 4
  ret float %PI30

then:                                             ; preds = %"while loop"
  %PI6 = load float, float* %PI, align 4
  %i7 = load i32, i32* %i, align 4
  %i8 = load i32, i32* %i, align 4
  %addtmp = add i32 %i8, 1
  %multmp = mul i32 %i7, %addtmp
  %i9 = load i32, i32* %i, align 4
  %addtmp10 = add i32 %i9, 2
  %multmp11 = mul i32 %multmp, %addtmp10
  %"converted RHS to type FLOAT" = sitofp i32 %multmp11 to float
  %dictmp = fdiv float 4.000000e+00, %"converted RHS to type FLOAT"
  %addtmp12 = fadd float %PI6, %dictmp
  %PI13 = load float, float* %PI, align 4
  store float %addtmp12, float* %PI, align 4
  br label %"after if block"

"else bock":                                      ; preds = %"while loop"
  %PI14 = load float, float* %PI, align 4
  %i15 = load i32, i32* %i, align 4
  %i16 = load i32, i32* %i, align 4
  %addtmp17 = add i32 %i16, 1
  %multmp18 = mul i32 %i15, %addtmp17
  %i19 = load i32, i32* %i, align 4
  %addtmp20 = add i32 %i19, 2
  %multmp21 = mul i32 %multmp18, %addtmp20
  %"converted RHS to type FLOAT22" = sitofp i32 %multmp21 to float
  %dictmp23 = fdiv float 4.000000e+00, %"converted RHS to type FLOAT22"
  %subtmp = fsub float %PI14, %dictmp23
  %PI24 = load float, float* %PI, align 4
  store float %subtmp, float* %PI, align 4
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi float [ %addtmp12, %then ], [ %subtmp, %"else bock" ]
  %flag25 = load i1, i1* %flag, align 1
  %"not temp" = xor i1 %flag25, true
  %flag26 = load i1, i1* %flag, align 1
  store i1 %"not temp", i1* %flag, align 1
  %i27 = load i32, i32* %i, align 4
  %addtmp28 = add i32 %i27, 2
  %i29 = load i32, i32* %i, align 4
  store i32 %addtmp28, i32* %i, align 4
  br label %condition
}
