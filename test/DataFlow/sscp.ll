; RUN: topt < %s -SSCP | FileCheck %s

define i32 @test1(i1 %B) {
    br i1 %B, label %BB1, label %BB2
BB1:        ; preds = %0
    %Val = add i32 0, 0        ; <i32> [#uses=1]
    br label %BB3
BB2:        ; preds = %0
    br label %BB3
BB3:        ; preds = %BB2, %BB1
    %Ret = phi i32 [ %Val, %BB1 ], [ 1, %BB2 ]        ; <i32> [#uses=1]
    ret i32 %Ret
}

; CHECK-LABEL:  BB1:
; CHECK-NOT:    add
; CHECK:        br label %BB3
; CHECK-LABEL:  BB3:
; CHECK:        %Ret = phi i32 [ 0, %BB1 ], [ 1, %BB2 ]