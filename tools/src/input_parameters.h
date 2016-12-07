#ifndef INPUT_PARAMETERS_H
#define INPUT_PARAMETERS_H

#include <vector>
#include <unistd.h>
#include <iostream>
#include <cstring>

class input_parameters
{   
    
    #define DEMON_FLAG "-d"
    #define CPU_FLAG "--cpu"
    #define NET_FLAG "--net"

public:
    input_parameters();
    virtual ~input_parameters();

public:
    static input_parameters parse_input_parameters(int argc, char* argv[]);
    
    bool is_empty() const;
    std::string to_string() const;
    bool set_pipes();
    void write_fst(int source);
    void write_snd(int source);
    int read_fst();
    int read_snd();
    void close_fds();
    void exec_command();

private:
    int read_fd(int fd);
    void write_fd(int fd, int source);

public:
    int m_cpu_load;
    bool m_is_deamon;
    std::string m_image_path;
    std::string m_ip;
    std::vector<char*> m_args;
    // 0 - read, 1 - write
    int m_pipe_fst[2];
    int m_pipe_snd[2];
};

#endif /* INPUT_PARAMETERS_H */

