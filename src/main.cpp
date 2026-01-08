#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

/* -------- Allocator APIs (implemented elsewhere) -------- */

void init_memory(size_t size);

int first_fit_malloc(size_t size);
int best_fit_malloc(size_t size);
int worst_fit_malloc(size_t size);

void free_block(int id);
void dump_memory();
void print_stats();

/* -------- Allocation mode abstraction -------- */

enum class AllocatorMode {
    FIRST,
    BEST,
    WORST
};

/* -------- Controller class (NEW STRUCTURE) -------- */

class SimulatorController {
private:
    AllocatorMode mode;

public:
    SimulatorController() {
        mode = AllocatorMode::FIRST;
    }

    void showBanner() {
        cout << "\n===== Memory Management Simulator =====\n";
        cout << "Available commands:\n";
        cout << "  init memory <size>\n";
        cout << "  set allocator <first|best|worst>\n";
        cout << "  malloc <size>\n";
        cout << "  free <id>\n";
        cout << "  dump\n";
        cout << "  stats\n";
        cout << "  exit\n\n";
    }

    int allocateMemory(size_t size) {
        switch (mode) {
            case AllocatorMode::FIRST:
                return first_fit_malloc(size);
            case AllocatorMode::BEST:
                return best_fit_malloc(size);
            case AllocatorMode::WORST:
                return worst_fit_malloc(size);
        }
        return -1;
    }

    void setAllocator(const string& type) {
        if (type == "first") {
            mode = AllocatorMode::FIRST;
            cout << "[INFO] Allocation strategy: First Fit\n";
        } 
        else if (type == "best") {
            mode = AllocatorMode::BEST;
            cout << "[INFO] Allocation strategy: Best Fit\n";
        } 
        else if (type == "worst") {
            mode = AllocatorMode::WORST;
            cout << "[INFO] Allocation strategy: Worst Fit\n";
        } 
        else {
            cout << "[ERROR] Unknown allocator type\n";
        }
    }

    bool executeCommand(const string& input) {
        stringstream parser(input);
        string command;
        parser >> command;

        if (command == "exit") {
            cout << "Simulator terminated.\n";
            return false;
        }

        else if (command == "init") {
            string target;
            size_t size;
            parser >> target >> size;

            if (target == "memory" && size > 0) {
                init_memory(size);
                cout << "[OK] Memory initialized (" << size << " units)\n";
            } else {
                cout << "Usage: init memory <size>\n";
            }
        }

        else if (command == "set") {
            string target, type;
            parser >> target >> type;

            if (target == "allocator") {
                setAllocator(type);
            } else {
                cout << "Usage: set allocator <first|best|worst>\n";
            }
        }

        else if (command == "malloc") {
            size_t size;
            parser >> size;

            if (size > 0) {
                int blockId = allocateMemory(size);
                if (blockId != -1) {
                    cout << "[ALLOC SUCCESS] Block ID: " << blockId << "\n";
                } else {
                    cout << "[ALLOC FAIL] Insufficient memory\n";
                }
            } else {
                cout << "Usage: malloc <size>\n";
            }
        }

        else if (command == "free") {
            int id;
            parser >> id;

            if (id >= 0) {
                free_block(id);
                cout << "[FREE] Block " << id << " released\n";
            } else {
                cout << "Usage: free <id>\n";
            }
        }

        else if (command == "dump") {
            dump_memory();
        }

        else if (command == "stats") {
            print_stats();
        }

        else {
            cout << "[ERROR] Invalid command\n";
        }

        return true;
    }
};

/* -------- Program Entry -------- */

int main() {
    SimulatorController simulator;
    simulator.showBanner();

    string inputLine;

    while (true) {
        cout << ">> ";
        getline(cin, inputLine);

        if (inputLine.empty())
            continue;

        if (!simulator.executeCommand(inputLine))
            break;
    }

    return 0;
}
