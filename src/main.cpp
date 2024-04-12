#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#define ROUNDS 4

const uint32_t delta = 0x9e3779b9;

uint32_t madryga_encrypt(uint32_t plain_text, uint32_t key)
{
    uint32_t L = plain_text >> 16, R = plain_text & 0xFFFF, sum = 0;
    for (int i = 0; i < ROUNDS; i++)
    {
        sum += delta;
        uint32_t Ki = key ^ sum;
        uint32_t F = ((R << 7) ^ Ki) + (R >> 5);
        uint32_t Lp = R;
        uint32_t Rp = L ^ F;
        L = Lp;
        R = Rp;
    }
    return (L << 16) | R;
}

uint32_t madryga_decrypt(uint32_t cipher_text, uint32_t key)
{
    uint32_t L = cipher_text >> 16, R = cipher_text & 0xFFFF, sum = delta * ROUNDS;
    for (int i = 0; i < ROUNDS; i++)
    {
        uint32_t Ki = key ^ sum;
        uint32_t F = ((L << 7) ^ Ki) + (L >> 5);
        uint32_t Rp = R;
        uint32_t Lp = L ^ F;
        R = Rp;
        L = Lp;
        sum -= delta;
    }
    return (R << 16) | L;
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

            uint32_t buffer;
            while (file.read(reinterpret_cast<char *>(&buffer), sizeof(buffer)))
            {
                plain_text.push_back(buffer);
            }

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

int main(int argc, char **argv)
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