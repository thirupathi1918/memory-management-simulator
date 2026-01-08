# Memory Management Simulator

This project implements a Memory Management Simulator demonstrating core operating system concepts such as dynamic memory allocation, multilevel cache simulation, and virtual memory management with disk interaction.

---

## Demo Video

The demo video demonstrating all features of the project is available at the link below:

📹 **Demo Video:**  
https://drive.google.com/drive/folders/1WDWwpefTTsx85qRZiynz_yzSBmKC9UkM?usp=sharing

---

## How to Run

### Memory Allocation Simulator
```bash
g++ src/main.cpp src/allocator/allocator.cpp src/buddy/buddy_allocator.cpp -o memsim.exe
./memsim.exe

###Cache Simulation
g++ src/cache/cache.cpp -o cache_test.exe
./cache_test.exe

###Virtual Memory Simulation
g++ src/virtual_memory/virtual_memory.cpp -o vm_test.exe
./vm_test.exe

