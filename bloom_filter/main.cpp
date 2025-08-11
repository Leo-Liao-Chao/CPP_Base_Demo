#include <iostream>
#include <vector>
#include <string>
#include <functional>

template<typename T>
class BloomFilter {
public:
    BloomFilter(size_t size, size_t numHashes)
        : bitArray(size), hashCount(numHashes), bitArraySize(size) {}

    void add(const T& item) {
        for (size_t i = 0; i < hashCount; ++i) {
            size_t hashValue = hash(item, i);
            bitArray[hashValue % bitArraySize] = true;
        }
    }

    bool possiblyContains(const T& item) const {
        for (size_t i = 0; i < hashCount; ++i) {
            size_t hashValue = hash(item, i);
            if (!bitArray[hashValue % bitArraySize]) {
                return false; // definitely not in set
            }
        }
        return true; // possibly in set
    }

private:
    std::vector<bool> bitArray;
    size_t hashCount;
    size_t bitArraySize;

    // 使用 std::hash + i 模拟多个哈希函数
    size_t hash(const T& item, size_t i) const {
        return std::hash<T>{}(item) ^ (i * 0x9e3779b9); // std::hash<T>{}(item) 生成hash值，然后与 i 结合
    }
};

int main() {
    BloomFilter<std::string> filter(1000, 3); // 1000 bits, 3 hash functions

    filter.add("apple");
    filter.add("banana");

    std::cout << std::boolalpha;

    std::cout << "apple: " << filter.possiblyContains("apple") << "\n";    // true
    std::cout << "banana: " << filter.possiblyContains("banana") << "\n";  // true
    std::cout << "grape: " << filter.possiblyContains("grape") << "\n";    // false or true (false positive)
}
