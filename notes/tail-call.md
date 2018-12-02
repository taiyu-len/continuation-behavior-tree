Currently to enable tail-call optimization in behavior tree the compiler flags
required are  
`-O3 -flto`

works for inverters, but not repeaters or sequences  
`-O1 -flto -foptimize-sibling-calls`

need to improve code to not require lto



some results of trying different compiler flags and features.
```
// 5 repeater
///////////////////////////
// using continuation by value
// g++     -Os      : 1041
// using continuation by rvalue reference
// g++     -Og -ggdb: 3792
// g++     -O3      : 745
// g++     -Os      : 753
// clang++ -O3      : 769
// clang++ -Os      : 745
// using improved unique_function
// g++     -O3      : 505
// g++     -Os      : 545
// clang++ -O3      : 529
// clang++ -Os      : 521
// using -flto
// g++     -O3      : 425
```

```c++
// 5*5 repeater
////////////////////////////
// using continuation by value
// g++     -Os      : 4641
// using continuation by rvalue reference
// g++     -Og -ggdb: 20672
// g++     -O3      : 3225
// g++     -Os      : 3153
// clang++ -O3      : 3489
// clang++ -Os      : 3305
// using improved unique_function
// g++     -O3      : 1945
// g++     -Os      : 1985
// clang++ -O3      : 2289
// clang++ -Os      : 2281
// using -flto
// g++     -O3      : 1465
```
