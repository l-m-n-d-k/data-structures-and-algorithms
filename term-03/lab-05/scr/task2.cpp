#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

struct Record {
    int key;
    char company[30];
};

const char* COMPANIES[10] = {
    "Rosgosstrah", "Ingosstrah", "AlfaStrahovanie", "SOGAZ", "RESO-Garantia",
    "VSK", "Tinkoff Strahovanie", "Soglasie", "MAKS", "Ugoria"
};

struct Pair {
    int key;
    int number;
};

bool comparePairs(const Pair& a, const Pair& b) {
    return a.key < b.key;
}

void createFile(const char* filename, int n, int keys[]) {
    bool used[900000] = { false };
    for (int i = 0; i < n; i++) {
        int key;
        do {
            key = 100000 + rand() % 900000;
        } while (used[key - 100000]);
        used[key - 100000] = true;
        keys[i] = key;
    }

    ofstream file(filename, ios::binary);
    for (int i = 0; i < n; i++) {
        Record rec;
        rec.key = keys[i];
        strcpy(rec.company, COMPANIES[rand() % 10]);
        file.write((char*)&rec, sizeof(Record));
    }
    file.close();
}

void buildTable(const char* filename, int n, Pair table[]) {
    ifstream file(filename, ios::binary);
    Record rec;
    int number = 0;
    while (file.read((char*)&rec, sizeof(Record))) {
        table[number].key = rec.key;
        table[number].number = number;
        number++;
    }
    file.close();
    sort(table, table + n, comparePairs);
}

int interpolationSearch(Pair table[], int n, int key, int& comparisons) {
    int low = 0, high = n - 1;
    comparisons = 0;
    while (low <= high && table[low].key <= key && key <= table[high].key) {
        int pos;
        if (table[low].key == table[high].key)
            pos = low;
        else
            pos = low + (long long)(key - table[low].key) * (high - low) / (table[high].key - table[low].key);

        comparisons++;
        if (table[pos].key == key)
            return table[pos].number;
        else if (table[pos].key < key)
            low = pos + 1;
        else
            high = pos - 1;
    }
    return -1;
}

void readRecord(const char* filename, int number, Record& found) {
    ifstream file(filename, ios::binary);
    file.seekg((long long)number * sizeof(Record));
    file.read((char*)&found, sizeof(Record));
    file.close();
}

int main() {
    srand(1);
    int demoKeys[10];
    createFile("demo.bin", 10, demoKeys);

    Pair demoTable[10];
    buildTable("demo.bin", 10, demoTable);

    cout << "Таблица (ключ, номер записи):" << endl;
    for (int i = 0; i < 10; i++)
        cout << "  [" << i << "] " << demoTable[i].key << " -> запись N" << demoTable[i].number << endl;

    int searchKey = demoKeys[3];
    cout << "Ищем ключ " << searchKey << endl;
    int comparisons;
    int number = interpolationSearch(demoTable, 10, searchKey, comparisons);
    Record found;
    readRecord("demo.bin", number, found);
    cout << "Номер записи: " << number << ", сравнений: " << comparisons
         << ", компания: " << found.company << endl << endl;

    srand(42);
    int sizes[] = { 100, 1000, 10000, 100000 };

    cout << "n\tтаблица, мс\tT(n), мс\tТф\tТф(среднее по 1000 ключам)" << endl;
    for (int s = 0; s < 4; s++) {
        int n = sizes[s];
        char filename[30];
        sprintf(filename, "data_%d.bin", n);

        int* keys = new int[n];
        createFile(filename, n, keys);
        int lastKey = keys[n - 1];

        Pair* table = new Pair[n];
        clock_t start = clock();
        buildTable(filename, n, table);
        double buildMs = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;

        int comp;
        Record rec;
        start = clock();
        for (int i = 0; i < 1000; i++) {
            number = interpolationSearch(table, n, lastKey, comp);
            readRecord(filename, number, rec);
        }
        double timeMs = (double)(clock() - start) / CLOCKS_PER_SEC / 1000 * 1000;

        long sum = 0;
        int samples = n < 1000 ? n : 1000;
        for (int i = 0; i < samples; i++) {
            int c;
            interpolationSearch(table, n, keys[rand() % n], c);
            sum += c;
        }
        double average = (double)sum / samples;

        cout << n << "\t" << buildMs << "\t" << timeMs << "\t" << comp << "\t" << average << endl;
        delete[] keys;
        delete[] table;
    }
    return 0;
}
