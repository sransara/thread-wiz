from concurrent.futures import ThreadPoolExecutor
import time
import os

def fibonacci(n):
    if n <= 0:
        return 0
    elif n == 1:
        return 1
    else:
        return fibonacci(n-1) + fibonacci(n-2)

def calculate_fibonacci(n):
    start_time = time.time()
    result = fibonacci(n)
    end_time = time.time()
    print(f"Fibonacci({n}) = {result}, calculated in {end_time - start_time:.2f} seconds")
    return result

if __name__ == "__main__":
    print(f"Process ID: {os.getpid()}")
    input("Press Enter to continue...")

    numbers = [20, 30, 40, 35]  # List of Fibonacci numbers to calculate
    with ThreadPoolExecutor(max_workers=4) as executor:
        executor.map(calculate_fibonacci, numbers)
