#include "mc/math/Vec3.h"
#include <functional>
#include <future>

class ChunkTraverser {
private:
    std::future<double> futureResult;
    bool                isTraversing       = false;
    int                 count              = 0;
    bool                _canGetElapsedTime = false;

    // 私有构造函数
    ChunkTraverser() : isTraversing(false) {}

    // 禁止拷贝构造函数和赋值操作符
    ChunkTraverser(const ChunkTraverser&)            = delete;
    ChunkTraverser& operator=(const ChunkTraverser&) = delete;

    double traverseChunkBlocksAsync(const Vec3& min, const Vec3& max, const std::function<void(const Vec3&)>& callback);

public:
    static ChunkTraverser& getInstance();

    void startTraversal(const Vec3& min, const Vec3& max, const std::function<void(const Vec3&)>& callback);

    double getElapsedTime();

    int getCount();

    bool canGetElapsedTime();
};