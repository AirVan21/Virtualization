#ifndef HELPERS_H
#define HELPERS_H

#include <vector>
#include <set>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <cerrno>
#include <fstream>

#define CONTAINERS_DIR std::string("containers")
#define CONTAINER_LIST_FILE ("containers/container_list")

bool is_exist_dir(const std::string& path)
{
    struct stat info;
    
    if (stat(path.c_str(), &info))
    {
        // can't access
        return false;
    }
    
    return info.st_mode & S_IFDIR; 
}

void create_dir(const std::string& path)
{
    if (is_exist_dir(path))
    {
        return;
    }
    
    __mode_t mode = 0777;
    if (mkdir(path.c_str(), mode) < 0)
    {
      std::cout << "Can not create directory " + path << std::endl;
      exit(1);
    }
}

void create_container_list()
{
    if (!is_exist_dir(CONTAINERS_DIR))
    {
        create_dir(CONTAINERS_DIR);
        std::ofstream output(CONTAINER_LIST_FILE);
        output.close();
    }
}

std::set<int> read_container_list()
{
    std::set<int> output;
    std::ifstream container_list(CONTAINER_LIST_FILE);

    int pid;
    while(container_list >> pid)
    {
        output.insert(pid);
    }
    
    return output;
}

void remove_from_list(int id)
{
    std::set<int> container_source = read_container_list();
    auto it = container_source.find(id);
    if (it == container_source.end())
    {
        return;
    }
    container_source.erase(it);
    
    std::ofstream container_list(CONTAINER_LIST_FILE);
    for (int cur_id : container_source)
    {
        container_list << cur_id << std::endl;
    }
}

void add_to_list(int pid)
{
    create_container_list();   
    std::ofstream contlist_file(CONTAINER_LIST_FILE, std::ofstream::out | std::ofstream::app);
    contlist_file << pid << std::endl;
}

bool exist_in_list(int id)
{
    create_container_list();
    std::set<int> container_source = read_container_list();
    
    return container_source.find(id) != container_source.end();
}

#endif /* HELPERS_H */

