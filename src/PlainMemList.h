#ifndef PLAINMEMLIST_H_
#define PLAINMEMLIST_H_

#include <Arduino.h>

#define PLAINMEMLIST_SIZE 64

class PlainMemList 
{
public:
    int length = 0;
    long data[64];
    void append(long value) 
    {
        if (length < PLAINMEMLIST_SIZE) data[length++] = value;
    }
    void remove(int index) 
    {
        if (index >= length || index < 0) return;
        memmove(&data[index], &data[index+1], sizeof(long) * (length - index - 1));

        length--;
    }
    int indexOf(long value)
    {
        for(int index = 0; index < length; index++)
        {
            if (data[index] == value) return index;
        }

        return -1;
    }
};

#endif