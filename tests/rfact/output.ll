; ModuleID = 'mini-c'
source_filename = "mini-c"

define i32 @multiplyNumbers(i32 %n) {
block:
  %result = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  %result2 = load i32, i32* %result, align 4
  store i32 0, i32* %result, align 4
  %n3 = load i32, i32* %n1, align 4
  %cmptmp = icmp uge i32 %n3, 1
  %booltmp = uitofp i1 %cmptmp to double
  %ifcondition = fcmp one double %booltmp, 0.000000e+00
  br i1 %ifcondition, label %then, label %"else bock"

then:                                             ; preds = %block
  %n4 = load i32, i32* %n1, align 4
  %n5 = load i32, i32* %n1, align 4
  %subtmp = sub i32 %n5, 1
  %calltmp = call i32 @multiplyNumbers(i32 %subtmp)
  %multmp = mul i32 %n4, %calltmp
  %result6 = load i32, i32* %result, align 4
  store i32 %multmp, i32* %result, align 4
  br label %"after if block"

"else bock":                                      ; preds = %block
  %result7 = load i32, i32* %result, align 4
  store i32 1, i32* %result, align 4
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi i32 [ %multmp, %then ], [ 1, %"else bock" ]
  %result8 = load i32, i32* %result, align 4
  ret i32 %result8
}

define i32 @rfact(i32 %n) {
block:
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  %n2 = load i32, i32* %n1, align 4
  %calltmp = call i32 @multiplyNumbers(i32 %n2)
  ret i32 %calltmp
}
