#include <iostream>

class MemoryPool {
 public:
  MemoryPool(size_t size, size_t count) : blocksize(size), blockcount(count) {
    memory = (char *)malloc(blocksize * blockcount);
    freelist = memory;  // 初始化，freelist指向内存池第一块内容
    char *curr_block = memory;
    for (size_t i = 0; i < count - 1; ++i) {
      // curr_block 是 char*，指向当前块的起始地址。
      // (void**)curr_block 把它转成 void**，表示“这里放一个指针”。
      // *(void**)curr_block = curr_block + blocksize;
      // 就是在当前块的头部写下一个块的地址。

      *(void **)curr_block = (void *)(curr_block + blocksize);
      curr_block += blocksize;
    }
    *(void **)curr_block = nullptr;
  }

  ~MemoryPool() { free(memory); }

  void *allocateBlock() {
    if (freelist == nullptr) {
      return nullptr;
    }
    void *block = freelist;
    freelist = *(void **)block;  // freelist指向下一个指针
    return block;
  }

  void deallocate(void *ptr) {
    // 检查内存是否在内存池的范围之内
    if (!(ptr >= memory && ptr < memory + blocksize * blockcount)) {
      return;
    }
    // 将块插入空闲链表的头部
    *(void **)ptr = freelist;
    freelist = ptr;
  }

 private:
  char *memory = nullptr;
  void *freelist = nullptr;
  size_t blocksize;
  size_t blockcount;
};

int main() {
  {
    MemoryPool pool(sizeof(int *), 1000);
    int *p = (int *)pool.allocateBlock();

    *p = 10;

    int *p2 = (int *)pool.allocateBlock();
    *p2 = *p;

    std::cout << "&p = " << p << " *p = " << *p << std::endl;
    std::cout << "&p2 = " << p2 << " *p2 = " << *p2 << std::endl;
  }
}