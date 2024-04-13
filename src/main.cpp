#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <Kernel/sys/syslimits.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

#define ROUNDS 16

typedef uint32_t u32;

u32 key[4];

void read_key_from_env()
{
    std::ifstream env_file(".env");
    if (!env_file)
    {
        std::cerr << "Failed to open .env file\n";
        exit(EXIT_FAILURE);
    }

    std::string line;
    int i = 0;
    while (std::getline(env_file, line) && i < 4)
    {
        std::istringstream iss(line);
        std::string id, eq, val;

        bool error = false;

        if (!(iss >> id >> eq >> val) || eq != "=")
        {
            error = true;
        }

        if (id == "KEY1" || id == "KEY2" || id == "KEY3" || id == "KEY4")
        {
            key[i] = std::stoul(val, nullptr, 16);
            i++;
        }
        else
        {
            error = true;
        }

        if (error)
        {
            std::cerr << "Bad formatting in .env file\n";
            exit(EXIT_FAILURE);
        }
    }
}

void madryga_encrypt(u32 *v, u32 *k)
{
    u32 v0 = v[0], v1 = v[1], sum = 0, i;
    u32 delta = 0x9E3779B9;

    for (i = 0; i < ROUNDS; i++)
    {
        sum += delta;
        v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
        v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
    }

    v[0] = v0;
    v[1] = v1;
}

void madryga_decrypt(u32 *v, u32 *k)
{
    u32 v0 = v[0], v1 = v[1], sum = 0xE3779B90, i;
    u32 delta = 0x9E3779B9;

    for (i = 0; i < ROUNDS; i++)
    {
        v1 -= ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
        v0 -= ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
        sum -= delta;
    }

    v[0] = v0;
    v[1] = v1;
}

void madryga_encrypt_data(unsigned char *data, int data_len)
{
    int i;
    uint32_t *ptr = reinterpret_cast<uint32_t *>(data);

    for (i = 0; i < data_len / 8; i++)
    {
        madryga_encrypt(ptr, key);
        ptr += 2;
    }

    int remaining = data_len % 8;

    if (remaining != 0)
    {
        unsigned char pad[8] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        memcpy(pad, ptr, remaining);
        madryga_encrypt(reinterpret_cast<uint32_t *>(pad), key);
        memcpy(ptr, pad, remaining);
    }
}

void madryga_decrypt_data(unsigned char *data, int data_len)
{
    int i;
    uint32_t *ptr = reinterpret_cast<uint32_t *>(data);

    for (i = 0; i < data_len / 8; i++)
    {
        madryga_decrypt(ptr, key);
        ptr += 2;
    }

    int remaining = data_len % 8;

    if (remaining != 0)
    {
        unsigned char pad[8] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        memcpy(pad, ptr, remaining);
        madryga_decrypt(reinterpret_cast<uint32_t *>(pad), key);
        memcpy(ptr, pad, remaining);
    }
}

void encrypt_file(std::string input_path)
{
    FILE *input_file = fopen(input_path.c_str(), "rb");
    std::string output_path = input_path + ".madryga";
    FILE *output_file = fopen(output_path.c_str(), "wb");

    if (input_file == NULL || output_file == NULL)
    {
        perror("error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    unsigned char *file_content = new unsigned char[file_size];
    fread(file_content, 1, file_size, input_file);

    for (int i = 0; i < file_size / 8; i++)
    {
        madryga_encrypt_data(file_content + i * 8, 8);
    }

    fwrite(file_content, 1, file_size, output_file);
    fclose(input_file);
    fclose(output_file);
    delete[] file_content;

    if (remove(input_path.c_str()) != 0)
    {
        perror("Error deleting original file");
        exit(EXIT_FAILURE);
    }
}

void decrypt_file(const char *input_path, const char *output_path)
{
    FILE *input_file = fopen(input_path, "rb");
    FILE *output_file = fopen(output_path, "wb");

    if (input_file == NULL || output_file == NULL)
    {
        perror("error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    unsigned char *file_content = new unsigned char[file_size];
    fread(file_content, 1, file_size, input_file);

    for (int i = 0; i < file_size / 8; i++)
    {
        madryga_decrypt_data(file_content + i * 8, 8);
    }

    fwrite(file_content, 1, file_size, output_file);
    fclose(input_file);
    fclose(output_file);
    delete[] file_content;

    if (remove(input_path) != 0)
    {
        perror("Error deleting original file");
        exit(EXIT_FAILURE);
    }
}

void encrypt_directory(std::string &directory_path)
{
    for (const auto &entry : std::filesystem::directory_iterator(directory_path))
    {
        if (entry.is_regular_file())
        {
            std::string file_path = entry.path();
            encrypt_file(file_path);
        }
    }
}

void decrypt_directory(std::string &directory_path)
{
    for (const auto &entry : std::filesystem::directory_iterator(directory_path))
    {
        if (entry.is_regular_file())
        {
            std::string file_path = entry.path();
            std::string output_path = file_path.substr(0, file_path.size() - 8);
            decrypt_file(file_path.c_str(), output_path.c_str());
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: %s <encrypt/decrypt> <file/directory>\n", argv[0]);
        return 1;
    }

    std::string mode = argv[1];
    std::string path = argv[2];

    if (mode == "encrypt")
    {
        struct stat path_stat;
        stat(path.c_str(), &path_stat);

        if (S_ISDIR(path_stat.st_mode))
        {
            encrypt_directory(path);
        }
        else if (S_ISREG(path_stat.st_mode))
        {
            encrypt_file(path);
        }
    }
    else if (mode == "decrypt")
    {
        struct stat path_stat;
        stat(path.c_str(), &path_stat);

        if (S_ISDIR(path_stat.st_mode))
        {
            decrypt_directory(path);
        }
        else if (S_ISREG(path_stat.st_mode))
        {
            std::string output_path = path.substr(0, path.size() - 8);
            decrypt_file(path.c_str(), output_path.c_str());
        }
    }

    return 0;
}