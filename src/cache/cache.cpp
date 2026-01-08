#include <iostream>
#include <vector>
#include <cmath>
#include <cstddef>

using namespace std;

/*
 MODIFIED MULTI-LEVEL CACHE SIMULATOR
 -----------------------------------
 - Two-level cache (L1 + L2)
 - Set associative
 - LRU in L1, FIFO in L2
 - Symbolic access timing
 - Modified access trace for originality
*/

// ---------- Replacement Policy ----------
enum class ReplacePolicy {
    FIFO,
    LRU
};

// ---------- Cache Line ----------
struct Line {
    bool valid;
    size_t tag;
    size_t stamp;

    Line() : valid(false), tag(0), stamp(0) {}
};

// ---------- Cache Level ----------
class Cache {
public:
    size_t cacheSize;
    size_t blockSize;
    size_t ways;
    size_t setsCount;

    vector<vector<Line>> sets;
    ReplacePolicy policy;

    size_t hits;
    size_t misses;
    size_t clock;
    size_t latency;

    Cache(size_t c, size_t b, size_t w,
          ReplacePolicy p, size_t delay)
        : cacheSize(c), blockSize(b), ways(w),
          policy(p), hits(0), misses(0),
          clock(0), latency(delay) {

        setsCount = (cacheSize / blockSize) / ways;
        sets.resize(setsCount, vector<Line>(ways));
    }

    bool access(size_t addr) {
        clock++;

        size_t offsetBits = log2(blockSize);
        size_t indexBits  = log2(setsCount);

        size_t index = (addr >> offsetBits) & ((1 << indexBits) - 1);
        size_t tag   = addr >> (offsetBits + indexBits);

        auto &set = sets[index];

        // HIT
        for (auto &line : set) {
            if (line.valid && line.tag == tag) {
                hits++;
                if (policy == ReplacePolicy::LRU)
                    line.stamp = clock;
                return true;
            }
        }

        // MISS
        misses++;

        for (auto &line : set) {
            if (!line.valid) {
                line.valid = true;
                line.tag = tag;
                line.stamp = clock;
                return false;
            }
        }

        // Replacement
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

    double hitRate() const {
        size_t total = hits + misses;
        return total ? (double)hits / total : 0.0;
    }
};

// ---------- Cache System ----------
class CacheHierarchy {
public:
    Cache L1;
    Cache L2;
    size_t totalTime;

    CacheHierarchy()
        : L1(256, 32, 4, ReplacePolicy::LRU, 1),
          L2(1024, 64, 4, ReplacePolicy::FIFO, 8),
          totalTime(0) {}

    void access(size_t addr) {
        totalTime += L1.latency;
        if (L1.access(addr)) {
            cout << "L1 HIT\n";
            return;
        }

        totalTime += L2.latency;
        if (L2.access(addr)) {
            cout << "L2 HIT -> promoted to L1\n";
            L1.access(addr);
            return;
        }

        cout << "CACHE MISS -> Main Memory\n";
        totalTime += 80;

        L2.access(addr);
        L1.access(addr);
    }

    void stats() {
        cout << "\n--- Cache Performance ---\n";
        cout << "L1 Hits: " << L1.hits << "\n";
        cout << "L1 Misses: " << L1.misses << "\n";
        cout << "L1 Hit Rate: " << L1.hitRate() * 100 << "%\n\n";

        cout << "L2 Hits: " << L2.hits << "\n";
        cout << "L2 Misses: " << L2.misses << "\n";
        cout << "L2 Hit Rate: " << L2.hitRate() * 100 << "%\n\n";

        cout << "Total Access Time: " << totalTime << " cycles\n";
    }
};

// ---------- Driver ----------
int main() {
    CacheHierarchy cache;

    size_t trace[] = {
        64, 128, 256, 64, 512,
        128, 64, 768, 1024, 64,
        256, 128
    };

    cout << "=== MULTI-LEVEL CACHE SIMULATION ===\n\n";

    for (size_t addr : trace) {
        cout << "Access PA " << addr << " : ";
        cache.access(addr);
    }

    cache.stats();
    return 0;
}
