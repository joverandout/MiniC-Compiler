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
  %cmptemp = icmp ugt i32 %number6, 0
  %booltmp = uitofp i1 %cmptemp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %number7 = load i32, i32* %number1, align 4
  %remtemp = srem i32 %number7, 10
  %rmndr8 = load i32, i32* %rmndr, align 4
  store i32 %remtemp, i32* %rmndr, align 4
  %rev9 = load i32, i32* %rev, align 4
  %multmp = mul i32 %rev9, 10
  %rmndr10 = load i32, i32* %rmndr, align 4
  %addtmp = add i32 %multmp, %rmndr10
  %rev11 = load i32, i32* %rev, align 4
  store i32 %addtmp, i32* %rev, align 4
  %number12 = load i32, i32* %number1, align 4
  %dictmp = sdiv i32 %number12, 10
  %number13 = load i32, i32* %number1, align 4
  store i32 %dictmp, i32* %number1, align 4
  br label %condition

"after loop":                                     ; preds = %condition
  %t14 = load i32, i32* %t, align 4
  %rev15 = load i32, i32* %rev, align 4
  %0 = sitofp i32 %t14 to float
  %1 = sitofp i32 %rev15 to float
  %cmptmp = fcmp ueq float %0, %1
  %booltmp16 = uitofp i1 %cmptmp to double
  %ifcondition = fcmp one double %booltmp16, 0.000000e+00
  br i1 %ifcondition, label %then, label %"else bock"

then:                                             ; preds = %"after loop"
  %result17 = load i1, i1* %result, align 1
  store i1 true, i1* %result, align 1
  br label %"after if block"

"else bock":                                      ; preds = %"after loop"
  %result18 = load i1, i1* %result, align 1
  store i1 false, i1* %result, align 1
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi i1 [ true, %then ], [ false, %"else bock" ]
  %result19 = load i1, i1* %result, align 1
  ret i1 %result19
}
