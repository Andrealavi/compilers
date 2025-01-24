target triple = "x86_64-pc-linux-gnu"

define i32 @f(i32 %x) {
entry:
  %i = alloca i32, align 4
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store i32 1, ptr %i, align 4
  br label %condition

condition:                                        ; preds = %update, %entry
  %i2 = load i32, ptr %i, align 4
  %cmplt = icmp slt i32 %i2, 10
  br i1 %cmplt, label %loop, label %exit

loop:                                             ; preds = %condition
  %x3 = load i32, ptr %x1, align 4
  %i4 = load i32, ptr %i, align 4
  %sum = add nsw i32 %x3, %i4
  store i32 %sum, ptr %x1, align 4
  br label %update

update:                                           ; preds = %loop
  %i5 = load i32, ptr %i, align 4
  %sum6 = add nsw i32 %i5, 1
  store i32 %sum6, ptr %i, align 4
  br label %condition

exit:                                             ; preds = %condition
  %x7 = load i32, ptr %x1, align 4
  %cmpgt = icmp sgt i32 %x7, 5
  br i1 %cmpgt, label %expr1, label %test2

expr1:                                            ; preds = %exit
  store i32 150, ptr %x1, align 4
  br label %exitblock

test2:                                            ; preds = %exit
  br i1 true, label %expr2, label %exitblock

expr2:                                            ; preds = %test2
  %x8 = load i32, ptr %x1, align 4
  br label %exitblock

exitblock:                                        ; preds = %expr2, %test2, %expr1
  %condval = phi i32 [ 150, %expr1 ], [ %x8, %expr2 ], [ 0, %test2 ]
  %x9 = load i32, ptr %x1, align 4
  ret i32 %x9
}

define i32 @main() {
entry:
  %callfun = call i32 @f(i32 1)
  ret i32 %callfun
}

