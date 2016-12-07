#include <sstream>

#include "input_parameters.h"

input_parameters::input_parameters()
    : m_cpu_load(100)
    , m_is_deamon(false)
    , m_image_path("")
    , m_ip("")
{}

input_parameters::~input_parameters() 
{
    close_fds();
}

input_parameters input_parameters::parse_input_parameters(int argc, char* argv[])
{
    input_parameters parameters;
    if (argc < 3)
    {
        std::cout << __func__ << ": wrong argument number!" << std::endl;   
    }

    // skips first argument (path)
    for (int i = 1; i < argc; ++i)
    {
        if (!std::strcmp(DEMON_FLAG, argv[i]))
        {
            parameters.m_is_deamon = true;
        }
        else if (!std::strcmp(CPU_FLAG ,argv[i]) && (i + 1) < argc)
        {
            int input_cpu = std::atoi(argv[++i]);
            if (input_cpu > 0 && input_cpu < 100)
            {
                parameters.m_cpu_load = input_cpu;
            }
            else
            {
                std::cout << __func__ << ": invalid --cpu argument" << std::endl;
                break;
            }
        }
        else if (!std::strcmp(NET_FLAG, argv[i]) && (i + 1) < argc)
        {
            parameters.m_ip = argv[++i];
        }
        else
        {
            if (parameters.m_image_path.empty())
            {
                parameters.m_image_path = argv[i];
                continue;
            }
            parameters.m_args.push_back(argv[i]);
        }
    }

    return parameters;
}

bool input_parameters::is_empty() const
{
    return m_image_path.empty();
}

std::string input_parameters::to_string() const 
{
    std::stringstream output;
    output << "cpu load: " << m_cpu_load << std::endl;
    output << "deamon  : " << m_is_deamon << std::endl;
    output << "path    : " << m_image_path << std::endl;
    output << "ip      : " << m_ip << std::endl;
    for (const auto& arg : m_args)
        output << "arg     : " << arg << std::endl;

    return output.str();
}

bool input_parameters::set_pipes()
{
    return pipe(m_pipe_fst) < 0 || pipe(m_pipe_snd) < 0;
}

void input_parameters::write_fst(int source)
{
    write_fd(m_pipe_fst[1], source);
}

void input_parameters::write_snd(int source)
{
    write_fd(m_pipe_snd[1], source);
}

int input_parameters::read_fst()
{
    return read_fd(m_pipe_fst[0]);
}

int input_parameters::read_snd()
{
    return read_fd(m_pipe_snd[0]);
}

void input_parameters::close_fds()
{
    close(m_pipe_fst[0]);
    close(m_pipe_fst[1]);
    close(m_pipe_snd[0]);
    close(m_pipe_snd[1]);
}

void input_parameters::exec_command()
{
    if (execvp(m_args[0], m_args.data()) < 0)
    {
      std::cout << __func__ << ": failed command run!" << std::endl;
      exit(1);
    }    
}

int input_parameters::read_fd(int fd)
{
    int output = 0;
    if (read(fd, &output, sizeof(int)) <= 0)
    {
        std::cout << __func__ << ": failed read from second fd" << std::endl;
        exit(1);
    }

    return output; 
}

void input_parameters::write_fd(int fd, int source)
{
    if (write(fd, &source, sizeof(int)) <= 0)
    {
        std::cout << __func__ <<": faile write to second fd" << std::endl;
        exit(1);
    }
}

