; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

define i32 @fibonacci(i32 %n) {
block:
  %total = alloca i32, align 4
  %c = alloca i32, align 4
  %next = alloca i32, align 4
  %second = alloca i32, align 4
  %first = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, i32* %n1, align 4
  %n2 = load i32, i32* %n1, align 4
  %calltmp = call i32 @print_int(i32 %n2)
  %first3 = load i32, i32* %first, align 4
  store i32 0, i32* %first, align 4
  %second4 = load i32, i32* %second, align 4
  store i32 1, i32* %second, align 4
  %c5 = load i32, i32* %c, align 4
  store i32 1, i32* %c, align 4
  %total6 = load i32, i32* %total, align 4
  store i32 0, i32* %total, align 4
  br label %condition

condition:                                        ; preds = %"after if block", %block
  %c7 = load i32, i32* %c, align 4
  %n8 = load i32, i32* %n1, align 4
  %cmptemp = icmp ult i32 %c7, %n8
  %booltmp = uitofp i1 %cmptemp to double
  %"loop cond" = fcmp one double %booltmp, 0.000000e+00
  br i1 %"loop cond", label %"while loop", label %"after loop"

"while loop":                                     ; preds = %condition
  %c9 = load i32, i32* %c, align 4
  %cmptmp = icmp ule i32 %c9, 1
  %booltmp10 = uitofp i1 %cmptmp to double
  %ifcondition = fcmp one double %booltmp10, 0.000000e+00
  br i1 %ifcondition, label %then, label %"else bock"

"after loop":                                     ; preds = %condition
  %total29 = load i32, i32* %total, align 4
  %calltmp30 = call i32 @print_int(i32 %total29)
  %total31 = load i32, i32* %total, align 4
  ret i32 %total31

then:                                             ; preds = %"while loop"
  %c11 = load i32, i32* %c, align 4
  %next12 = load i32, i32* %next, align 4
  store i32 %c11, i32* %next, align 4
  br label %"after if block"

"else bock":                                      ; preds = %"while loop"
  %first13 = load i32, i32* %first, align 4
  %second14 = load i32, i32* %second, align 4
  %addtmp = add i32 %first13, %second14
  %next15 = load i32, i32* %next, align 4
  store i32 %addtmp, i32* %next, align 4
  %second16 = load i32, i32* %second, align 4
  %first17 = load i32, i32* %first, align 4
  store i32 %second16, i32* %first, align 4
  %next18 = load i32, i32* %next, align 4
  %second19 = load i32, i32* %second, align 4
  store i32 %next18, i32* %second, align 4
  br label %"after if block"

"after if block":                                 ; preds = %"else bock", %then
  %"then tmp" = phi i32 [ %c11, %then ], [ %next18, %"else bock" ]
  %next20 = load i32, i32* %next, align 4
  %calltmp21 = call i32 @print_int(i32 %next20)
  %c22 = load i32, i32* %c, align 4
  %addtmp23 = add i32 %c22, 1
  %c24 = load i32, i32* %c, align 4
  store i32 %addtmp23, i32* %c, align 4
  %total25 = load i32, i32* %total, align 4
  %next26 = load i32, i32* %next, align 4
  %addtmp27 = add i32 %total25, %next26
  %total28 = load i32, i32* %total, align 4
  store i32 %addtmp27, i32* %total, align 4
  br label %condition
}
