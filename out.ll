; ModuleID = './test/DataFlow/sccp.ll'
source_filename = "./test/DataFlow/sccp.ll"

define i32 @test1(i32 %i0, i32 %j0) {
BB1:
  br label %BB2

BB2:                                              ; preds = %BB1
  br i1 true, label %BB3, label %BB4

BB3:                                              ; preds = %BB2
  br label %BB5

BB4:                                              ; preds = %BB2
  br label %BB5

BB5:                                              ; preds = %BB4, %BB3
  ret i32 10
}

define i32 @test2(i32 %i0, i32 %j0) {
BB1:
  br label %BB2

BB2:                                              ; preds = %BB7, %BB1
  %k2 = phi i32 [ %k4, %BB7 ], [ 0, %BB1 ]
  %kcond = icmp slt i32 %k2, 100
  br i1 %kcond, label %BB3, label %BB4

BB3:                                              ; preds = %BB2
  br i1 true, label %BB5, label %BB6

BB4:                                              ; preds = %BB2
  ret i32 1

BB5:                                              ; preds = %BB3
  %k3 = add i32 %k2, 1
  br label %BB7

BB6:                                              ; preds = %BB3
  br label %BB7

BB7:                                              ; preds = %BB6, %BB5
  %k4 = phi i32 [ %k3, %BB5 ], [ undef, %BB6 ]
  br label %BB2
}
