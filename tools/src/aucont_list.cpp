#include <fstream>
#include <iostream>

#include "helpers.h"

int main(int argc, char* argv[])
{
    std::ifstream container_list(CONTAINER_LIST_FILE);
    int pid = 0;
    while (container_list >> pid)
    {
        std::cout << pid << std::endl;
    }
       
    return 0;
}