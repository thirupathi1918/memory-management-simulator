#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstddef>

using namespace std;

/* ================= BUDDY SYSTEM STATE ================= */

static size_t TOTAL_SIZE = 0;
static size_t BASE_BLOCK = 32;
static int MAX_LEVEL = 0;

// freeBlocks[level] → list of free block base addresses
static vector<vector<size_t>> freeBlocks;

/* ================= INTERNAL UTILITIES ================= */

// normalize requested size to power-of-two block
static size_t normalize_size(size_t request) {
    size_t size = BASE_BLOCK;
    while (size < request)
        size <<= 1;
    return size;
}

// convert block size to level
static int size_to_level(size_t size) {
    int level = 0;
    size_t curr = BASE_BLOCK;
    while (curr < size) {
        curr <<= 1;
        level++;
    }
    return level;
}

// compute buddy using block size alignment
static size_t buddy_address(size_t addr, int level) {
    size_t blockSize = BASE_BLOCK << level;
    return addr ^ blockSize;
}

/* ================= INITIALIZATION ================= */

void buddy_init(size_t memorySize) {
    TOTAL_SIZE = memorySize;

    MAX_LEVEL = 0;
    size_t temp = memorySize / BASE_BLOCK;
    while ((1UL << MAX_LEVEL) < temp)
        MAX_LEVEL++;

    freeBlocks.clear();
    freeBlocks.resize(MAX_LEVEL + 1);

    freeBlocks[MAX_LEVEL].push_back(0);

    cout << "[BUDDY INIT] Memory size = " << memorySize << "\n";
}

/* ================= ALLOCATION ================= */

size_t buddy_malloc(size_t request) {
    size_t allocSize = normalize_size(request);
    int targetLevel = size_to_level(allocSize);

    if (targetLevel > MAX_LEVEL) {
        cout << "[BUDDY] Allocation failed (too large)\n";
        return SIZE_MAX;
    }

    int level = targetLevel;
    while (level <= MAX_LEVEL && freeBlocks[level].empty())
        level++;

    if (level > MAX_LEVEL) {
        cout << "[BUDDY] Allocation failed (no block)\n";
        return SIZE_MAX;
    }

    size_t addr = freeBlocks[level].back();
    freeBlocks[level].pop_back();

    while (level > targetLevel) {
        level--;
        size_t splitAddr = addr + (BASE_BLOCK << level);
        freeBlocks[level].push_back(splitAddr);
    }

    cout << "[BUDDY] Allocated block at " << addr
         << " (size " << allocSize << ")\n";

    return addr;
}

/* ================= DEALLOCATION ================= */

void buddy_free(size_t addr, size_t originalSize) {
    size_t size = normalize_size(originalSize);
    int level = size_to_level(size);

    while (level < MAX_LEVEL) {
        size_t buddy = buddy_address(addr, level);
        auto &list = freeBlocks[level];

        auto it = find(list.begin(), list.end(), buddy);
        if (it == list.end())
            break;

        list.erase(it);
        addr = min(addr, buddy);
        level++;
    }

    freeBlocks[level].push_back(addr);
    cout << "[BUDDY] Freed block at " << addr << "\n";
}

/* ================= DEBUG VIEW ================= */

void buddy_dump() {
    cout << "\n--- Buddy Free Lists ---\n";
    for (int lvl = 0; lvl <= MAX_LEVEL; lvl++) {
        cout << "Level " << lvl << " (" 
             << (BASE_BLOCK << lvl) << "): ";
        for (auto addr : freeBlocks[lvl])
            cout << addr << " ";
        cout << "\n";
    }
}
