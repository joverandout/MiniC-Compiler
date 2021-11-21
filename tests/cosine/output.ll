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
  %x11 = load float, float* %x1, align 4
  %n12 = load float, float* %n, align 4
  %n13 = load float, float* %n, align 4
  %addtmp = fadd float %n13, 1.000000e+00
  %dictmp = fdiv float %n12, %addtmp
  %dictmp14 = fdiv float %x11, %dictmp
  %multmp = fmul float %x10, %dictmp14
  %multmp15 = fmul float %term9, %multmp
  %term16 = load float, float* %term, align 4
  store float %multmp15, float* %term, align 4
  %cos17 = load float, float* %cos, align 4
  %alt18 = load float, float* %alt, align 4
  %term19 = load float, float* %term, align 4
  %multmp20 = fmul float %alt18, %term19
  %addtmp21 = fadd float %cos17, %multmp20
  %alt22 = load float, float* %alt, align 4
  %"neg temp" = fneg float %alt22
  %alt23 = load float, float* %alt, align 4
  store float %"neg temp", float* %alt, align 4
  %n24 = load float, float* %n, align 4
  %addtmp25 = fadd float %n24, 2.000000e+00
  %n26 = load float, float* %n, align 4
  store float %addtmp25, float* %n, align 4
  %term27 = load float, float* %term, align 4
  %calltmp = call float @print_float(float %term27)
  br label %condition

"after loop":                                     ; preds = %condition
  %cos28 = load float, float* %cos, align 4
  %calltmp29 = call float @print_float(float %cos28)
  %cos30 = load float, float* %cos, align 4
  ret float %cos30
}
