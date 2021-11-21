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
  %PI26 = load float, float* %PI, align 4
  ret float %PI26

then:                                             ; preds = %"while loop"
  %PI6 = load float, float* %PI, align 4
  %i7 = load i32, i32* %i, align 4
  %i8 = load i32, i32* %i, align 4
  %addtmp = add i32 %i8, 1
  %multmp = mul i32 %i7, %addtmp
  %"converted RHS to type FLOAT" = sitofp i32 %multmp to float
  %dictmp = fdiv float 4.000000e+00, %"converted RHS to type FLOAT"
  %addtmp9 = fadd float %dictmp, 2.000000e+00
  %addtmp10 = fadd float %PI6, %addtmp9
  %PI11 = load float, float* %PI, align 4
  store float %addtmp10, float* %PI, align 4
  br label %"after if block"

"else bock":                                      ; preds = %"while loop"
  %PI12 = load float, float* %PI, align 4
  %i13 = load i32, i32* %i, align 4
  %i14 = load i32, i32* %i, align 4
  %addtmp15 = add i32 %i14, 1
  %multmp16 = mul i32 %i13, %addtmp15
  %"converted RHS to type FLOAT17" = sitofp i32 %multmp16 to float
  %dictmp18 = fdiv float 4.000000e+00, %"converted RHS to type FLOAT17"
  %addtmp19 = fadd float %dictmp18, 2.000000e+00
  %subtmp = fsub float %PI12, %addtmp19
  %PI20 = load float, float* %PI, align 4
  store float %subtmp, float* %PI, align 4
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi float [ %addtmp10, %then ], [ %subtmp, %"else bock" ]
  %flag21 = load i1, i1* %flag, align 1
  %"not temp" = xor i1 %flag21, true
  %flag22 = load i1, i1* %flag, align 1
  store i1 %"not temp", i1* %flag, align 1
  %i23 = load i32, i32* %i, align 4
  %addtmp24 = add i32 %i23, 2
  %i25 = load i32, i32* %i, align 4
  store i32 %addtmp24, i32* %i, align 4
  br label %condition
}
