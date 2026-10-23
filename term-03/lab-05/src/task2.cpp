#include <algorithm>
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

struct TableRow {
    int32_t key;
    long long offset;
};

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

bool compareByKey(const TableRow& a, const TableRow& b) {
    return a.key < b.key;
}

std::vector<TableRow> buildTable(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    std::vector<TableRow> table;
    CarInsurance record;
    long long offset = 0;
    while (in.read(reinterpret_cast<char*>(&record), sizeof(CarInsurance))) {
        TableRow row;
        row.key = record.number;
        row.offset = offset;
        table.push_back(row);
        offset += sizeof(CarInsurance);
    }

    std::sort(table.begin(), table.end(), compareByKey);
    return table;
}

long long interpolationSearch(const std::vector<TableRow>& table, int32_t key, long long& comparisons) {
    comparisons = 0;
    int low = 0;
    int high = (int)table.size() - 1;

    while (low <= high && key >= table[low].key && key <= table[high].key) {
        if (table[low].key == table[high].key) {
            comparisons++;
            if (table[low].key == key) {
                return table[low].offset;
            }
            return -1;
        }

        long long shift = (long long)(key - table[low].key) * (high - low);
        int pos = low + shift / (table[high].key - table[low].key);

        comparisons++;
        if (table[pos].key == key) {
            return table[pos].offset;
        }

        comparisons++;
        if (table[pos].key < key) {
            low = pos + 1;
        } else {
            high = pos - 1;
        }
    }
    return -1;
}

bool readByOffset(const std::string& filename, long long offset, CarInsurance& result) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    in.seekg(offset, std::ios::beg);
    in.read(reinterpret_cast<char*>(&result), sizeof(CarInsurance));
    return (bool)in;
}

bool findRecord(const std::string& filename, const std::vector<TableRow>& table,
                int32_t key, CarInsurance& result, long long& comparisons) {
    long long offset = interpolationSearch(table, key, comparisons);
    if (offset == -1) {
        return false;
    }
    return readByOffset(filename, offset, result);
}

void showExample() {
    std::cout << "=== Тестовый пример (10 записей) ===\n";

    std::vector<int32_t> keys = generateKeys(10);
    std::vector<CarInsurance> records = fillRecords(keys);
    createBinaryFile("example.bin", records);
    printBinaryFile("example.bin", 10);

    std::vector<TableRow> table = buildTable("example.bin");
    std::cout << "\nТаблица (ключ, смещение), отсортирована по ключу:\n";
    for (const TableRow& row : table) {
        std::cout << row.key << "   " << row.offset << '\n';
    }

    int32_t key = keys[3];
    long long comparisons = 0;
    CarInsurance found;
    std::cout << "\nИщем ключ: " << key << '\n';
    if (findRecord("example.bin", table, key, found, comparisons)) {
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

    std::cout << "=== Интерполяционный поиск через таблицу, ключ последней записи файла ===\n\n";
    std::cout << "n\tT(n), мкс\tТф\tсборка таблицы, мс\n";
    std::cout << "--------------------------------------------------\n";

    for (int n : sizes) {
        std::vector<int32_t> keys = generateKeys(n);
        std::vector<CarInsurance> records = fillRecords(keys);
        createBinaryFile(filename, records);

        int32_t key = keys.back();
        CarInsurance found;
        long long comparisons = 0;

        auto buildStart = std::chrono::high_resolution_clock::now();
        std::vector<TableRow> table = buildTable(filename);
        auto buildEnd = std::chrono::high_resolution_clock::now();
        double buildTime = std::chrono::duration_cast<std::chrono::nanoseconds>(buildEnd - buildStart).count() / 1000000.0;

        long long total = 0;
        for (int i = 0; i < REPEATS; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            findRecord(filename, table, key, found, comparisons);
            auto end = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        }
        double time = total / (double)REPEATS / 1000.0;

        std::cout << n << "\t" << time << "\t\t" << comparisons << "\t" << buildTime << '\n';
    }

    return 0;
}
