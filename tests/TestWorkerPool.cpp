#include <gtest/gtest.h>
#include "WorkerPool.h"

TEST(WorkerPoolTest, DoTasks) {
    bool arr[16] = {false};

    WorkerPool pool(16);
    for (int i = 0; i < 16; i++)
        pool.enqueue([&arr, i]()  {
            arr[i] = true;
        });
        
    pool.stop();
    
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(arr[i], true);
    }
}
