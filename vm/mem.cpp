#include <iostream>
#include <vector>
namespace vm{
    using index_type=size_t;
    class RawMem
    {
    public:
        char *mem;
        template<typename T>
        T& get_val(index_type pos)
        {
            return *(T*)(mem+pos);
        }
        index_type capacity;
        void alloc();        
    };
}