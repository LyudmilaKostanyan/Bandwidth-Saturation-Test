#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <atomic>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#if defined(_MSC_VER)
#include <malloc.h>
#endif
#ifdef __AVX2__
#include <immintrin.h>
#endif

using namespace std::chrono;

const size_t BUFFER_SIZE = 2ULL * 1024ULL * 1024ULL * 1024ULL;
const size_t STRIDE = 16;
const size_t ELEMENT_SIZE = sizeof(int64_t);
const size_t ELEMENTS = BUFFER_SIZE / ELEMENT_SIZE;
const int64_t NUM_ITERATIONS = 1000;
const unsigned NUM_THREADS = std::thread::hardware_concurrency();

std::atomic<size_t> totalBytesProcessed(0);

// Кроссплатформенная функция выделения выровненной памяти
void* aligned_alloc_cross(size_t alignment, size_t size) {
#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#elif defined(__MINGW32__)
    return __mingw_aligned_malloc(size, alignment);
#elif defined(__APPLE__) || defined(__linux__)
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) ptr = nullptr;
    return ptr;
#else
    // C++17 aligned_alloc (но не все реализации поддерживают)
    return std::aligned_alloc(alignment, size);
#endif
}

// Кроссплатформенное освобождение памяти
void aligned_free_cross(void* ptr) {
#if defined(_MSC_VER)
    _aligned_free(ptr);
#elif defined(__MINGW32__)
    __mingw_aligned_free(ptr);
#else
    free(ptr);
#endif
}

void threadWorker(int threadId, int64_t* buf, size_t totalElements) {
    size_t chunkSize = totalElements / NUM_THREADS;
    size_t startIdx = threadId * chunkSize;
    size_t endIdx = (threadId == NUM_THREADS - 1) ? totalElements : startIdx + chunkSize;

    size_t bytesProcessed = 0;

#ifdef __AVX2__
    __m256i pattern = _mm256_set1_epi64x(0xDEADBEEFDEADBEEF);
    for (int64_t iter = 0; iter < NUM_ITERATIONS; ++iter) {
        size_t i = startIdx;
        for (; i + 4 <= endIdx; i += 4) {
            _mm256_stream_si256(reinterpret_cast<__m256i*>(&buf[i]), pattern);
            bytesProcessed += 32;
        }
    }
#else
    for (int64_t iter = 0; iter < NUM_ITERATIONS; ++iter) {
        for (size_t i = startIdx; i < endIdx; ++i) {
            buf[i] = 0xDEADBEEFDEADBEEF;
            bytesProcessed += sizeof(int64_t);
        }
    }
#endif

    totalBytesProcessed.fetch_add(bytesProcessed, std::memory_order_relaxed);
}

int main() {
    int64_t* buffer = reinterpret_cast<int64_t*>(aligned_alloc_cross(32, BUFFER_SIZE));
    if (!buffer) {
        std::cerr << "Failed to allocate aligned buffer!" << std::endl;
        return 1;
    }
	std::cout << "Hardware concurrency: " << NUM_THREADS << std::endl;

	std::vector<std::thread> threads;
	threads.reserve(NUM_THREADS);

	auto startTime = steady_clock::now();

	for (unsigned i = 0; i < NUM_THREADS; ++i) {
		threads.emplace_back(threadWorker, i, buffer, ELEMENTS);
	}

	for (auto& t : threads)
		t.join();

    aligned_free_cross(buffer); // Освобождаем память

		auto endTime = steady_clock::now();
	auto totalDuration = duration_cast<milliseconds>(endTime - startTime);
	double seconds = totalDuration.count() / 1000.0;

	double totalMB = totalBytesProcessed.load() / (1024.0 * 1024.0);
	double totalThroughput = totalMB / seconds;

	std::vector<double> threadThroughputs(NUM_THREADS, 0.0);
	double maxThroughput = 0.0;

	for (int i = 0; i < NUM_THREADS; ++i) {
		threadThroughputs[i] = totalThroughput / NUM_THREADS;
		if (threadThroughputs[i] > maxThroughput)
			maxThroughput = threadThroughputs[i];
	}

	double theoreticalBandwidth = 68160.0;
	double efficiency = (totalThroughput / theoreticalBandwidth) * 100;

	std::cout << "\nPerformance Results:\n";
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "+--------------------------+-----------------+\n";
	std::cout << "| Metric                   | Value           |\n";
	std::cout << "+--------------------------+-----------------+\n";
	std::cout << "| Max Thread Throughput    | " << std::setw(10) << maxThroughput << " MB/s |\n";
	std::cout << "| Total Throughput         | " << std::setw(10) << totalThroughput << " MB/s |\n";
	std::cout << "| Theoretical Bandwidth    | " << std::setw(10) << theoreticalBandwidth << " MB/s |\n";
	std::cout << "| Efficiency               | " << std::setw(10) << efficiency << " %    |\n";
	std::cout << "+--------------------------+-----------------+\n";

	return 0;
}
