#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#pragma pack(push, 1)
struct CarInsurance {
    int32_t number;
    char company[50];
};
#pragma pack(pop)

const char* COMPANIES[] = {
    "Ингосстрах", "РЕСО-Гарантия", "АльфаСтрахование", "Согласие",
    "ВСК", "Ренессанс Страхование", "СОГАЗ", "Тинькофф Страхование"
};
const int COMPANIES_COUNT = sizeof(COMPANIES) / sizeof(COMPANIES[0]);

const int MIN_KEY = 100000;
const int MAX_KEY = 999999;
const int REPEATS = 1000;
const unsigned SEED = 10;

std::mt19937 generator(SEED);

std::vector<int32_t> generateKeys(int n) {
    std::uniform_int_distribution<int> distribution(MIN_KEY, MAX_KEY);
    std::unordered_set<int32_t> used;
    std::vector<int32_t> keys;

    while ((int)keys.size() < n) {
        int32_t key = distribution(generator);
        if (used.insert(key).second) {
            keys.push_back(key);
        }
    }
    return keys;
}

std::vector<CarInsurance> fillRecords(const std::vector<int32_t>& keys) {
    std::vector<CarInsurance> records(keys.size());

    for (size_t i = 0; i < keys.size(); i++) {
        CarInsurance record{};
        record.number = keys[i];
        const char* name = COMPANIES[generator() % COMPANIES_COUNT];
        std::strncpy(record.company, name, sizeof(record.company) - 1);
        records[i] = record;
    }
    return records;
}

void createBinaryFile(const std::string& filename, const std::vector<CarInsurance>& records) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + filename);
    }
    for (const CarInsurance& record : records) {
        out.write(reinterpret_cast<const char*>(&record), sizeof(CarInsurance));
    }
}

void printBinaryFile(const std::string& filename, int limit) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    std::cout << "\nНомер    Компания\n";
    std::cout << "------------------------------\n";

    CarInsurance record;
    int count = 0;
    while (count < limit && in.read(reinterpret_cast<char*>(&record), sizeof(CarInsurance))) {
        std::cout << record.number << "   " << record.company << '\n';
        count++;
    }
}

bool linearSearch(const std::string& filename, int32_t key, CarInsurance& result, long long& comparisons) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    comparisons = 0;
    CarInsurance record;
    while (in.read(reinterpret_cast<char*>(&record), sizeof(CarInsurance))) {
        comparisons++;
        if (record.number == key) {
            result = record;
            return true;
        }
    }
    return false;
}

void showExample() {
    std::cout << "=== Тестовый пример (10 записей) ===\n";

    std::vector<int32_t> keys = generateKeys(10);
    std::vector<CarInsurance> records = fillRecords(keys);
    createBinaryFile("example.bin", records);
    printBinaryFile("example.bin", 10);

    int32_t key = keys[6];
    long long comparisons = 0;
    CarInsurance found;
    std::cout << "\nИщем ключ: " << key << '\n';
    if (linearSearch("example.bin", key, found, comparisons)) {
        std::cout << "Найдено: " << found.number << "   " << found.company << '\n';
        std::cout << "Сравнений: " << comparisons << '\n';
    } else {
        std::cout << "Не найдено\n";
    }
    std::cout << '\n';
}

int main() {
    showExample();

    const int sizes[] = {100, 1000, 10000, 100000};
    const std::string filename = "CarInsurance.bin";

    std::cout << "=== Линейный поиск, худший случай (ключ последней записи) ===\n\n";
    std::cout << "n\tT(n), мкс\tТф\n";
    std::cout << "---------------------------------\n";

    for (int n : sizes) {
        std::vector<int32_t> keys = generateKeys(n);
        std::vector<CarInsurance> records = fillRecords(keys);
        createBinaryFile(filename, records);

        int32_t key = keys.back();
        CarInsurance found;
        long long comparisons = 0;
        long long total = 0;

        for (int i = 0; i < REPEATS; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            linearSearch(filename, key, found, comparisons);
            auto end = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        }
        double time = total / (double)REPEATS / 1000.0;

        std::cout << n << "\t" << time << "\t\t" << comparisons << '\n';
    }

    return 0;
}
