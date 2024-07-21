
// Created by Jana Fadl on 19/07/2024.

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

#define DBG 1
#define DRAM_SIZE (64 * 1024 * 1024)
#define CACHE_SIZE (64 * 1024)

enum cacheResType { MISS = 0, HIT = 1 };

// The following implements a random number generator
unsigned int m_w = 0xABABAB55; // must not be zero, nor 0x464fffff
unsigned int m_z = 0x05080902; // must not be zero, nor 0x9068ffff
unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w; // 32-bit result
}

unsigned int memGen1()
{
    static unsigned int addr = 0;
    return (addr++) % DRAM_SIZE;
}

unsigned int memGen2()
{
    static unsigned int addr = 0;
    return rand_() % (24 * 1024);
}

unsigned int memGen3()
{
    return rand_() % DRAM_SIZE;
}

unsigned int memGen4()
{
    static unsigned int addr = 0;
    return (addr++) % (4 * 1024);
}

unsigned int memGen5()
{
    static unsigned int addr = 0;
    return (addr++) % (1024 * 64);
}

unsigned int memGen6()
{
    static unsigned int addr = 0;
    return (addr += 32) % (64 * 4 * 1024);
}

// Define a struct for cache lines
struct CacheLine {
    bool valid;           // Valid bit
    unsigned int tag;     // Tag
};

// Declare pointers to cache arrays and cache line counts
CacheLine* cacheDM;       // Pointer to direct-mapped cache array
CacheLine* cacheFA;       // Pointer to fully associative cache array
int cacheLineCountDM;     // Number of cache lines in direct-mapped cache
int cacheLineCountFA;     // Number of cache lines in fully associative cache

// Function to initialize direct-mapped cache
void initializeCacheDM(int cache_size) {
    cacheLineCountDM = cache_size;               // Set the number of cache lines
    cacheDM = new CacheLine[cacheLineCountDM];   // Allocate memory for the cache lines
    for (int i = 0; i < cacheLineCountDM; i++) { // Initialize each cache line
        cacheDM[i].valid = false;                // Set valid bit to false
        cacheDM[i].tag = 0;                      // Initialize tag to 0
    }
}

// Function to initialize fully associative cache
void initializeCacheFA(int cache_size) {
    cacheLineCountFA = cache_size;               // Set the number of cache lines
    cacheFA = new CacheLine[cacheLineCountFA];   // Allocate memory for the cache lines
    for (int i = 0; i < cacheLineCountFA; i++) { // Initialize each cache line
        cacheFA[i].valid = false;                // Set valid bit to false
        cacheFA[i].tag = 0;                      // Initialize tag to 0
    }
}

// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr, int cacheLineSize)
{
    int index = (addr / cacheLineSize) % cacheLineCountDM;       // Calculate index
    unsigned int tag = addr / (cacheLineSize * cacheLineCountDM); // Calculate tag
    if (cacheDM[index].valid && cacheDM[index].tag == tag) {     // Check if valid and tags match
        return HIT;                                              // Return HIT if match
    } else {
        cacheDM[index].valid = true;                             // Set valid bit to true
        cacheDM[index].tag = tag;                                // Update tag
        return MISS;                                             // Return MISS
    }
}

// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr, int cacheLineSize)
{
    unsigned int tag = addr / cacheLineSize;     // Calculate tag
    for (int i = 0; i < cacheLineCountFA; i++) { // Search for the tag in the cache
        if (cacheFA[i].valid && cacheFA[i].tag == tag) {
            return HIT;                          // Return HIT if found
        }
    }
    int replace_index = rand_() % cacheLineCountFA; // Choose a random cache line to replace
    cacheFA[replace_index].valid = true;            // Set valid bit to true
    cacheFA[replace_index].tag = tag;               // Update tag
    return MISS;                                    // Return MISS
}

// Array of result messages
char* msg[2] = { "Miss", "Hit" };

#define NO_OF_Iterations 1000000 // Define the number of iterations

// Function to run the cache simulation
void runSimulation(int iterations, int* cacheLineSizes, int numCacheSizes, unsigned int(**memGens)(), int numMemGens)
{
    unsigned int hit;      // Variable to count hits
    cacheResType r;        // Variable to store cache result
    unsigned int addr;     // Variable to store memory address

    // Loop over each memory generator
    for (int memGenIdx = 0; memGenIdx < numMemGens; memGenIdx++) {
        // Loop over each cache line size
        for (int clsIdx = 0; clsIdx < numCacheSizes; clsIdx++) {
            int cls = cacheLineSizes[clsIdx];
            cout << "Direct Mapped Cache Simulator for Cache Line Size: " << cls << " bytes, Memory Generator: memGen" << (memGenIdx + 1) << endl;
            initializeCacheDM(CACHE_SIZE / cls); // Initialize the direct-mapped cache
            hit = 0;                            // Reset hit counter

            // Loop for each iteration
            for (int inst = 0; inst < iterations; inst++) {
                addr = memGens[memGenIdx]();     // Generate a memory address
                r = cacheSimDM(addr, cls);       // Simulate cache access
                if (r == HIT) hit++;             // Increment hit counter if HIT
            }
            cout << "Hit ratio = " << (100.0 * hit / iterations) << "%" << endl; // Print hit ratio
            delete[] cacheDM; // Deallocate memory
        }
    }

    // Loop over each memory generator for fully associative cache
    for (int memGenIdx = 0; memGenIdx < numMemGens; memGenIdx++) {
        // Loop over each cache line size
        for (int clsIdx = 0; clsIdx < numCacheSizes; clsIdx++) {
            int cls = cacheLineSizes[clsIdx];
            cout << "Fully Associative Cache Simulator for Cache Line Size: " << cls << " bytes, Memory Generator: memGen" << (memGenIdx + 1) << endl;
            initializeCacheFA(CACHE_SIZE / cls); // Initialize the fully associative cache
            hit = 0;                            // Reset hit counter

            // Loop for each iteration
            for (int inst = 0; inst < iterations; inst++) {
                addr = memGens[memGenIdx]();     // Generate a memory address
                r = cacheSimFA(addr, cls);       // Simulate cache access
                if (r == HIT) hit++;             // Increment hit counter if HIT
            }
            cout << "Hit ratio = " << (100.0 * hit / iterations) << "%" << endl; // Print hit ratio
            delete[] cacheFA; // Deallocate memory
        }
    }
}

int main()
{
    // Define cache line sizes and memory generators
    int cacheLineSizes[] = { 16, 32, 64, 128 };
    unsigned int(*memGens[])() = { memGen1, memGen2, memGen3, memGen4, memGen5, memGen6 };

    // Run the simulation with different parameters
    runSimulation(NO_OF_Iterations, cacheLineSizes, 4, memGens, 6);

    return 0; // Return 0 to indicate successful execution
}
