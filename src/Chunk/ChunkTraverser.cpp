#include "ChunkTraverser.h"

// stl
#include <chrono>
#include <functional>
#include <future>
#include <iostream>

// mc
#include "mc/math/Vec3.h"

double ChunkTraverser::traverseChunkBlocksAsync(
    const Vec3&                             min,
    const Vec3&                             max,
    const std::function<void(const Vec3&)>& callback
) {
    auto start = std::chrono::high_resolution_clock::now(); // 开始时间

    count = 0;                                                   // 遍历的方块计数
    for (float y = max.y; y >= min.y; --y) {                     // 从上到下遍历
        bool isEvenLayer = static_cast<int>(max.y - y) % 2 == 0; // 判断当前层是偶数层还是奇数层
        if (isEvenLayer) {
            for (float z = min.z; z <= min.z + 15; ++z) {     // 偶数层，Z轴正向遍历
                for (float x = min.x; x <= min.x + 15; ++x) { // X轴正向遍历
                    callback(Vec3(x, y, z));                  // 调用回调函数处理当前方块
                    count++;
                }
            }
        } else {
            for (float z = min.z + 15; z >= min.z; --z) {     // 奇数层，Z轴反向遍历
                for (float x = min.x + 15; x >= min.x; --x) { // X轴反向遍历
                    callback(Vec3(x, y, z));                  // 调用回调函数处理当前方块
                    count++;
                }
            }
        }
    }

    auto                                      end     = std::chrono::high_resolution_clock::now(); // 结束时间
    std::chrono::duration<double, std::milli> elapsed = end - start;                               // 计算耗时
    return elapsed.count(); // 返回耗时（毫秒）
}


ChunkTraverser& ChunkTraverser::getInstance() {
    static ChunkTraverser instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
    return instance;
}

void ChunkTraverser::startTraversal(
    const Vec3&                             min,
    const Vec3&                             max,
    const std::function<void(const Vec3&)>& callback
) {
    if (isTraversing) { // 如果已经在遍历中，则不再开始新的遍历
        std::cout << "Traversal is already in progress." << std::endl;
        return;
    }
    isTraversing       = true;  // 标记为正在遍历
    _canGetElapsedTime = false; // 标记不能获取遍历耗时
    futureResult       = std::async(
        std::launch::async,
        &ChunkTraverser::traverseChunkBlocksAsync,
        this,
        min,
        max,
        callback
    );                    // 异步开始遍历
    isTraversing       = false; // 重置遍历标记
    _canGetElapsedTime = true;  // 标记可以获取遍历耗时
}

double ChunkTraverser::getElapsedTime() {
    if (!_canGetElapsedTime) { // 如果不能获取遍历耗时，则返回0
        std::cout << "Traversal has not been started." << std::endl;
        return 0.0;
    }
    double duration = futureResult.get(); // 获取遍历结果（耗时）
    return duration;                      // 返回耗时
}

int ChunkTraverser::getCount() {
    return count; // 返回遍历的方块数量
}

bool ChunkTraverser::canGetElapsedTime() {
    return _canGetElapsedTime; // 返回是否可以获取遍历耗时
}
