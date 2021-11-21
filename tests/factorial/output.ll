; ModuleID = 'mini-c'
source_filename = "mini-c"

define i32 @factorial(i32 %n) {
block:
  %factorial = alloca i32, align 4
  %i = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  %factorial2 = load i32, i32* %factorial, align 4
  store i32 1, i32* %factorial, align 4
  %i3 = load i32, i32* %i, align 4
  store i32 1, i32* %i, align 4
  br label %condition

condition:                                        ; preds = %"while loop", %block
  %i4 = load i32, i32* %i, align 4
  %n5 = load i32, i32* %n1, align 4
  %cmptmp = icmp ule i32 %i4, %n5
  %booltmp = uitofp i1 %cmptmp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %factorial6 = load i32, i32* %factorial, align 4
  %i7 = load i32, i32* %i, align 4
  %multmp = mul i32 %factorial6, %i7
  %factorial8 = load i32, i32* %factorial, align 4
  store i32 %multmp, i32* %factorial, align 4
  %i9 = load i32, i32* %i, align 4
  %addtmp = add i32 %i9, 1
  %i10 = load i32, i32* %i, align 4
  store i32 %addtmp, i32* %i, align 4
  br label %condition

"after loop":                                     ; preds = %condition
  %factorial11 = load i32, i32* %factorial, align 4
  ret i32 %factorial11
}
