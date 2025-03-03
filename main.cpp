#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <immintrin.h>
#include <iomanip>

const size_t BUFFER_SIZE_BYTES = 1ULL * 1024ULL * 1024ULL * 1024ULL;
const size_t ELEMENT_SIZE = sizeof(int64_t);
const size_t NUM_ELEMENTS = BUFFER_SIZE_BYTES / ELEMENT_SIZE;
const int NUM_THREADS = 17;

std::atomic<int64_t> totalElementsProcessed(0);
std::mutex throughputMutex;
std::vector<double> threadThroughputs(NUM_THREADS, 0.0);

void memoryTest(int threadId, int64_t* data) {
    auto start = std::chrono::high_resolution_clock::now();

    size_t dataPerThread = NUM_ELEMENTS / NUM_THREADS;
    size_t startIdx = threadId * dataPerThread;
    size_t endIdx = (threadId == NUM_THREADS - 1) ? NUM_ELEMENTS : (threadId + 1) * dataPerThread;

    __m256i one = _mm256_set1_epi64x(1);
    __m256i sum = _mm256_setzero_si256();

    size_t i = startIdx;
    for (; i + 32 <= endIdx; i += 32) {
        __m256i vec1 = _mm256_load_si256((__m256i*)&data[i]);
        __m256i vec2 = _mm256_load_si256((__m256i*)&data[i + 4]);
        __m256i vec3 = _mm256_load_si256((__m256i*)&data[i + 8]);
        __m256i vec4 = _mm256_load_si256((__m256i*)&data[i + 12]);
        __m256i vec5 = _mm256_load_si256((__m256i*)&data[i + 16]);
        __m256i vec6 = _mm256_load_si256((__m256i*)&data[i + 20]);
        __m256i vec7 = _mm256_load_si256((__m256i*)&data[i + 24]);
        __m256i vec8 = _mm256_load_si256((__m256i*)&data[i + 28]);

        vec1 = _mm256_add_epi64(vec1, one);
        vec2 = _mm256_add_epi64(vec2, vec1);
        vec3 = _mm256_add_epi64(vec3, vec2);
        vec4 = _mm256_add_epi64(vec4, vec3);
        vec5 = _mm256_add_epi64(vec5, vec4);
        vec6 = _mm256_add_epi64(vec6, vec5);
        vec7 = _mm256_add_epi64(vec7, vec6);
        vec8 = _mm256_add_epi64(vec8, vec7);

        _mm256_stream_si256((__m256i*)&data[i], vec1);
        _mm256_stream_si256((__m256i*)&data[i + 4], vec2);
        _mm256_stream_si256((__m256i*)&data[i + 8], vec3);
        _mm256_stream_si256((__m256i*)&data[i + 12], vec4);
        _mm256_stream_si256((__m256i*)&data[i + 16], vec5);
        _mm256_stream_si256((__m256i*)&data[i + 20], vec6);
        _mm256_stream_si256((__m256i*)&data[i + 24], vec7);
        _mm256_stream_si256((__m256i*)&data[i + 28], vec8);

        sum = _mm256_add_epi64(sum, vec8);
        totalElementsProcessed.fetch_add(32, std::memory_order_relaxed);
    }

    for (; i < endIdx; ++i) {
        data[i] += 1;
        totalElementsProcessed.fetch_add(1, std::memory_order_relaxed);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    double throughputMB = (dataPerThread * sizeof(int64_t)) / (1024.0 * 1024.0) * 2;
    double throughputPerSecond = throughputMB / duration.count();

    {
        std::lock_guard<std::mutex> lock(throughputMutex);
        threadThroughputs[threadId] = throughputPerSecond;
    }

    volatile int64_t dummy = _mm256_extract_epi64(sum, 0);
    (void)dummy;
}

int main() {

    int64_t* data = (int64_t*)std::aligned_alloc(32, NUM_ELEMENTS * sizeof(int64_t));
    if (!data) {
        std::cerr << "Memory allocation failed!" << std::endl;
        return 1;
    }

    std::fill(data, data + NUM_ELEMENTS, 0);

    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(memoryTest, i, data);
    }

    for (auto& t : threads) {
        t.join();
    }

    double maxThroughput = *std::max_element(threadThroughputs.begin(), threadThroughputs.end());
    double totalThroughput = std::accumulate(threadThroughputs.begin(), threadThroughputs.end(), 0.0);
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

    free(data);
    return 0;
}