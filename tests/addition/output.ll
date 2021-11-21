; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

define i32 @addition(i32 %n, i32 %m) {
block:
  %result = alloca i32, align 4
  %m2 = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  store i32 %m, i32* %m2, align 4
  %n3 = load i32, i32* %n1, align 4
  %m4 = load i32, i32* %m2, align 4
  %addtmp = add i32 %n3, %m4
  %result5 = load i32, i32* %result, align 4
  store i32 %addtmp, i32* %result, align 4
  %n6 = load i32, i32* %n1, align 4
  %0 = sitofp i32 %n6 to float
  %cmptmp = fcmp ueq float %0, 4.000000e+00
  %booltmp = uitofp i1 %cmptmp to double
  %ifcondition = fcmp one double %booltmp, 0.000000e+00
  br i1 %ifcondition, label %then, label %"else bock"

then:                                             ; preds = %block
  %n7 = load i32, i32* %n1, align 4
  %m8 = load i32, i32* %m2, align 4
  %addtmp9 = add i32 %n7, %m8
  %calltmp = call i32 @print_int(i32 %addtmp9)
  br label %"after if block"

"else bock":                                      ; preds = %block
  %n10 = load i32, i32* %n1, align 4
  %m11 = load i32, i32* %m2, align 4
  %multmp = mul i32 %n10, %m11
  %calltmp12 = call i32 @print_int(i32 %multmp)
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi i32 [ %calltmp, %then ], [ %calltmp12, %"else bock" ]
  %result13 = load i32, i32* %result, align 4
  ret i32 %result13
}
