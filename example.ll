define dso_local i32 @foo(i32 %0, i32 %1, i32 %2,
i32 %3) {
%5 = sub i32 %2, %3
%6 = mul i32 %5, %1
%7 = add i32 %1, %6
%8 = mul i32 %2, %7
%9 = add i32 %0, %8
%10 = sub i32 %2, %3
%11 = mul i32 %10, %1
%12 = add i32 %9, %11
ret i32 %12
}

define i32 @test1() {
t0BB1:
    %t00 = add i32 1, 1
    ret i32 %t00
}


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