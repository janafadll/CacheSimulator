
// Created by Jana Fadl on 20/07/2024.

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

struct CacheLine {
    bool valid;
    unsigned int tag;
};

CacheLine* cacheDM;
CacheLine* cacheFA;
int cacheLineCountDM;
int cacheLineCountFA;

void initializeCacheDM(int cache_size) {
    cacheLineCountDM = cache_size;
    cacheDM = new CacheLine[cacheLineCountDM];
    for (int i = 0; i < cacheLineCountDM; i++) {
        cacheDM[i].valid = false;
        cacheDM[i].tag = 0;
    }
}

void initializeCacheFA(int cache_size) {
    cacheLineCountFA = cache_size;
    cacheFA = new CacheLine[cacheLineCountFA];
    for (int i = 0; i < cacheLineCountFA; i++) {
        cacheFA[i].valid = false;
        cacheFA[i].tag = 0;
    }
}

// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr, int cacheLineSize)
{
    int index = (addr / cacheLineSize) % cacheLineCountDM;
    unsigned int tag = addr / (cacheLineSize * cacheLineCountDM);
    if (cacheDM[index].valid && cacheDM[index].tag == tag) {
        return HIT;
    } else {
        cacheDM[index].valid = true;
        cacheDM[index].tag = tag;
        return MISS;
    }
}

// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr, int cacheLineSize)
{
    unsigned int tag = addr / cacheLineSize;
    for (int i = 0; i < cacheLineCountFA; i++) {
        if (cacheFA[i].valid && cacheFA[i].tag == tag) {
            return HIT;
        }
    }
    int replace_index = rand_() % cacheLineCountFA;
    cacheFA[replace_index].valid = true;
    cacheFA[replace_index].tag = tag;
    return MISS;
}

char* msg[2] = { "Miss", "Hit" };

#define NO_OF_Iterations 1000000 // Change to 1,000,000 for actual tests

int main()
{
    //srand(time(0)); // Seed for random replacement

    unsigned int hit;
    cacheResType r;
    unsigned int addr;

    int cacheLineSizes[] = { 16, 32, 64, 128 };
    unsigned int(*memGens[])() = { memGen1, memGen2, memGen3, memGen4, memGen5, memGen6 };

    for (int memGenIdx = 0; memGenIdx < 6; memGenIdx++) {
        for (int cls : cacheLineSizes) {
            cout << "Direct Mapped Cache Simulator for Cache Line Size: " << cls << " bytes, Memory Generator: memGen" << (memGenIdx + 1) << endl;
            initializeCacheDM(CACHE_SIZE / cls);
            hit = 0;

            for (int inst = 0; inst < NO_OF_Iterations; inst++) {
                addr = memGens[memGenIdx]();
                r = cacheSimDM(addr, cls);
                if (r == HIT) hit++;
              //  if (DBG) {
             //       cout << "0x" << setfill('0') << setw(8) << hex << addr << " (" << msg[r] << ")\n";
              //  }
            }
            cout << "Hit ratio = " << (100.0 * hit / NO_OF_Iterations) << "%" << endl;
            delete[] cacheDM;
        }
    }

    for (int memGenIdx = 0; memGenIdx < 6; memGenIdx++) {
        for (int cls : cacheLineSizes) {
            cout << "Fully Associative Cache Simulator for Cache Line Size: " << cls << " bytes, Memory Generator: memGen" << (memGenIdx + 1) << endl;
            initializeCacheFA(CACHE_SIZE / cls);
            hit = 0;

            for (int inst = 0; inst < NO_OF_Iterations; inst++) {
                addr = memGens[memGenIdx]();
                r = cacheSimFA(addr, cls);
                if (r == HIT) hit++;
              //  if (DBG) {
              //      cout << "0x" << setfill('0') << setw(8) << hex << addr << " (" << msg[r] << ")\n";
               // }
            }
            cout << "Hit ratio = " << (100.0 * hit / NO_OF_Iterations) << "%" << endl;
            delete[] cacheFA;
        }
    }

    return 0;
}