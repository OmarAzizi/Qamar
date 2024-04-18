// Testing functions and recursion
fun fib(n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

var start = clock(); // using the native function `clock` to benchmark
print fib(35);
print clock() - start;
