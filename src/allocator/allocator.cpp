#include <iostream>
#include <vector>
#include <limits>
#include "../../include/block.h"

using namespace std;

/* ================= GLOBAL STATE ================= */

static vector<Block> segments;
static size_t TOTAL_MEMORY = 0;
static int NEXT_ID = 1;

static int success_count = 0;
static int failure_count = 0;

/* ================= INTERNAL HELPERS ================= */

// Rebuilds memory by merging all adjacent free blocks
static void coalesce_free_segments() {
    if (segments.empty()) return;

    vector<Block> merged;
    merged.push_back(segments[0]);

    for (size_t i = 1; i < segments.size(); i++) {
        Block &last = merged.back();

        if (last.free && segments[i].free) {
            last.size += segments[i].size;
        } else {
            merged.push_back(segments[i]);
        }
    }

    segments = merged;
}

// Generic allocator used by all strategies
static int allocate_using_index(int index, size_t req) {
    int id = NEXT_ID++;

    Block &target = segments[index];

    size_t remaining = target.size - req;
    size_t base_addr = target.start;

    target.size = req;
    target.free = false;
    target.id = id;

    if (remaining > 0) {
        Block tail(
            base_addr + req,
            remaining,
            true,
            -1
        );
        segments.insert(segments.begin() + index + 1, tail);
    }

    success_count++;
    return id;
}

/* ================= PUBLIC API ================= */

void init_memory(size_t size) {
    segments.clear();
    TOTAL_MEMORY = size;
    NEXT_ID = 1;
    success_count = 0;
    failure_count = 0;

    segments.emplace_back(0, size, true, -1);

    cout << "[INIT] Memory initialized with " << size << " units\n";
}

/* ---------------- FIRST FIT ---------------- */

int first_fit_malloc(size_t req) {
    for (size_t i = 0; i < segments.size(); i++) {
        if (segments[i].free && segments[i].size >= req) {
            int id = allocate_using_index(i, req);
            cout << "[FIRST FIT] Allocated block " << id << "\n";
            return id;
        }
    }

    failure_count++;
    cout << "[FIRST FIT] Allocation failed\n";
    return -1;
}

/* ---------------- BEST FIT ---------------- */

int best_fit_malloc(size_t req) {
    int chosen = -1;
    size_t best_size = numeric_limits<size_t>::max();

    for (size_t i = 0; i < segments.size(); i++) {
        if (segments[i].free && segments[i].size >= req) {
            if (segments[i].size < best_size) {
                best_size = segments[i].size;
                chosen = i;
            }
        }
    }

    if (chosen == -1) {
        failure_count++;
        cout << "[BEST FIT] Allocation failed\n";
        return -1;
    }

    int id = allocate_using_index(chosen, req);
    cout << "[BEST FIT] Allocated block " << id << "\n";
    return id;
}

/* ---------------- WORST FIT ---------------- */

int worst_fit_malloc(size_t req) {
    int chosen = -1;
    size_t worst_size = 0;

    for (size_t i = 0; i < segments.size(); i++) {
        if (segments[i].free && segments[i].size >= req) {
            if (segments[i].size > worst_size) {
                worst_size = segments[i].size;
                chosen = i;
            }
        }
    }

    if (chosen == -1) {
        failure_count++;
        cout << "[WORST FIT] Allocation failed\n";
        return -1;
    }

    int id = allocate_using_index(chosen, req);
    cout << "[WORST FIT] Allocated block " << id << "\n";
    return id;
}

/* ---------------- FREE ---------------- */

void free_block(int id) {
    bool found = false;

    for (auto &seg : segments) {
        if (!seg.free && seg.id == id) {
            seg.free = true;
            seg.id = -1;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "[FREE] Invalid block id\n";
        return;
    }

    coalesce_free_segments();
    cout << "[FREE] Block " << id << " released\n";
}

/* ---------------- DUMP ---------------- */

void dump_memory() {
    cout << "\n--- Memory Layout ---\n";

    for (auto &seg : segments) {
        cout << "[0x" << hex << seg.start
             << " - 0x" << (seg.start + seg.size - 1) << "] ";

        if (seg.free)
            cout << "FREE\n";
        else
            cout << "USED (id=" << dec << seg.id << ")\n";
    }

    cout << dec;
}

/* ---------------- STATS ---------------- */

void print_stats() {
    size_t used = 0;
    size_t free_mem = 0;
    size_t largest_gap = 0;

    for (auto &seg : segments) {
        if (seg.free) {
            free_mem += seg.size;
            largest_gap = max(largest_gap, seg.size);
        } else {
            used += seg.size;
        }
    }

    cout << "\n--- Memory Statistics ---\n";
    cout << "Total Memory: " << TOTAL_MEMORY << "\n";
    cout << "Used Memory : " << used << "\n";
    cout << "Free Memory : " << free_mem << "\n";

    double utilization = TOTAL_MEMORY ? (double)used / TOTAL_MEMORY : 0.0;
    cout << "Utilization : " << utilization * 100 << "%\n";

    if (free_mem > 0) {
        double ext = 1.0 - ((double)largest_gap / free_mem);
        cout << "External Fragmentation: " << ext * 100 << "%\n";
    } else {
        cout << "External Fragmentation: 0%\n";
    }

    cout << "Internal Fragmentation: 0%\n";
    cout << "Alloc Success: " << success_count << "\n";
    cout << "Alloc Failure: " << failure_count << "\n";
}
