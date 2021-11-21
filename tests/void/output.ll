; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

define void @"("() {
block:
  %result = alloca i32, align 4
  %result1 = load i32, i32* %result, align 4
  store i32 0, i32* %result, align 4
  %result2 = load i32, i32* %result, align 4
  %calltmp = call i32 @print_int(i32 %result2)
  br label %condition

condition:                                        ; preds = %"while loop", %block
  %result3 = load i32, i32* %result, align 4
  %cmptemp = icmp ult i32 %result3, 10
  %booltmp = uitofp i1 %cmptemp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %result4 = load i32, i32* %result, align 4
  %addtmp = add i32 %result4, 1
  %result5 = load i32, i32* %result, align 4
  store i32 %addtmp, i32* %result, align 4
  %result6 = load i32, i32* %result, align 4
  %calltmp7 = call i32 @print_int(i32 %result6)
  br label %condition

"after loop":                                     ; preds = %condition
  ret void
}
