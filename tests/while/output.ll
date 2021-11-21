; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

define i32 @While(i32 %n) {
block:
  %result = alloca i32, align 4
  %b = alloca i1, align 1
  %f = alloca float, align 4
  %test = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  %test2 = load i32, i32* %test, align 4
  store i32 12, i32* %test, align 4
  %result3 = load i32, i32* %result, align 4
  store i32 0, i32* %result, align 4
  %test4 = load i32, i32* %test, align 4
  %calltmp = call i32 @print_int(i32 %test4)
  br label %condition

condition:                                        ; preds = %"while loop", %block
  %result5 = load i32, i32* %result, align 4
  %cmptemp = icmp ult i32 %result5, 10
  %booltmp = uitofp i1 %cmptemp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %result6 = load i32, i32* %result, align 4
  %addtmp = add i32 %result6, 1
  %result7 = load i32, i32* %result, align 4
  store i32 %addtmp, i32* %result, align 4
  br label %condition

"after loop":                                     ; preds = %condition
  %result8 = load i32, i32* %result, align 4
  ret i32 %result8
}
