; RUN: topt < %s -SCCP | FileCheck %s

define i32 @test1(i32 %i0, i32 %j0) {
BB1:
    br label %BB2
BB2:
    %var1 = add i32 2, 3
    %var2 = add i32 6, 4
    %kcond = icmp slt i32 %var1, %var2
    br i1 %kcond, label %BB3, label %BB4
BB3:
    %var1_2 = add i32 %var1, 5
    br label %BB5
BB4:
    %var2_2 = add i32 %var2, 5
    br label %BB5
BB5:
    %var3 = phi i32 [ %var1_2, %BB3 ], [ %var2_2, %BB4 ]
    ret i32 %var3
}
; CHECK-LABEL:  BB2:
; CHECK:        br i1 true, label %BB3, label %BB4
; CHECK-LABEL:  BB3:
; CHECK:        br label %BB5
; CHECK-LABEL:  BB4:
; CHECK:        br label %BB5
; CHECK-LABEL:  BB5:
; CHECK:        ret i32 10

define i32 @test2(i32 %i0, i32 %j0) {
BB1:
    br label %BB2
BB2:
    %j2 = phi i32 [ %j4, %BB7 ], [ 1, %BB1 ]
    %k2 = phi i32 [ %k4, %BB7 ], [ 0, %BB1 ]
    %kcond = icmp slt i32 %k2, 100
    br i1 %kcond, label %BB3, label %BB4
BB3:
    %jcond = icmp slt i32 %j2, 20
    br i1 %jcond, label %BB5, label %BB6
BB4:
    ret i32 %j2
BB5:
    %k3 = add i32 %k2, 1
    br label %BB7
BB6:
    %k5 = add i32 %k2, 1
    br label %BB7
BB7:
    %j4 = phi i32 [ 1, %BB5 ], [ %k2, %BB6 ]
    %k4 = phi i32 [ %k3, %BB5 ], [ %k5, %BB6 ]
    br label %BB2
}

; CHECK-LABEL:  BB2:
; CHECK:        %k2 = phi i32 [ %k4, %BB7 ], [ 0, %BB1 ]
; CHECK:        %kcond = icmp slt i32 %k2, 100
; CHECK:        br i1 %kcond, label %BB3, label %BB4
; CHECK-LABEL:  BB3:
; CHECK:        br i1 true, label %BB5, label %BB6
; CHECK-LABEL:  BB4:
; CHECK:        ret i32 1
; CHECK-LABEL:  BB5:
; CHECK:        %k3 = add i32 %k2, 1
; CHECK:        br label %BB7
; CHECK-LABEL:  BB6:
; CHECK:        br label %BB7
; CHECK-LABEL:  BB7:
; CHECK:        %k4 = phi i32 [ %k3, %BB5 ], [ undef, %BB6 ]
; CHECK:        br label %BB2
