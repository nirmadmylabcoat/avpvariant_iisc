#pragma once  // Ensures this header is only included once

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace boost::interprocess;

// Shared synchronization barrier block
struct SyncBlock {
    interprocess_mutex mutex;         // Mutex to ensure thread-safe access
    interprocess_semaphore sem{0};    // Semaphore to signal waiting threads
    int arrived = 0;                  // Counter for arrivals

    // Wait for all parties to reach this point
    void arrive_and_wait(int total) {
        bool last = false;
        {
            scoped_lock<interprocess_mutex> lock(mutex);
            arrived++;
            if (arrived == total) {
                last = true;
                for (int i = 0; i < total; ++i)
                    sem.post();  // Wake all threads
            }
        }
        sem.wait();  // Wait for signal
    }

    // Optional reset method (can be used after a round)
    void reset() {
        scoped_lock<interprocess_mutex> lock(mutex);
        arrived = 0;
    }
};

// Gets or creates the shared sync block
inline SyncBlock* get_sync_block(bool create = false) {
    static managed_shared_memory* segment = nullptr;
    if (!segment) {
        if (create) {
            segment = new managed_shared_memory(create_only, "SharedSync", 65536);
        } else {
            segment = new managed_shared_memory(open_only, "SharedSync");
        }
    }
    return segment->find_or_construct<SyncBlock>("sync_block")();
}


// Optional cleanup (called from coordinator if needed)
inline void cleanup_sync_block() {
    shared_memory_object::remove("SharedSync");
}
