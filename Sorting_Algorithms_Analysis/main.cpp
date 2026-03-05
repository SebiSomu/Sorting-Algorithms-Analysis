#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>
#include <functional>
#include <stack>
#include <cmath>
#include <sstream>

// ──────────────────────────────────────────────
// Counter — passed by reference to every sort
// sw = swaps (2 writes = 1 swap)
// wr = element writes (shifts / copies)
// ──────────────────────────────────────────────
struct Counter {
    long long value = 0;
    void operator++() { ++value; }
    void reset() { value = 0; }
};

// Format number with thousands separator  e.g. 1234567 → "1,234,567"
std::string fmtNum(long long n) {
    std::string s = std::to_string(n);
    int pos = (int)s.length() - 3;
    while (pos > 0) {
        s.insert(pos, ",");
        pos -= 3;
    }
    return s;
}

// Centre a string inside a field of width w
std::string centre(const std::string& s, int w) {
    int pad = w - (int)s.size();
    if (pad <= 0) return s;
    int l = pad / 2;
    int r = pad - l;
    return std::string(l, ' ') + s + std::string(r, ' ');
}

// ──────────────────────────────────────────────
// Data generators
// ──────────────────────────────────────────────

std::vector<int> generateRandomVector(int size, int minVal = 1, int maxVal = 1000) {
    std::vector<int> vec(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minVal, maxVal);
    for (int i = 0; i < size; i++) {
        vec[i] = dis(gen);
    }
    return vec;
}

std::vector<int> generateSortedVector(int size) {
    std::vector<int> vec(size);
    for (int i = 0; i < size; i++) {
        vec[i] = i + 1;
    }
    return vec;
}

std::vector<int> generateReverseSortedVector(int size) {
    std::vector<int> vec(size);
    for (int i = 0; i < size; i++) {
        vec[i] = size - i;
    }
    return vec;
}

// ──────────────────────────────────────────────
// Measure: returns {avg_ms, last_count}
// ──────────────────────────────────────────────

template<typename Func>
std::pair<double, long long> measureExecutionTime(Func sortFunc, std::vector<int>& data, int repeats = 3) {
    double totalTime = 0.0;
    long long lastCount = 0;

    for (int i = 0; i < repeats; i++) {
        std::vector<int> tempData = data;
        Counter cnt;

        auto start = std::chrono::steady_clock::now();
        sortFunc(tempData, cnt);
        auto end = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        totalTime += duration.count() / 1000.0;
        lastCount = cnt.value;
    }
    return { totalTime / repeats, lastCount };
}

// Returns true if the array is sorted in ascending order
bool isSorted(const std::vector<int>& arr) {
    for (size_t i = 0; i < arr.size() - 1; i++) {
        if (arr[i] > arr[i + 1])
            return false;
    }
    return true;
}

// ══════════════════════════════════════════════
// SORTING ALGORITHMS
// ══════════════════════════════════════════════

// Swap Sort
// Time:  O(n²) — best, average, worst
// Space: O(1)
// Compares every pair (i, j) and swaps if out of order.
// Simple but performs the most swaps of all O(n²) sorts.
void swapSort(std::vector<int>& data, Counter& cnt) {
    for (size_t i = 0; i < data.size(); i++) {
        for (size_t j = i + 1; j < data.size(); j++) {
            if (data[i] > data[j]) {
                std::swap(data[i], data[j]);
                ++cnt;
            }
        }
    }
}

// Bubble Sort
// Time:  O(n²) average/worst  |  O(n) best (already sorted)
// Space: O(1)
// Repeatedly bubbles the largest unsorted element to the end.
void bubbleSort(std::vector<int>& data, Counter& cnt) {
    size_t n = data.size();
    for (size_t i = 0; i < n - 1; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                std::swap(data[j], data[j + 1]);
                ++cnt;
            }
        }
    }
}

// Selection Sort
// Time:  O(n²) — best, average, worst
// Space: O(1)
// Selects the minimum element in each pass and places it in position.
// Performs at most n-1 swaps — fewest swaps of all O(n²) sorts.
void selectionSort(std::vector<int>& data, Counter& cnt) {
    for (size_t i = 0; i < data.size(); i++) {
        int minIndex = i;
        for (size_t j = i + 1; j < data.size(); j++) {
            if (data[j] < data[minIndex]) {
                minIndex = j;
            }
        }
        if (minIndex != (int)i) {
            std::swap(data[i], data[minIndex]);
            ++cnt;
        }
    }
}

// Insertion Sort
// Time:  O(n²) average/worst  |  O(n) best (already sorted)
// Space: O(1)
// Builds the sorted array one element at a time using shifts.
// Efficient for small or nearly sorted data; used as base case in Intro/Tim Sort.
void insertionSort(std::vector<int>& data, Counter& cnt) {
    size_t n = data.size();
    for (size_t i = 1; i < n; i++) {
        int key = data[i];
        int j = static_cast<int>(i) - 1;
        while (j >= 0 && data[j] > key) {
            data[j + 1] = data[j];
            ++cnt;
            j--;
        }
        data[j + 1] = key;
        ++cnt;
    }
}

// Merge Sort
// Time:  O(n log n) — best, average, worst
// Space: O(n)       — requires auxiliary buffer for merging
// Divide-and-conquer: splits the array in half recursively, then merges.
// Stable sort; preferred when stability and guaranteed O(n log n) matter.
void mergeSort(std::vector<int>& data, Counter& cnt, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(data, cnt, left, mid);
        mergeSort(data, cnt, mid + 1, right);

        // Merge the two sorted halves
        std::vector<int> temp;
        int i = left, j = mid + 1;

        while (i <= mid && j <= right) {
            if (data[i] <= data[j])
                temp.push_back(data[i++]);
            else
                temp.push_back(data[j++]);
        }
        while (i <= mid)
            temp.push_back(data[i++]);
        while (j <= right)
            temp.push_back(data[j++]);

        for (size_t k = 0; k < temp.size(); k++) {
            data[left + k] = temp[k];
            ++cnt;
        }
    }
}

void mergeSortWrapper(std::vector<int>& data, Counter& cnt) {
    mergeSort(data, cnt, 0, data.size() - 1);
}

// Quick Sort — Recursive
// Time:  O(n log n) average  |  O(n²) worst (sorted/reverse input with last-element pivot)
// Space: O(log n) average    |  O(n) worst (call stack)
// Partitions around a pivot and recursively sorts both halves.
void quickSortRecursive(std::vector<int>& data, Counter& cnt, int left, int right) {
    if (left < right) {
        int pivot = data[right];
        int i = left - 1;

        for (int j = left; j < right; j++) {
            if (data[j] <= pivot) {
                i++;
                std::swap(data[i], data[j]);
                ++cnt;
            }
        }
        std::swap(data[i + 1], data[right]);
        ++cnt;
        int pi = i + 1;

        quickSortRecursive(data, cnt, left, pi - 1);
        quickSortRecursive(data, cnt, pi + 1, right);
    }
}

// Quick Sort — Iterative
// Time:  O(n log n) average  |  O(n²) worst
// Space: O(log n) average    |  O(n) worst (explicit stack replaces call stack)
// Same logic as recursive Quick Sort but avoids stack overflow on large inputs
// by using an explicit stack and always pushing the smaller partition first.
void quickSortIterative(std::vector<int>& data, Counter& cnt, int left, int right) {
    std::vector<std::pair<int, int>> stack;
    stack.push_back({ left, right });

    while (!stack.empty()) {
        auto [l, r] = stack.back();
        stack.pop_back();

        if (l < r) {
            int pivot = data[r];
            int i = l - 1;

            for (int j = l; j < r; j++) {
                if (data[j] <= pivot) {
                    i++;
                    std::swap(data[i], data[j]);
                    ++cnt;
                }
            }
            std::swap(data[i + 1], data[r]);
            ++cnt;
            int pi = i + 1;

            // Push the smaller partition first to keep stack depth at O(log n)
            if (pi - l < r - pi) {
                stack.push_back({ pi + 1, r });
                stack.push_back({ l, pi - 1 });
            }
            else {
                stack.push_back({ l, pi - 1 });
                stack.push_back({ pi + 1, r });
            }
        }
    }
}

void quickSortRecWrapper(std::vector<int>& data, Counter& cnt) {
    quickSortRecursive(data, cnt, 0, data.size() - 1);
}

void quickSortWrapper(std::vector<int>& data, Counter& cnt) {
    quickSortIterative(data, cnt, 0, data.size() - 1);
}

// Count Sort
// Time:  O(n + k)  where k = value range (max element)
// Space: O(k)
// Non-comparison sort; counts occurrences then reconstructs the sorted array.
// Very fast when k is small relative to n; impractical for large value ranges.
void countSort(std::vector<int>& data, Counter& cnt) {
    if (data.empty()) return;

    int maxVal = *std::max_element(data.begin(), data.end());
    std::vector<int> count(maxVal + 1, 0);

    for (int num : data)
        count[num]++;

    for (int i = 1; i <= maxVal; i++)
        count[i] += count[i - 1];

    std::vector<int> output(data.size());
    for (int i = data.size() - 1; i >= 0; i--) {
        output[count[data[i]] - 1] = data[i];
        count[data[i]]--;
    }

    for (size_t i = 0; i < data.size(); i++) {
        data[i] = output[i];
        ++cnt;
    }
}

// Radix Sort
// Time:  O(d * (n + k))  where d = number of digits, k = base (10)
// Space: O(n + k)
// Non-comparison sort; sorts digit by digit from least to most significant.
// Stable per-digit pass via counting sort; efficient for integers with few digits.
void radixSort(std::vector<int>& data, Counter& cnt) {
    if (data.empty()) return;

    int maxElem = *std::max_element(data.begin(), data.end());

    for (int exp = 1; maxElem / exp > 0; exp *= 10) {
        std::vector<int> output(data.size());
        std::vector<int> count(10, 0);

        for (int num : data)
            count[(num / exp) % 10]++;

        for (int i = 1; i < 10; i++)
            count[i] += count[i - 1];

        for (int i = data.size() - 1; i >= 0; i--) {
            int digit = (data[i] / exp) % 10;
            output[count[digit] - 1] = data[i];
            count[digit]--;
        }

        for (size_t i = 0; i < data.size(); i++) {
            data[i] = output[i];
            ++cnt;
        }
    }
}

// Heap Sort
// Time:  O(n log n) — best, average, worst
// Space: O(1)       — in-place via max-heap
// Builds a max-heap then repeatedly extracts the maximum to the end.
// Not stable; poor cache performance due to non-sequential memory access.
void heapSort(std::vector<int>& data, Counter& cnt) {
    int n = data.size();

    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        auto heapify = [&](int n, int i, auto&& heapify_ref) -> void {
            int largest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if (left < n && data[left] > data[largest])
                largest = left;
            if (right < n && data[right] > data[largest])
                largest = right;

            if (largest != i) {
                std::swap(data[i], data[largest]);
                ++cnt;
                heapify_ref(n, largest, heapify_ref);
            }
            };
        heapify(n, i, heapify);
    }

    // Extract elements from heap one by one
    for (int i = n - 1; i >= 0; i--) {
        std::swap(data[0], data[i]);
        ++cnt;
        auto heapify = [&](int n, int i, auto&& heapify_ref) -> void {
            int largest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if (left < n && data[left] > data[largest])
                largest = left;
            if (right < n && data[right] > data[largest])
                largest = right;

            if (largest != i) {
                std::swap(data[i], data[largest]);
                ++cnt;
                heapify_ref(n, largest, heapify_ref);
            }
            };
        heapify(i, 0, heapify);
    }
}

// Shell Sort
// Time:  O(n^(3/2)) with Knuth's gap sequence  |  O(n log² n) with other sequences
// Space: O(1)
// Generalization of Insertion Sort that allows far-apart element exchanges.
// Reduces inversions quickly by sorting elements at decreasing gap intervals.
void shellSort(std::vector<int>& data, Counter& cnt) {
    int n = data.size();

    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = data[i];
            int j = i;

            while (j >= gap && data[j - gap] > temp) {
                data[j] = data[j - gap];
                ++cnt;
                j -= gap;
            }
            data[j] = temp;
            ++cnt;
        }
    }
}

// Cocktail Sort  (Bidirectional Bubble Sort)
// Time:  O(n²) average/worst  |  O(n) best (already sorted)
// Space: O(1)
// Extends Bubble Sort by alternating the direction of each pass.
// Slightly faster than Bubble Sort in practice; handles turtles and rabbits better.
void cocktailSort(std::vector<int>& data, Counter& cnt) {
    int n = data.size();
    bool swapped = true;
    int start = 0;
    int end = n - 1;

    while (swapped) {
        swapped = false;

        for (int i = start; i < end; i++) {
            if (data[i] > data[i + 1]) {
                std::swap(data[i], data[i + 1]);
                ++cnt;
                swapped = true;
            }
        }

        if (!swapped) break;

        swapped = false;
        end--;

        for (int i = end - 1; i >= start; i--) {
            if (data[i] > data[i + 1]) {
                std::swap(data[i], data[i + 1]);
                ++cnt;
                swapped = true;
            }
        }
        start++;
    }
}

// Comb Sort
// Time:  O(n²) worst  |  O(n log n) average in practice
// Space: O(1)
// Improves Bubble Sort by using a shrinking gap (shrink factor ≈ 1.3).
// Eliminates turtles (small values near the end) efficiently.
void combSort(std::vector<int>& data, Counter& cnt) {
    int n = data.size();
    int gap = n;
    bool swapped = true;
    const double shrink = 1.3;

    while (gap > 1 || swapped) {
        gap = int(gap / shrink);
        if (gap < 1) gap = 1;

        swapped = false;

        for (int i = 0; i + gap < n; i++) {
            if (data[i] > data[i + gap]) {
                std::swap(data[i], data[i + gap]);
                ++cnt;
                swapped = true;
            }
        }
    }
}

// Gnome Sort  (Stupid Sort)
// Time:  O(n²) average/worst  |  O(n) best (already sorted)
// Space: O(1)
// Moves each element to its correct position like a garden gnome arranging pots.
// Equivalent to Insertion Sort but uses swaps instead of shifts.
void gnomeSort(std::vector<int>& data, Counter& cnt) {
    int n = data.size();
    int index = 0;

    while (index < n) {
        if (index == 0 || data[index] >= data[index - 1]) {
            index++;
        }
        else {
            std::swap(data[index], data[index - 1]);
            ++cnt;
            index--;
        }
    }
}

// Pigeonhole Sort
// Time:  O(n + range)  where range = max - min + 1
// Space: O(range)
// Non-comparison sort; places each element into its corresponding "hole".
// Optimal when range is close to n; impractical for large value ranges.
void pigeonholeSort(std::vector<int>& data, Counter& cnt) {
    if (data.empty()) return;

    int minVal = *std::min_element(data.begin(), data.end());
    int maxVal = *std::max_element(data.begin(), data.end());
    int range = maxVal - minVal + 1;

    std::vector<int> holes(range, 0);

    for (int num : data)
        holes[num - minVal]++;

    int index = 0;
    for (int i = 0; i < range; i++) {
        while (holes[i] > 0) {
            data[index++] = i + minVal;
            ++cnt;
            holes[i]--;
        }
    }
}

// Bucket Sort
// Time:  O(n + k) average  |  O(n²) worst (all elements in one bucket)
// Space: O(n + k)
// Distributes elements into buckets, sorts each bucket, then concatenates.
// Performance depends heavily on input distribution.
void bucketSort(std::vector<int>& data, Counter& cnt) {
    if (data.empty()) return;

    int minVal = *std::min_element(data.begin(), data.end());
    int maxVal = *std::max_element(data.begin(), data.end());
    int range = maxVal - minVal + 1;
    int n = static_cast<int>(data.size());
    int bucketCount = std::max(1, n / 128 + 1);
    std::vector<std::vector<int>> buckets(bucketCount);

    for (int v : data) {
        int idx = (int)((static_cast<long long>(v - minVal) * bucketCount) / (range ? range : 1));
        if (idx >= bucketCount) idx = bucketCount - 1;
        if (idx < 0) idx = 0;
        buckets[idx].push_back(v);
    }

    int pos = 0;
    for (auto& b : buckets) {
        std::sort(b.begin(), b.end());
        for (int v : b) {
            data[pos++] = v;
            ++cnt;
        }
    }
}

// ── Intro Sort helpers ────────────────────────

// Insertion Sort on a subrange [lo, hi] — used as base case in Intro Sort
static void insertionSortRange(std::vector<int>& a, Counter& cnt, int lo, int hi) {
    for (int i = lo + 1; i <= hi; ++i) {
        int key = a[i];
        int j = i - 1;
        while (j >= lo && a[j] > key) {
            a[j + 1] = a[j];
            ++cnt;
            --j;
        }
        a[j + 1] = key;
        ++cnt;
    }
}

// Lomuto partition scheme on a subrange
static int partitionRange(std::vector<int>& a, Counter& cnt, int lo, int hi) {
    int pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; ++j) {
        if (a[j] <= pivot) {
            ++i;
            std::swap(a[i], a[j]);
            ++cnt;
        }
    }
    std::swap(a[i + 1], a[hi]);
    ++cnt;
    return i + 1;
}

// Heap Sort on a subrange [lo, hi] — fallback when depth limit is reached
static void heapSortRange(std::vector<int>& a, Counter& cnt, int lo, int hi) {
    int n = hi - lo + 1;
    if (n <= 1) return;
    auto heapify = [&](int heapN, int i, auto&& self) -> void {
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < heapN && a[lo + l] > a[lo + largest]) largest = l;
        if (r < heapN && a[lo + r] > a[lo + largest]) largest = r;
        if (largest != i) {
            std::swap(a[lo + i], a[lo + largest]);
            ++cnt;
            self(heapN, largest, self);
        }
        };
    for (int i = n / 2 - 1; i >= 0; --i) heapify(n, i, heapify);
    for (int i = n - 1; i > 0; --i) {
        std::swap(a[lo], a[lo + i]);
        ++cnt;
        heapify(i, 0, heapify);
    }
}

static void introSortImpl(std::vector<int>& a, Counter& cnt, int lo, int hi, int depthLimit) {
    const int THRESH = 16;
    while (hi - lo > THRESH) {
        if (depthLimit == 0) {
            // Depth limit exceeded — fall back to Heap Sort to guarantee O(n log n)
            heapSortRange(a, cnt, lo, hi);
            return;
        }
        int p = partitionRange(a, cnt, lo, hi);
        --depthLimit;
        if (p - lo < hi - p) {
            introSortImpl(a, cnt, lo, p - 1, depthLimit);
            lo = p + 1;
        }
        else {
            introSortImpl(a, cnt, p + 1, hi, depthLimit);
            hi = p - 1;
        }
    }
    // Small partition — finish with Insertion Sort
    insertionSortRange(a, cnt, lo, hi);
}

// Intro Sort
// Time:  O(n log n) — best, average, worst  (guaranteed via HeapSort fallback)
// Space: O(log n)
// Hybrid of Quick Sort, Heap Sort, and Insertion Sort.
// Starts with Quick Sort; switches to Heap Sort if recursion depth exceeds 2*log(n);
// uses Insertion Sort for small partitions (size ≤ 16).
// This is the strategy behind std::sort in most standard library implementations.
void introSort(std::vector<int>& data, Counter& cnt) {
    int n = static_cast<int>(data.size());
    if (n <= 1) return;
    int depthLimit = 2 * static_cast<int>(std::log2(std::max(1, n)));
    introSortImpl(data, cnt, 0, n - 1, depthLimit);
}

// Tim Sort
// Time:  O(n log n) average/worst  |  O(n) best (already sorted or few runs)
// Space: O(n)
// Hybrid of Merge Sort and Insertion Sort, used in Python's sorted() and Java's Arrays.sort().
// Splits the array into natural or fixed-size runs (RUN=32), sorts each with Insertion Sort,
// then merges runs bottom-up. Extremely efficient on real-world partially sorted data.
void timSort(std::vector<int>& data, Counter& cnt) {
    int n = static_cast<int>(data.size());
    if (n <= 1) return;
    const int RUN = 32;

    auto insSortRange = [&](int left, int right) {
        for (int i = left + 1; i <= right; i++) {
            int key = data[i];
            int j = i - 1;
            while (j >= left && data[j] > key) {
                data[j + 1] = data[j];
                ++cnt;
                --j;
            }
            data[j + 1] = key;
            ++cnt;
        }
        };

    // Sort individual runs of size RUN using Insertion Sort
    for (int i = 0; i < n; i += RUN) {
        int r = std::min(i + RUN - 1, n - 1);
        insSortRange(i, r);
    }

    auto mergeRange = [&](int l, int m, int r) {
        int n1 = m - l + 1;
        int n2 = r - m;
        std::vector<int> L(n1), R(n2);
        for (int i = 0; i < n1; ++i) L[i] = data[l + i];
        for (int j = 0; j < n2; ++j) R[j] = data[m + 1 + j];
        int i = 0, j = 0, k = l;
        while (i < n1 && j < n2) {
            if (L[i] <= R[j]) {
                data[k++] = L[i++];
            }
            else {
                data[k++] = R[j++];
            }
            ++cnt;
        }
        while (i < n1) {
            data[k++] = L[i++];
            ++cnt;
        }
        while (j < n2) {
            data[k++] = R[j++];
            ++cnt;
        }
        };

    // Merge sorted runs bottom-up, doubling the merge size each iteration
    for (int size = RUN; size < n; size *= 2) {
        for (int left = 0; left < n; left += 2 * size) {
            int mid = std::min(left + size - 1, n - 1);
            int right = std::min(left + 2 * size - 1, n - 1);
            if (mid < right) mergeRange(left, mid, right);
        }
    }
}

static void bitonicCompare(std::vector<int>& a, Counter& cnt, int i, int j, bool dir) {
    if (dir == (a[i] > a[j])) {
        std::swap(a[i], a[j]);
        ++cnt;
    }
}

static void bitonicMerge(std::vector<int>& a, Counter& cnt, int low, int c, bool dir) {
    if (c > 1) {
        int k = c / 2;
        for (int i = low; i < low + k; i++) bitonicCompare(a, cnt, i, i + k, dir);
        bitonicMerge(a, cnt, low, k, dir);
        bitonicMerge(a, cnt, low + k, k, dir);
    }
}

static void bitonicSortRec(std::vector<int>& a, Counter& cnt, int low, int c, bool dir) {
    if (c > 1) {
        int k = c / 2;
        bitonicSortRec(a, cnt, low, k, true);
        bitonicSortRec(a, cnt, low + k, k, false);
        bitonicMerge(a, cnt, low, c, dir);
    }
}

// Bitonic Sort
// Time:  O(n log² n) — best, average, worst
// Space: O(log² n)   — recursion stack
// Comparison network sort: builds a bitonic sequence then repeatedly merges.
// Requires n to be a power of 2; highly parallelizable on GPU/hardware.
// Falls back to std::sort if n is not a power of 2.
void bitonicSort(std::vector<int>& data, Counter& cnt) {
    int n = static_cast<int>(data.size());
    auto isPow2 = [&](int x) { return x && ((x & (x - 1)) == 0); };
    if (!isPow2(n)) {
        std::sort(data.begin(), data.end());
        return;
    }
    bitonicSortRec(data, cnt, 0, n, true);
}

// Tree Sort  (BST Sort)
// Time:  O(n log n) average  |  O(n²) worst (sorted input → degenerate BST)
// Space: O(n)  — binary search tree nodes allocated on the heap
// Inserts all elements into a BST, then reads back via in-order traversal.
// Worst case occurs when input is already sorted (tree degenerates into a linked list).
void treeSort(std::vector<int>& data, Counter& cnt) {
    struct Node {
        int key;
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;
    auto insert = [&](int key) {
        Node** cur = &root;
        while (*cur) {
            if (key < (*cur)->key) cur = &((*cur)->left);
            else cur = &((*cur)->right);
        }
        *cur = new Node(key);
        };

    for (int v : data) insert(v);

    // In-order traversal writes sorted elements back to the array
    std::stack<Node*> st;
    Node* curr = root;
    int idx = 0;
    while (curr || !st.empty()) {
        while (curr) { st.push(curr); curr = curr->left; }
        curr = st.top(); st.pop();
        data[idx++] = curr->key;
        ++cnt;
        curr = curr->right;
    }

    // Free all BST nodes
    if (root) {
        std::stack<Node*> del;
        del.push(root);
        while (!del.empty()) {
            Node* n = del.top(); del.pop();
            if (n->left)  del.push(n->left);
            if (n->right) del.push(n->right);
            delete n;
        }
    }
}

// ══════════════════════════════════════════════
// ANALYSIS
// ══════════════════════════════════════════════

void analyzeSortingPerformance() {
    std::vector<int> sizes = { 5000, 10000, 15000, 20000, 25000 };

    using SortFn = std::function<void(std::vector<int>&, Counter&)>;
    std::vector<std::tuple<std::string, SortFn, bool>> algorithms = {
        {"Swap Sort",       swapSort,         true },
        {"Bubble Sort",     bubbleSort,        true },
        {"Selection Sort",  selectionSort,     true },
        {"Insertion Sort",  insertionSort,     false},
        {"Merge Sort",      mergeSortWrapper,  false},
        {"Quick Sort",      quickSortWrapper,  true },
        {"Bucket Sort",     bucketSort,        false},
        {"Intro Sort",      introSort,         false},
        {"Tim Sort",        timSort,           false},
        {"Bitonic Sort",    bitonicSort,       true },
        {"Tree Sort",       treeSort,          false},
        {"Count Sort",      countSort,         false},
        {"Radix Sort",      radixSort,         false},
        {"Heap Sort",       heapSort,          true },
        {"Shell Sort",      shellSort,         false},
        {"Cocktail Sort",   cocktailSort,      true },
        {"Comb Sort",       combSort,          true },
        {"Gnome Sort",      gnomeSort,         true },
        {"Pigeonhole Sort", pigeonholeSort,    false},
    };

    const int NAME_W = 18;
    const int CELL_W = 26;

    auto printTable = [&](
        const std::string& title,
        auto dataGenFunc,
        bool includeRecQuickSort = false)
        {
            std::cout << "\n\n>>> " << title << ":\n";
            std::cout << "  (sw = swaps,  wr = element writes)\n";

            std::vector<std::tuple<std::string, SortFn, bool>> current;
            if (includeRecQuickSort) {
                for (const auto& [name, fn, isSwap] : algorithms) {
                    if (name == "Quick Sort") {
                        current.emplace_back("Quick Sort (Rec)", quickSortRecWrapper, true);
                        current.emplace_back("Quick Sort (Iter)", quickSortWrapper, true);
                    }
                    else {
                        current.emplace_back(name, fn, isSwap);
                    }
                }
            }
            else {
                current = algorithms;
            }

            // Header
            std::cout << std::left << std::setw(NAME_W) << "Algorithm" << " |";
            for (int s : sizes) {
                std::string lbl = "n=" + std::to_string(s);
                std::cout << centre(lbl, CELL_W) << "|";
            }
            std::cout << "\n";

            // Separator
            std::cout << std::string(NAME_W, '-') << "-|";
            for (size_t i = 0; i < sizes.size(); i++) {
                std::cout << std::string(CELL_W, '-') << "|";
            }
            std::cout << "\n";

            // Rows
            for (const auto& [name, fn, isSwap] : current) {
                std::cout << std::left << std::setw(NAME_W) << name << " |";

                for (int size : sizes) {
                    std::vector<int> testData = dataGenFunc(size);
                    auto [ms, count] = measureExecutionTime(fn, testData);

                    std::ostringstream cell;
                    cell << std::fixed << std::setprecision(2) << ms << " ms"
                        << "  " << fmtNum(count) << (isSwap ? " sw" : " wr");

                    std::cout << centre(cell.str(), CELL_W) << "|";
                }
                std::cout << "\n";
            }
        };

    printTable("Testing on vectors with randomly generated numbers",
        [](int s) { return generateRandomVector(s); }, true);
    printTable("Testing on ascending sorted vectors",
        [](int s) { return generateSortedVector(s); }, false);
    printTable("Testing on descending sorted vectors",
        [](int s) { return generateReverseSortedVector(s); }, false);
}

int main() {
    analyzeSortingPerformance();
    std::cout << "\nAnalysis complete!\n";
    return 0;
}