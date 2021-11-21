; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

define i32 @addNumbers(i32 %n) {
block:
  %result = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  %result2 = load i32, i32* %result, align 4
  store i32 0, i32* %result, align 4
  %n3 = load i32, i32* %n1, align 4
  %0 = sitofp i32 %n3 to float
  %cmptmp = fcmp une float %0, 0.000000e+00
  %booltmp = uitofp i1 %cmptmp to double
  %ifcondition = fcmp one double %booltmp, 0.000000e+00
  br i1 %ifcondition, label %then, label %"else bock"

then:                                             ; preds = %block
  %n4 = load i32, i32* %n1, align 4
  %n5 = load i32, i32* %n1, align 4
  %subtmp = sub i32 %n5, 1
  %calltmp = call i32 @addNumbers(i32 %subtmp)
  %addtmp = add i32 %n4, %calltmp
  %result6 = load i32, i32* %result, align 4
  store i32 %addtmp, i32* %result, align 4
  br label %"after if block"

"else bock":                                      ; preds = %block
  %n7 = load i32, i32* %n1, align 4
  %result8 = load i32, i32* %result, align 4
  store i32 %n7, i32* %result, align 4
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi i32 [ %addtmp, %then ], [ %n7, %"else bock" ]
  %result9 = load i32, i32* %result, align 4
  %calltmp10 = call i32 @print_int(i32 %result9)
  %result11 = load i32, i32* %result, align 4
  ret i32 %result11
}

define i32 @recursion_driver(i32 %num) {
block:
  %num1 = alloca i32, align 4
  store i32 %num, i32* %num1, align 4
  %num2 = load i32, i32* %num1, align 4
  %calltmp = call i32 @addNumbers(i32 %num2)
  ret i32 %calltmp
}
