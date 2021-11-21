; ModuleID = 'mini-c'
source_filename = "mini-c"

define i1 @palindrome(i32 %number) {
block:
  %result = alloca i1, align 1
  %rmndr = alloca i32, align 4
  %rev = alloca i32, align 4
  %t = alloca i32, align 4
  %number1 = alloca i32, align 4
  store i32 %number, i32* %number1, align 4
  %rev2 = load i32, i32* %rev, align 4
  store i32 0, i32* %rev, align 4
  %result3 = load i1, i1* %result, align 1
  store i1 false, i1* %result, align 1
  %number4 = load i32, i32* %number1, align 4
  %t5 = load i32, i32* %t, align 4
  store i32 %number4, i32* %t, align 4
  br label %condition

condition:                                        ; preds = %"while loop", %block
  %number6 = load i32, i32* %number1, align 4
  %remtemp = srem i32 %number6, 10
  %rmndr7 = load i32, i32* %rmndr, align 4
  store i32 %remtemp, i32* %rmndr, align 4
  %rev8 = load i32, i32* %rev, align 4
  %multmp = mul i32 %rev8, 10
  %rmndr9 = load i32, i32* %rmndr, align 4
  %addtmp = add i32 %multmp, %rmndr9
  %rev10 = load i32, i32* %rev, align 4
  store i32 %addtmp, i32* %rev, align 4
  %number11 = load i32, i32* %number1, align 4
  %dictmp = sdiv i32 %number11, 10
  %number12 = load i32, i32* %number1, align 4
  store i32 %dictmp, i32* %number1, align 4
  %number13 = load i32, i32* %number1, align 4
  %cmptemp = icmp ugt i32 %number13, 0
  %booltmp = uitofp i1 %cmptemp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %number14 = load i32, i32* %number1, align 4
  %remtemp15 = srem i32 %number14, 10
  %rmndr16 = load i32, i32* %rmndr, align 4
  store i32 %remtemp15, i32* %rmndr, align 4
  %rev17 = load i32, i32* %rev, align 4
  %multmp18 = mul i32 %rev17, 10
  %rmndr19 = load i32, i32* %rmndr, align 4
  %addtmp20 = add i32 %multmp18, %rmndr19
  %rev21 = load i32, i32* %rev, align 4
  store i32 %addtmp20, i32* %rev, align 4
  %number22 = load i32, i32* %number1, align 4
  %dictmp23 = sdiv i32 %number22, 10
  %number24 = load i32, i32* %number1, align 4
  store i32 %dictmp23, i32* %number1, align 4
  br label %condition

"after loop":                                     ; preds = %condition
  %t25 = load i32, i32* %t, align 4
  %rev26 = load i32, i32* %rev, align 4
  %0 = sitofp i32 %t25 to float
  %1 = sitofp i32 %rev26 to float
  %cmptmp = fcmp ueq float %0, %1
  %booltmp27 = uitofp i1 %cmptmp to double
  %ifcondition = fcmp one double %booltmp27, 0.000000e+00
  br i1 %ifcondition, label %then, label %"else bock"

then:                                             ; preds = %"after loop"
  %result28 = load i1, i1* %result, align 1
  store i1 true, i1* %result, align 1
  br label %"after if block"

"else bock":                                      ; preds = %"after loop"
  %result29 = load i1, i1* %result, align 1
  store i1 false, i1* %result, align 1
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi i1 [ true, %then ], [ false, %"else bock" ]
  %result30 = load i1, i1* %result, align 1
  ret i1 %result30
}
