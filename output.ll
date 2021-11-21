; ModuleID = 'mini-c'
source_filename = "mini-c"

@test = common global i32 0
@f = common global float 0.000000e+00
@b = common global i1 false

declare i32 @print_int(i32)

define i32 @While(i32 %n) {
block:
  %result = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  %result2 = load i32, i32* %result, align 4
  store i32 0, i32* %result, align 4
  %test = load i32, i32* @test, align 4
  %calltmp = call i32 @print_int(i32 %test)
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
  br label %condition

"after loop":                                     ; preds = %condition
  %result6 = load i32, i32* %result, align 4
  ret i32 %result6
}
