#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#define ROUNDS 4

const uint32_t delta = 0x9e3779b9;

uint32_t madryga_encrypt(uint32_t plain_text, uint32_t key)
{
    uint32_t v0 = plain_text, v1 = key, sum = 0;
    for (int i = 0; i < ROUNDS; i++)
    {
        sum += delta;
        v0 += ((v1 << 4) + key) ^ (v1 + sum) ^ ((v1 >> 5) + key);
        v1 += ((v0 << 4) + key) ^ (v0 + sum) ^ ((v0 >> 5) + key);
    }
    return v0;
}

uint32_t madryga_decrypt(uint32_t cipher_text, uint32_t key)
{
    uint32_t v0 = cipher_text, v1 = key, sum = delta * ROUNDS;
    for (int i = 0; i < ROUNDS; i++)
    {
        v1 -= ((v0 << 4) + key) ^ (v0 + sum) ^ ((v0 >> 5) + key);
        v0 -= ((v1 << 4) + key) ^ (v1 + sum) ^ ((v1 >> 5) + key);
        sum -= delta;
    }
    return v0;
}

void encrypt_directory(std::string &directory_path, std::string key)
{
    for (const auto &entry : std::filesystem::directory_iterator(directory_path))
    {
        if (entry.is_regular_file())
        {
            std::string file_path = entry.path();
            std::ifstream file(file_path, std::ios::binary);
            std::vector<uint32_t> plain_text;

            std::vector<uint32_t> cipher_text;
            for (uint32_t text_chunk : plain_text)
            {
                cipher_text.push_back(madryga_encrypt(text_chunk, std::stoul(key, nullptr, 0)));
            }

            std::string encrypt_file_path = directory_path + ".madryga";
            std::ofstream encrypt_file(encrypt_file_path, std::ios::binary);

            for (uint32_t text_chunk : cipher_text)
            {
                encrypt_file.write(reinterpret_cast<const char *>(&text_chunk), sizeof(text_chunk));
            }

            file.close();
            encrypt_file.close();
        }
    }
}

void decrypt_directory(std::string &directory_path, std::string key)
{
    for (const auto &entry : std::filesystem::directory_iterator(directory_path))
    {
        if (entry.is_regular_file())
        {
            std::string file_path = entry.path();
            std::ifstream file(file_path, std::ios::binary);
            std::vector<uint32_t> cipher_text;
            std::vector<uint32_t> plain_text;
            uint32_t text_chunk;
            while (file.read(reinterpret_cast<char *>(&text_chunk), sizeof(text_chunk)))
            {
                cipher_text.push_back(text_chunk);
            }
            for (uint32_t text_chunk : cipher_text)
            {
                plain_text.push_back(madryga_decrypt(text_chunk, std::stoul(key, nullptr, 0)));
            }
            std::string decrypt_file_path = file_path + ".decrypted";
            std::ofstream decrypt_file(decrypt_file_path, std::ios::binary);
            for (uint32_t text_chunk : plain_text)
            {
                decrypt_file.write(reinterpret_cast<const char *>(&text_chunk), sizeof(text_chunk));
            }
            file.close();
            decrypt_file.close();
        }
    }
}

int main(char *argv[], int argc)
{
    std::string directory_path, key;
    int num_rounds;

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <key>\n";
        return 1;
    }

    directory_path = argv[1];
    key = argv[2];

    int choice;
    std::cout << "1. Encrypt\n2. Decrypt\n";
    std::cin >> choice;

    if (choice == 1)
    {
        encrypt_directory(directory_path, key);
    }
    else if (choice == 2)
    {
        decrypt_directory(directory_path, key);
    }
    else
    {
        std::cerr << "Invalid choice\n";
    }

    return 0;
}