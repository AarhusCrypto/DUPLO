//
//  interpreter.cpp
//  
//
//
//

#include "interpreter.h"

uint8_t * openMMap(string name, long & size)
{
    int m_circuit_fd;
    struct stat statbuf;
    uint8_t * m_ptr_begin;

    if ((m_circuit_fd = open(name.c_str(), O_RDONLY)) < 0)
    {
        perror("can't open file for reading");
    }

    if (fstat(m_circuit_fd, &statbuf) < 0)
    {
        perror("fstat in load_binary failed");
    }
    
    if(statbuf.st_size == 0)
    {
        size = 0;
        return 0;
    }

    if ((m_ptr_begin = (uint8_t *)mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, m_circuit_fd, 0)) == MAP_FAILED)
    {
        size=0;
        perror("Warning: mmap failed (This should happen if the function does nothing, returns nothing, and has no arguments).");
        return 0;
    }

    uint8_t * m_ptr = m_ptr_begin;
    size = statbuf.st_size;

    return m_ptr;
   
}


