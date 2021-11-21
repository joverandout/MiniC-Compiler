; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

declare float @print_float(float)

define float @unary(i32 %n, float %m) {
block:
  %sum = alloca float, align 4
  %result = alloca float, align 4
  %m2 = alloca float, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  store float %m, float* %m2, align 4
  %sum3 = load float, float* %sum, align 4
  store float 0.000000e+00, float* %sum, align 4
  %n4 = load i32, i32* %n1, align 4
  %m5 = load float, float* %m2, align 4
  %"converted LHS to type FLOAT" = sitofp i32 %n4 to float
  %addtmp = fadd float %"converted LHS to type FLOAT", %m5
  %result6 = load float, float* %result, align 4
  store float %addtmp, float* %result, align 4
  %result7 = load float, float* %result, align 4
  %calltmp = call float @print_float(float %result7)
  %sum8 = load float, float* %sum, align 4
  %result9 = load float, float* %result, align 4
  %addtmp10 = fadd float %sum8, %result9
  %sum11 = load float, float* %sum, align 4
  store float %addtmp10, float* %sum, align 4
  %n12 = load i32, i32* %n1, align 4
  %m13 = load float, float* %m2, align 4
  %"neg temp" = fneg float %m13
  %"converted LHS to type FLOAT14" = sitofp i32 %n12 to float
  %addtmp15 = fadd float %"converted LHS to type FLOAT14", %"neg temp"
  %result16 = load float, float* %result, align 4
  store float %addtmp15, float* %result, align 4
  %result17 = load float, float* %result, align 4
  %calltmp18 = call float @print_float(float %result17)
  %sum19 = load float, float* %sum, align 4
  %result20 = load float, float* %result, align 4
  %addtmp21 = fadd float %sum19, %result20
  %sum22 = load float, float* %sum, align 4
  store float %addtmp21, float* %sum, align 4
  %n23 = load i32, i32* %n1, align 4
  %m24 = load float, float* %m2, align 4
  %"neg temp25" = fneg float %m24
  %"neg temp26" = fneg float %"neg temp25"
  %"converted LHS to type FLOAT27" = sitofp i32 %n23 to float
  %addtmp28 = fadd float %"converted LHS to type FLOAT27", %"neg temp26"
  %result29 = load float, float* %result, align 4
  store float %addtmp28, float* %result, align 4
  %result30 = load float, float* %result, align 4
  %calltmp31 = call float @print_float(float %result30)
  %sum32 = load float, float* %sum, align 4
  %result33 = load float, float* %result, align 4
  %addtmp34 = fadd float %sum32, %result33
  %sum35 = load float, float* %sum, align 4
  store float %addtmp34, float* %sum, align 4
  %n36 = load i32, i32* %n1, align 4
  %"int->float" = sitofp i32 %n36 to float
  %"neg temp37" = fneg float %"int->float"
  %"int->float38" = fptosi float %"neg temp37" to i32
  %m39 = load float, float* %m2, align 4
  %"neg temp40" = fneg float %m39
  %"converted LHS to type FLOAT41" = sitofp i32 %"int->float38" to float
  %addtmp42 = fadd float %"converted LHS to type FLOAT41", %"neg temp40"
  %result43 = load float, float* %result, align 4
  store float %addtmp42, float* %result, align 4
  %result44 = load float, float* %result, align 4
  %calltmp45 = call float @print_float(float %result44)
  %sum46 = load float, float* %sum, align 4
  %result47 = load float, float* %result, align 4
  %addtmp48 = fadd float %sum46, %result47
  %sum49 = load float, float* %sum, align 4
  store float %addtmp48, float* %sum, align 4
  %sum50 = load float, float* %sum, align 4
  ret float %sum50
}
