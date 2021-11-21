; ModuleID = 'mini-c'
source_filename = "mini-c"

define float @pi() {
block:
  %d = alloca float, align 4
  %c = alloca float, align 4
  %b = alloca float, align 4
  %a = alloca float, align 4
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
  %i5 = load i32, i32* %i, align 4
  %"converted LHS to type FLOAT" = sitofp i32 %i5 to float
  %addtmp = fadd float %"converted LHS to type FLOAT", 2.000000e+00
  %a6 = load float, float* %a, align 4
  store float %addtmp, float* %a, align 4
  %i7 = load i32, i32* %i, align 4
  %"converted LHS to type FLOAT8" = sitofp i32 %i7 to float
  %addtmp9 = fadd float %"converted LHS to type FLOAT8", 1.000000e+00
  %b10 = load float, float* %b, align 4
  store float %addtmp9, float* %b, align 4
  %i11 = load i32, i32* %i, align 4
  %a12 = load float, float* %a, align 4
  %b13 = load float, float* %b, align 4
  %multmp = fmul float %a12, %b13
  %"converted LHS to type FLOAT14" = sitofp i32 %i11 to float
  %multmp15 = fmul float %"converted LHS to type FLOAT14", %multmp
  %c16 = load float, float* %c, align 4
  store float %multmp15, float* %c, align 4
  %c17 = load float, float* %c, align 4
  %dictmp = fdiv float 4.000000e+00, %c17
  %d18 = load float, float* %d, align 4
  store float %dictmp, float* %d, align 4
  %flag19 = load i1, i1* %flag, align 1
  %ifconditionS = icmp ne i1 %flag19, false
  br i1 %ifconditionS, label %then, label %"else bock"

"after loop":                                     ; preds = %condition
  %PI32 = load float, float* %PI, align 4
  ret float %PI32

then:                                             ; preds = %"while loop"
  %PI20 = load float, float* %PI, align 4
  %d21 = load float, float* %d, align 4
  %addtmp22 = fadd float %PI20, %d21
  %PI23 = load float, float* %PI, align 4
  store float %addtmp22, float* %PI, align 4
  br label %"after if block"

"else bock":                                      ; preds = %"while loop"
  %PI24 = load float, float* %PI, align 4
  %d25 = load float, float* %d, align 4
  %subtmp = fsub float %PI24, %d25
  %PI26 = load float, float* %PI, align 4
  store float %subtmp, float* %PI, align 4
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi float [ %addtmp22, %then ], [ %subtmp, %"else bock" ]
  %flag27 = load i1, i1* %flag, align 1
  %"not temp" = xor i1 %flag27, true
  %flag28 = load i1, i1* %flag, align 1
  store i1 %"not temp", i1* %flag, align 1
  %i29 = load i32, i32* %i, align 4
  %addtmp30 = add i32 %i29, 2
  %i31 = load i32, i32* %i, align 4
  store i32 %addtmp30, i32* %i, align 4
  br label %condition
}
