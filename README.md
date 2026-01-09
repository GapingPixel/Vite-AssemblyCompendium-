To properly use a target hardware you do Assembly in c/c++. 

The Game Industry has historically needed well executed Assembly due to the demanding performance requirements by customers and propietary stable Target hardware.  

"Bibliography"

https://medium.com/@myn.create/why-apple-silicon-is-better-7d6628aa0c3e


[for practical terms you do assembly in c/c++](https://arxiv.org/html/2502.05317v1)


https://www.copetti.org/writings/consoles/game-boy-advance/

----------------------------------------------------------------------------------------------------


**Framework for matrix multiplication**

float* internal_make_page_aligned_matrix(
    unsigned int n,               // Size of the matrix (NxN)
    unsigned int& memory_length   // Reference to return allocated memory size
) {
    unsigned int required_size = n * n * sizeof(float); // Calculate required memory
    unsigned int rem = required_size % APPLE_M1_PAGE_SIZE; // Check alignment

    memory_length =
        rem == 0 ? required_size : required_size + APPLE_M1_PAGE_SIZE - rem;

    // Allocate page-aligned memory
    auto* arr = (float*)aligned_alloc(APPLE_M1_PAGE_SIZE, memory_length);
    assert(arr != nullptr); // Ensure allocation was successful
    return arr;
}


void internal_power_sample() {
    const char* pid_string = std::getenv("POWER_MONITOR"); // Get PID from environment
    char* end;
    int pid = static_cast<int>(std::strtol(pid_string, &end, 10)); // Convert PID to integer
    kill(pid, SIGINFO); // Send SIGINFO signal to the process
}

**Test suite to measure the performance of matrix multiplication** 

template<typename MatMul>
void test_suite(
    MatMul consumer,                         // Callback for the matrix multiplication
    const std::string& pathPrefix = "../data/" // Path to input data
) {
    unsigned int n = MATRIX_N; // Matrix size (NxN)
    unsigned int memory_length;

    // Allocate page-aligned matrices
    float* left  = internal_make_page_aligned_matrix(n, memory_length);
    float* right = internal_make_page_aligned_matrix(n, memory_length);
    float* out   = internal_make_page_aligned_matrix(n, memory_length);

    // Load matrices from file
    internal_load_matrix(pathPrefix, n, 0, left);  // Load left matrix
    internal_load_matrix(pathPrefix, n, 1, right); // Load right matrix

    // Measure the execution time
    auto before = std::chrono::high_resolution_clock::now();
    consumer(n, memory_length, left, right, out); // Perform matrix multiplication
    auto after = std::chrono::high_resolution_clock::now();

    // Signal power measurement and log elapsed time
    internal_power_sample();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before);
    std::cout << elapsed.count() << std::endl; // Print elapsed time in nanoseconds
}
