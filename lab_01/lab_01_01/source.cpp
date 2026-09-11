#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace std;

// Структура для хранения показателей сложности
struct Stats {
  long long Cn = 0; // Сравнения
  long long Mn = 0; // Перемещения
  long long Tn = 0; // Суммарная сложность
};

// Алгоритм 1
Stats delFirstMethod(char* x, int &n, char key) {
  Stats s;
  int i = 0;
  while (i < n) {
    s.Cn++; // Сравнение x[i] == key
    if (x[i] == key) {
      for (int j = i; j < n - 1; j++) {
        x[j] = x[j + 1];
        s.Mn++; // Перемещение элемента
      }
      n--; // Уменьшение эффективного размера
    } else {
      i++;
    }
  }
  s.Tn = s.Cn + s.Mn;
  return s;
}

// Алгоритм 2: Удаление за один проход (метод двух указателей)
Stats delOtherMethod(char* x, int &n, char key) {
  Stats s;
  int j = 0;
  for (int i = 0; i < n; i++) {
    s.Cn++; // Сравнение x[i] != key
    if (x[i] != key) {
      x[j] = x[i];
      s.Mn++; // Перемещение (копирование)
      j++;
    }
  }
  n = j; // Новый размер массива
  s.Tn = s.Cn + s.Mn;
  return s;
}

// Функция для заполнения массива случайными символами
void fillRandom(char* arr, int n) {
  for (int i = 0; i < n; i++) {
    arr[i] = 'A' + rand() % 26;
  }
}

int main() {
  setlocale(LC_ALL, "Russian");
  srand(time(0));

  int n_sizes[] = {100, 200, 500, 1000, 2000, 5000, 10000}; 
  char key = 'A';

  for (int n_orig : n_sizes) {
    cout << "--- Размер массива: " << n_orig << " ---" << endl;

    // Тестирование Алгоритма 1
    char* arr1 = new char[n_orig];
    fillRandom(arr1, n_orig);
    int n1 = n_orig;
    Stats s1 = delFirstMethod(arr1, n1, key);

    // Тестирование Алгоритма 2
    char* arr2 = new char[n_orig];
    fillRandom(arr2, n_orig);
    int n2 = n_orig;
    Stats s2 = delOtherMethod(arr2, n2, key);

    cout << "Алгоритм 1: Cn=" << s1.Cn << ", Mn=" << s1.Mn << ", Tn=" << s1.Tn << endl;
    cout << "Алгоритм 2: Cn=" << s2.Cn << ", Mn=" << s2.Mn << ", Tn=" << s2.Tn << endl;

    delete[] arr1;
    delete[] arr2;
    cout << endl;
  }

  cout <<"------------------------------------------" << endl << endl;

  int n_size = 10;  
  key = '_';

  // --- СЛУЧАЙ Б: Все элементы удовлетворяют условию (Все — это key) ---
  cout << "---Все элементы удовелтворяют условию поиска. ---" << endl;
  char* arrB1 = new char[n_size];
  char* arrB2 = new char[n_size];
  for (int i = 0; i < n_size; i++) {
    arrB1[i] = arrB2[i] = key; 
  }

  int nB1 = n_size;
  int nB2 = n_size;
  Stats sB1 = delFirstMethod(arrB1, nB1, key);
  Stats sB2 = delOtherMethod(arrB2, nB2, key);

  cout << "Алгоритм 1: Cn=" << sB1.Cn << ", Mn=" << sB1.Mn << ", Tn=" << sB1.Tn << endl;
  cout << "Алгоритм 2: Cn=" << sB2.Cn << ", Mn=" << sB2.Mn << ", Tn=" << sB2.Tn << endl;

  cout << endl <<"------------------------------------------" << endl << endl;

  // --- СЛУЧАЙ В: Ни один элемент не удовлетворяет условию (Нет ни одного key) ---
  cout << "---Ни один элемент не удовлетворяет условию поиска. ---" << endl;
  char* arrV1 = new char[n_size];
  char* arrV2 = new char[n_size];
  for (int i = 0; i < n_size; i++) {
    arrV1[i] = arrV2[i] = 'A'; 
  }

  int nV1 = n_size;
  int nV2 = n_size;
  Stats sV1 = delFirstMethod(arrV1, nV1, key);
  Stats sV2 = delOtherMethod(arrV2, nV2, key);

  cout << "Алгоритм 1: Cn=" << sV1.Cn << ", Mn=" << sV1.Mn << ", Tn=" << sV1.Tn << endl;
  cout << "Алгоритм 2: Cn=" << sV2.Cn << ", Mn=" << sV2.Mn << ", Tn=" << sV2.Tn << endl;

  // Чистим память
  delete[] arrB1; delete[] arrB2;
  delete[] arrV1; delete[] arrV2;
  return 0;
}
