/*
                  printf :: Func(Ptr, ..., Void)
                      i1 :: Int32
                    op.= :: Call(Func(*T0, *T0, Bool), n, i1)
                    op.= :: Bool
                      if :: m
                      i2 :: Int32
                    op.% :: Call(Func(*T0, *T0, *T0), n, i2)
                      i0 :: Int32
                  op.=.1 :: Call(Func(*T0, *T0, Bool), op.%, i0)
                  op.=.1 :: Bool
                    i2.1 :: Int32
                    op./ :: Call(Func(*T0, *T0, *T0), n, i2.1)
                    i1.1 :: Int32
                    op.+ :: Call(Func(*T0, *T0, *T0), m, i1.1)
            call.collatz :: Call(collatz, op./, op.+)
                    if.1 :: call.collatz
                      i3 :: Int32
                    op.* :: Call(Func(*T0, *T0, *T0), i3, n)
                    i1.2 :: Int32
                  op.+.1 :: Call(Func(*T0, *T0, *T0), op.*, i1.2)
                    i1.3 :: Int32
                  op.+.2 :: Call(Func(*T0, *T0, *T0), m, i1.3)
          call.collatz.1 :: Call(collatz, op.+.1, op.+.2)
                    if.1 :: call.collatz.1
                      if :: if.1
                 collatz :: Func(n, m, if)
                  op.=.2 :: Call(Func(*T0, *T0, Bool), a, b)
                  op.=.2 :: Bool
            str."done\n" :: Ptr
             call.printf :: Call(printf, str."done\n")
                    if.2 :: call.printf
 str."collatz %d = %d\n" :: Ptr
                    i0.1 :: Int32
          call.collatz.2 :: Call(collatz, a, i0.1)
           call.printf.1 :: Call(printf, str."collatz %d = %d\n", a, call.collatz.2)
                    i1.4 :: Int32
                  op.+.3 :: Call(Func(*T0, *T0, *T0), a, i1.4)
       call.collatz_loop :: Call(collatz_loop, op.+.3, b)
                    if.2 :: call.collatz_loop
            collatz_loop :: Func(a, b, if.2)
                    i1.5 :: Int32
                     i30 :: Int32
     call.collatz_loop.1 :: Call(collatz_loop, i1.5, i30)
                    i0.2 :: Int32
                    main :: Func(i0.2)
*/
