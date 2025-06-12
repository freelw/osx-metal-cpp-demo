#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <Foundation/Foundation.hpp>
#include <iostream>
#include <vector>
#include <type_traits>

int main() {
    // 创建 Metal 设备
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "无法获取 Metal 设备" << std::endl;
        return 1;
    }

    // 创建命令队列
    MTL::CommandQueue* commandQueue = device->newCommandQueue();

    // 编写计算着色器
    const char* shaderSource = R"(
        #include <metal_stdlib>
        using namespace metal;
        
        kernel void add_arrays(device const float* inA [[buffer(0)]],
                              device const float* inB [[buffer(1)]],
                              device float* result [[buffer(2)]],
                              uint index [[thread_position_in_grid]]) {
            result[index] = inA[index] + inB[index];
        }
    )";

    // 编译着色器
    MTL::Library* library = nullptr;
    NS::Error* error = nullptr;
    library = device->newLibrary(NS::String::string(shaderSource, NS::StringEncoding::UTF8StringEncoding), nullptr, &error);

    if (!library) {
        std::cerr << "编译着色器失败: " << error->localizedDescription()->utf8String() << std::endl;
        return 1;
    }

    MTL::Function* function = library->newFunction(NS::String::string("add_arrays", NS::StringEncoding::UTF8StringEncoding));

    // 创建计算管线
    MTL::ComputePipelineState* pipelineState = device->newComputePipelineState(function, &error);
    if (!pipelineState) {
        std::cerr << "创建计算管线失败: " << error->localizedDescription()->utf8String() << std::endl;
        return 1;
    }

    // 准备数据
    const int arrayLength = 1024;
    std::vector<float> a(arrayLength), b(arrayLength), result(arrayLength);

    for (int i = 0; i < arrayLength; i++) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(i * 2);
    }

    // 创建 GPU 缓冲区
    MTL::Buffer* bufferA = device->newBuffer(a.data(), arrayLength * sizeof(float), MTL::ResourceStorageModeShared);
    MTL::Buffer* bufferB = device->newBuffer(b.data(), arrayLength * sizeof(float), MTL::ResourceStorageModeShared);
    MTL::Buffer* bufferResult = device->newBuffer(result.size() * sizeof(float), MTL::ResourceStorageModeShared);

    // 创建命令缓冲区和编码器
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();

    // 设置计算管线和缓冲区
    encoder->setComputePipelineState(pipelineState);
    encoder->setBuffer(bufferA, 0, 0);
    encoder->setBuffer(bufferB, 0, 1);
    encoder->setBuffer(bufferResult, 0, 2);

    // 设置线程组和网格大小
    MTL::Size threadGroupSize = MTL::Size(64, 1, 1);
    MTL::Size threadGridSize = MTL::Size(arrayLength, 1, 1);
    encoder->dispatchThreads(threadGridSize, threadGroupSize);

    // 结束编码并提交命令
    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    // 读取结果
    float* resultData = static_cast<float*>(bufferResult->contents());
    for (int i = 0; i < 10; i++) {
        std::cout << resultData[i] << " ";
    }
    std::cout << std::endl;

    // 释放资源
    bufferA->release();
    bufferB->release();
    bufferResult->release();
    encoder->release();
    commandBuffer->release();
    pipelineState->release();
    function->release();
    library->release();
    commandQueue->release();
    device->release();

    return 0;
}

