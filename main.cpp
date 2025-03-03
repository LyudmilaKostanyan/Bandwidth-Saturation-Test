#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <cstdlib>

const size_t BUFFER_SIZE_BYTES = 1ULL * 1024ULL * 1024ULL * 1024ULL;  // 1 GB
const size_t ELEMENT_SIZE = sizeof(int64_t);
const size_t NUM_ELEMENTS = BUFFER_SIZE_BYTES / ELEMENT_SIZE;
const int NUM_THREADS = 22;

// Для подсчета пропускной способности
std::atomic<int64_t> totalElementsProcessed(0);
std::mutex throughputMutex;
std::vector<double> threadThroughputs(NUM_THREADS, 0.0);

// Использование aligned_alloc для выделения памяти с выравниванием
void memoryTest(int threadId, int64_t* data) {
    auto start = std::chrono::high_resolution_clock::now();

    size_t dataPerThread = NUM_ELEMENTS / NUM_THREADS;
    size_t startIdx = threadId * dataPerThread;
    size_t endIdx = (threadId == NUM_THREADS - 1) ? NUM_ELEMENTS : (threadId + 1) * dataPerThread;

    // Обработка данных без кэширования
    for (size_t i = startIdx; i < endIdx; i += 8) {  // Работать с 8 элементами за раз
        for (size_t j = 0; j < 8; ++j) {
            data[i + j] += 1;
            totalElementsProcessed.fetch_add(1, std::memory_order_relaxed);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    double throughputMB = (dataPerThread * sizeof(int64_t)) / (1024.0 * 1024.0);
    double throughputPerSecond = throughputMB / duration.count();

    {
        std::lock_guard<std::mutex> lock(throughputMutex);
        threadThroughputs[threadId] = throughputPerSecond;
    }
}

int main() {
    // Использование aligned_alloc для выделения выровненной памяти
    int64_t* data = (int64_t*)aligned_alloc(64, NUM_ELEMENTS * sizeof(int64_t));

    if (!data) {
        std::cerr << "Memory allocation failed!" << std::endl;
        return 1;
    }

    std::fill(data, data + NUM_ELEMENTS, 0); // Инициализируем память

    std::vector<std::thread> threads;

    // Запуск всех потоков
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(memoryTest, i, data);
    }

    // Ожидание завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    // Подсчет максимальной пропускной способности потока
    double maxThroughput = *std::max_element(threadThroughputs.begin(), threadThroughputs.end());

    // Теоретическая пропускная способность
    double theoreticalBandwidth = 68160.0; // Теоретическая пропускная способность из расчета

    std::cout << "Max thread throughput: " << maxThroughput << " MB/s\n";
    std::cout << "Theoretical system memory bandwidth: " << theoreticalBandwidth << " MB/s\n";
    std::cout << "Efficiency (throughput/theoretical): " << (maxThroughput / theoreticalBandwidth) * 100 << "%\n";

    // Освобождение выделенной памяти
    free(data);

    return 0;
}
