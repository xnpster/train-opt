Remove
Remove
Remove
Remove
Remove
Remove
Remove
Remove
Remove
Remove
Remove
Remove
Remove
Remove
; ModuleID = 'example.ll'
source_filename = "example.ll"

define dso_local i32 @foo(i32 %0, i32 %1, i32 %2, i32 %3) {
  %5 = add i32 undef, undef
  ret i32 %5
}

define i32 @test1() {
t0BB0:
  br i1 true, label %t0BB1, label %t0BB2

t0BB1:                                            ; preds = %t0BB0
  br label %t0BB3

t0BB2:                                            ; preds = %t0BB0
  br label %t0BB3

t0BB3:                                            ; preds = %t0BB2, %t0BB1
  %t03 = phi i32 [ 3, %t0BB1 ], [ undef, %t0BB2 ]
  ret i32 %t03
}

define i32 @test2(i32 %i0, i32 %j0) {
BB1:
  br label %BB2

BB2:                                              ; preds = %BB7, %BB1
  %j2 = phi i32 [ %j4, %BB7 ], [ 1, %BB1 ]
  %k2 = phi i32 [ %k4, %BB7 ], [ 0, %BB1 ]
  %kcond = icmp slt i32 %k2, 100
  br i1 %kcond, label %BB3, label %BB4

BB3:                                              ; preds = %BB2
  %jcond = icmp slt i32 %j2, 20
  br i1 %jcond, label %BB5, label %BB6

BB4:                                              ; preds = %BB2
  ret i32 %j2

BB5:                                              ; preds = %BB3
  br label %BB7

BB6:                                              ; preds = %BB3
  br label %BB7

BB7:                                              ; preds = %BB6, %BB5
  %j4 = phi i32 [ 1, %BB5 ], [ %k2, %BB6 ]
  %k4 = phi i32 [ 1, %BB5 ], [ undef, %BB6 ]
  br label %BB2
}
