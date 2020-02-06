
#pragma once

#include <liveMedia.hh>
#include <queue>
#include <iostream>

#define POOL_SIZE 10000 //TODO:: to define the right value
class memory_pool
{

public:
    memory_pool()
    {
        //alloc memory
        for (int i = 0; i < POOL_SIZE; i++)
        {
            unsigned char *mem = new unsigned char[1280 * 720 * 2]; //TODO:to use OutPacketBuffer::maxSize;
            //memset(mem,'X',1280 * 720 * 3);
            pool.push(mem);
        }
        std::cout<<"memory_pool: pool size is: "<<pool.size()<<"\n";
    }

    unsigned char *getNextMem()
    {
        unsigned char *mem = nullptr;

        //std::cout<<"before getMem: pool size is: "<<pool.size()<<"\n";
        if (!pool.empty())
        {
            mem = pool.front();
            pool.pop();
        }
        else
        {
            std::cout<< "pool is empty\n";
        }
        return mem;
    }

    void returnMem(unsigned char* mem)
    {
        if (mem != nullptr)
        {
            pool.push(mem);
            //std::cout<<"after returnMem: pool size is: "<<pool.size()<<"\n";
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
