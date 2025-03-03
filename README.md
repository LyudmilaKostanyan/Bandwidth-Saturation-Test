# Bandwidth Saturation Test

## Project Overview

The **Bandwidth Saturation Test** simulates memory saturation by utilizing multiple threads to continuously read and write from a large buffer, aiming to saturate the memory bus. This test collects throughput data (in MB/s) to evaluate the impact of high memory usage and to explore how memory bandwidth limitations can create a bottleneck in modern computer systems.

The test demonstrates the concept of memory bottlenecks, recalling the original Von Neumann architecture's limitation where the same data bus is shared for both instructions and data, affecting overall system performance.

### Expected Outcome
- The test is designed to show how memory throughput decreases when the memory bus becomes saturated.
- The test will simulate high memory usage and demonstrate how the limited bandwidth can impact overall performance.
- The results will show how multiple threads accessing the same buffer can cause contention, and how this contention is a result of the shared memory bus in a system architecture.

## Features
1. **Multiple Threads**:  
   The program uses multiple threads to perform continuous read and write operations on a large buffer, simulating memory saturation.

2. **Throughput Data**:  
   The program collects throughput data, displaying the number of MBs transferred per second during the test.

3. **Memory Bottleneck Simulation**:  
   By saturating the memory bus, the program demonstrates how memory contention occurs and how the bandwidth limitation becomes a bottleneck.

4. **Von Neumann Bottleneck Explanation**:  
   The project explains the Von Neumann bottleneck, where the shared data and instruction pathways limit overall throughput.

## Example Output

The output of the program will show the throughput in MB/s for different thread configurations. An example output might look like this:

```
Performance Results:
+--------------------------+-----------------+
| Metric                   | Value           |
+--------------------------+-----------------+
| Max Thread Throughput    |    4732.65 MB/s |
| Total Throughput         |   67496.45 MB/s |
| Theoretical Bandwidth    |   68160.00 MB/s |
| Efficiency               |      99.03 %    |
+--------------------------+-----------------+
```

As more threads are added, the throughput gradually decreases, illustrating the saturation of the memory bus.

## Performance Considerations

### Why Memory Saturation Happens
- **Single Memory Bus**: Modern systems, especially those based on the Von Neumann architecture, suffer from the limitation of having a single memory bus shared by both data and instructions. This creates a bottleneck, especially when multiple threads are accessing memory simultaneously.
  
- **Thread Contention**: When multiple threads access the same memory regions, they contend for the bandwidth of the memory bus. This results in delays, reducing the overall throughput and performance of the system.

- **Von Neumann Bottleneck**: The Von Neumann architecture, which uses a shared bus for both data and instructions, limits how quickly data can be transferred between the CPU and memory. As the number of threads increases, the system cannot keep up with the demand for data, leading to reduced performance.

## How to Compile and Run the Code

1. **Clone the repository**:  
   If you haven't cloned the repository yet, do so by running:
   ```bash
   git clone https://github.com/LyudmilaKostanyan/Bandwidth-Saturation-Test.git
   cd bandwidth-saturation-test
   ```

2. **Build the project**:  
   Once you're in the project directory, compile the code with:
   ```bash
   cmake -S . -B build
   cmake --build build
   ```

3. **Run the compiled executable**:  
   After compiling, you can run the program:
   ```bash
   cd build
   ./main
   ```

## Explanation of Results

The results will show how throughput is affected by the number of threads. Initially, with fewer threads, throughput may be high because the memory bus is not fully saturated. However, as more threads are introduced, throughput will gradually decrease, demonstrating the limitations of a shared memory bus.

The program will also display the total time taken for each thread configuration, allowing you to compare performance across different levels of parallelism. This comparison will help illustrate how the memory bandwidth becomes a bottleneck in high-load scenarios.
