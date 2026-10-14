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

bool linearSearch(const char* filename, int key, Record& found, int& comparisons) {
    ifstream file(filename, ios::binary);
    Record rec;
    comparisons = 0;
    while (file.read((char*)&rec, sizeof(Record))) {
        comparisons++;
        if (rec.key == key) {
            found = rec;
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

int main() {
    srand(42);
    int sizes[] = { 100, 1000, 10000, 100000 };

    cout << "n\tT(n), мс\tТф" << endl;
    for (int s = 0; s < 4; s++) {
        int n = sizes[s];
        char filename[30];
        sprintf(filename, "data_%d.bin", n);

        int* keys = new int[n];
        createFile(filename, n, keys);
        int lastKey = keys[n - 1];

        Record found;
        int comparisons = 0;
        clock_t start = clock();
        for (int i = 0; i < 10; i++)
            linearSearch(filename, lastKey, found, comparisons);
        clock_t end = clock();
        double timeMs = (double)(end - start) / CLOCKS_PER_SEC / 10 * 1000;

        cout << n << "\t" << timeMs << "\t" << comparisons << endl;
        delete[] keys;
    }
    return 0;
}
