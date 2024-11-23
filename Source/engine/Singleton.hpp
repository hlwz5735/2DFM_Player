#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <functional> // 包含 std::call_once 和 std::once_flag

template<typename T>
class Singleton {
public:
    // 禁用拷贝构造函数和赋值运算符
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // 获取单例实例的静态方法
    static T& getInstance() {
        std::call_once(instanceFlag, []() {
            instance = std::make_unique<T>();
        });
        return *instance;
    }

    // 虚析构函数，确保派生类的正确析构（可选，如果 T 是多态类型）
    virtual ~Singleton() = default;

protected:
    // 构造函数被保护，防止外部直接实例化
    Singleton() = default;

private:
    // 静态实例指针和一次标志
    static std::unique_ptr<T> instance;
    static std::once_flag instanceFlag;
};

// 模板静态成员变量的定义和初始化
template<typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr; // 初始化为 nullptr（虽然 std::unique_ptr 默认初始化就是 nullptr）
template<typename T>
std::once_flag Singleton<T>::instanceFlag;
