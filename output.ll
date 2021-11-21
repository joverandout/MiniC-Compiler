; ModuleID = 'mini-c'
source_filename = "mini-c"

declare float @print_float(float)

define float @cosine(float %x) {
block:
  %alt = alloca float, align 4
  %eps = alloca float, align 4
  %term = alloca float, align 4
  %n = alloca float, align 4
  %cos = alloca float, align 4
  %x1 = alloca float, align 4
  store float %x, float* %x1, align 4
  %eps2 = load float, float* %eps, align 4
  store float 0x3EB0C6F7A0000000, float* %eps, align 4
  %n3 = load float, float* %n, align 4
  store float 1.000000e+00, float* %n, align 4
  %cos4 = load float, float* %cos, align 4
  store float 1.000000e+00, float* %cos, align 4
  %term5 = load float, float* %term, align 4
  store float 1.000000e+00, float* %term, align 4
  %alt6 = load float, float* %alt, align 4
  store float -1.000000e+00, float* %alt, align 4
  br label %condition

condition:                                        ; preds = %"while loop", %block
  %term7 = load float, float* %term, align 4
  %eps8 = load float, float* %eps, align 4
  %cmptemp = fcmp ugt float %term7, %eps8
  %booltmp = uitofp i1 %cmptemp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %term9 = load float, float* %term, align 4
  %x10 = load float, float* %x1, align 4
  %multmp = fmul float %term9, %x10
  %x11 = load float, float* %x1, align 4
  %multmp12 = fmul float %multmp, %x11
  %n13 = load float, float* %n, align 4
  %dictmp = fdiv float %multmp12, %n13
  %n14 = load float, float* %n, align 4
  %addtmp = fadd float %n14, 1.000000e+00
  %dictmp15 = fdiv float %dictmp, %addtmp
  %term16 = load float, float* %term, align 4
  store float %dictmp15, float* %term, align 4
  %cos17 = load float, float* %cos, align 4
  %alt18 = load float, float* %alt, align 4
  %term19 = load float, float* %term, align 4
  %multmp20 = fmul float %alt18, %term19
  %addtmp21 = fadd float %cos17, %multmp20
  %cos22 = load float, float* %cos, align 4
  store float %addtmp21, float* %cos, align 4
  %alt23 = load float, float* %alt, align 4
  %"neg temp" = fneg float %alt23
  %alt24 = load float, float* %alt, align 4
  store float %"neg temp", float* %alt, align 4
  %n25 = load float, float* %n, align 4
  %addtmp26 = fadd float %n25, 2.000000e+00
  %n27 = load float, float* %n, align 4
  store float %addtmp26, float* %n, align 4
  %term28 = load float, float* %term, align 4
  %calltmp = call float @print_float(float %term28)
  br label %condition

"after loop":                                     ; preds = %condition
  %cos29 = load float, float* %cos, align 4
  %calltmp30 = call float @print_float(float %cos29)
  %cos31 = load float, float* %cos, align 4
  ret float %cos31
}
