#include <iostream>
#include <vector>
#include <unordered_set>
#include <cstddef>
#include <climits>
#include <cmath>

using namespace std;

/*
 MODIFIED DISK-AWARE VIRTUAL MEMORY SIMULATOR
 -------------------------------------------
 - Paging based virtual memory
 - FIFO page replacement
 - Explicit page-in / page-out logging
 - Integrated two-level cache access
*/

// ================= CACHE SUBSYSTEM =================

enum class CachePolicy { FIFO, LRU };

struct CacheLine {
    bool valid;
    size_t tag;
    size_t stamp;
    CacheLine() : valid(false), tag(0), stamp(0) {}
};

class CacheLevel {
public:
    size_t cacheSize, blockSize, ways, setsCount;
    vector<vector<CacheLine>> sets;
    CachePolicy policy;
    size_t hits, misses, clock;

    CacheLevel(size_t c, size_t b, size_t w, CachePolicy p)
        : cacheSize(c), blockSize(b), ways(w),
          policy(p), hits(0), misses(0), clock(0) {

        setsCount = (cacheSize / blockSize) / ways;
        sets.resize(setsCount, vector<CacheLine>(ways));
    }

    bool access(size_t pa) {
        clock++;

        size_t offsetBits = log2(blockSize);
        size_t indexBits  = log2(setsCount);

        size_t index = (pa >> offsetBits) & ((1 << indexBits) - 1);
        size_t tag   = pa >> (offsetBits + indexBits);

        auto &set = sets[index];

        for (auto &line : set) {
            if (line.valid && line.tag == tag) {
                hits++;
                if (policy == CachePolicy::LRU)
                    line.stamp = clock;
                return true;
            }
        }

        misses++;

        for (auto &line : set) {
            if (!line.valid) {
                line.valid = true;
                line.tag = tag;
                line.stamp = clock;
                return false;
            }
        }

        size_t victim = 0;
        size_t minStamp = set[0].stamp;
        for (size_t i = 1; i < set.size(); i++) {
            if (set[i].stamp < minStamp) {
                minStamp = set[i].stamp;
                victim = i;
            }
        }

        set[victim].tag = tag;
        set[victim].stamp = clock;
        return false;
    }
};

class CacheSystem {
public:
    CacheLevel L1;
    CacheLevel L2;

    CacheSystem()
        : L1(256, 32, 4, CachePolicy::LRU),
          L2(1024, 64, 4, CachePolicy::FIFO) {}

    void access(size_t pa) {
        if (L1.access(pa)) {
            cout << "    Cache: L1 HIT\n";
            return;
        }
        if (L2.access(pa)) {
            cout << "    Cache: L2 HIT → promoted to L1\n";
            L1.access(pa);
            return;
        }

        cout << "    Cache: MISS → Main Memory\n";
        L2.access(pa);
        L1.access(pa);
    }
};

// ================= VIRTUAL MEMORY =================

struct PageEntry {
    bool valid;
    size_t frame;
    size_t time;
    PageEntry() : valid(false), frame(0), time(0) {}
};

class VirtualMemory {
public:
    size_t pageSize, pages, frames;
    vector<PageEntry> table;
    vector<int> frameMap;
    unordered_set<size_t> disk;
    size_t clock, hits, faults;

    CacheSystem cache;

    VirtualMemory(size_t vSize, size_t pSize, size_t pSizePg)
        : pageSize(pSizePg), clock(0), hits(0), faults(0) {

        pages  = vSize / pageSize;
        frames = pSize / pageSize;

        table.resize(pages);
        frameMap.resize(frames, -1);

        for (size_t i = 0; i < pages; i++)
            disk.insert(i);
    }

    void access(size_t va) {
        clock++;

        size_t page = va / pageSize;
        size_t off  = va % pageSize;

        cout << "VA " << va << " → ";

        if (table[page].valid) {
            hits++;
            table[page].time = clock;
            size_t pa = table[page].frame * pageSize + off;
            cout << "PA " << pa << " (PAGE HIT)\n";
            cache.access(pa);
            return;
        }

        faults++;
        cout << "PAGE FAULT\n";

        for (size_t f = 0; f < frames; f++) {
            if (frameMap[f] == -1) {
                page_in(page, f);
                size_t pa = f * pageSize + off;
                cache.access(pa);
                return;
            }
        }

        size_t victim = select_victim();
        size_t frame  = table[victim].frame;

        page_out(victim);
        page_in(page, frame);

        size_t pa = frame * pageSize + off;
        cout << "    Replaced page " << victim
             << " with page " << page << "\n";
        cache.access(pa);
    }

    void stats() {
        cout << "\n--- Virtual Memory Summary ---\n";
        cout << "Page Hits   : " << hits << "\n";
        cout << "Page Faults: " << faults << "\n";
        cout << "Pages on Disk: " << disk.size() << "\n";
    }

private:
    void page_in(size_t p, size_t f) {
        disk.erase(p);
        table[p].valid = true;
        table[p].frame = f;
        table[p].time = clock;
        frameMap[f] = p;
        cout << "    PAGE IN  : Disk → Memory (page " << p << ")\n";
    }

    void page_out(size_t p) {
        size_t f = table[p].frame;
        table[p].valid = false;
        frameMap[f] = -1;
        disk.insert(p);
        cout << "    PAGE OUT : Memory → Disk (page " << p << ")\n";
    }

    size_t select_victim() {
        size_t v = 0, t = SIZE_MAX;
        for (size_t i = 0; i < table.size(); i++) {
            if (table[i].valid && table[i].time < t) {
                t = table[i].time;
                v = i;
            }
        }
        return v;
    }
};

// ================= DRIVER =================

int main() {
    VirtualMemory vm(2048, 512, 64);

    size_t trace[] = {0, 128, 256, 512, 128, 0, 768, 256, 0};

    cout << "=== DISK-AWARE VIRTUAL MEMORY SIMULATION ===\n\n";

    for (size_t va : trace) {
        vm.access(va);
        cout << "\n";
    }

    vm.stats();
    return 0;
}
