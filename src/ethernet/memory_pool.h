#ifndef _RS_MEMORY_POOL_HH
#define _RS_MEMORY_POOL_HH

#include <queue>
#include <iostream>
#include "../../tools/rs-server/RsCommon.hh"

#define POOL_SIZE 200 //TODO:: to define the right value
#define MAX_FRAME_SIZE 1280 * 720 * 3 //TODO:: to define the right value
class memory_pool
{

public:
    memory_pool()
    {
        //alloc memory
        for (int i = 0; i < POOL_SIZE; i++)
        {
            unsigned char *mem = new unsigned char[sizeof(rs_over_ethernet_data_header) + MAX_FRAME_SIZE]; //TODO:to use OutPacketBuffer::maxSize;
            pool.push(mem);
        }
        std::cout<<"memory_pool: pool size is: "<<pool.size()<<"\n";
    }

    unsigned char *getNextMem()
    {
        unsigned char *mem = nullptr;

        if (!pool.empty())
        {
            mem = pool.front();
            pool.pop();
        }
        else
        {
            std::cout<< "pool is empty\n";
        }
        //printf("memory_pool:  after getMem: pool size is: %d, mem is %p\n",pool.size(),mem);
        return mem;
    }

    void returnMem(unsigned char* mem)
    {
        if (mem != nullptr)
        {
            pool.push(mem);
            //printf("memory_pool: after returnMem: pool size is: %d, mem is %p\n",pool.size(),mem);
        }
        else
        {
            std::cout << "returnMem:invalid mem\n";
        }
    }

    ~memory_pool()
    {
        while (!pool.empty())
        {
            unsigned char *mem = pool.front();
            //printf("memory_pool: ~memory_pool:not empty, mem is %p\n",mem);
            pool.pop();
            if (mem != nullptr)
            {
                delete mem;
            }
            else
            {
                std::cout << "~memory_pool:invalid mem\n";
            }
        }
    }

private:
    std::queue<unsigned char*> pool;
};

#endif
