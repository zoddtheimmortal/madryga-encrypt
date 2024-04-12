#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

uint32_t madryga_encrypt(uint32_t plain_text, uint32_t key, int num_rounds)
{
    return NULL;
}

uint32_t madryga_decrypt(uint32_t cipher_text, uint32_t key, int num_rounds)
{
    return NULL;
}

void encrypt_directory(std::string &directory_path, std::string key, int num_rounds)
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
                cipher_text.push_back(madryga_encrypt(text_chunk, std::stoul(key, nullptr, 0), num_rounds));
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

    void decrypt_directory(std::string & directory_path, std::string key, int num_rounds)
    {
        return;
    }

    int main(char *argv[], int argc)
    {
        std::string directory_path, key;
        int num_rounds;

        if (argc != 4)
        {
            std::cerr << "Usage: " << argv[0] << " <directory_path> <key> <num_rounds>\n";
            return 1;
        }

        directory_path = argv[1];
        key = argv[2];
        num_rounds = std::stoi(argv[3]);

        int choice;
        std::cout << "1. Encrypt\n2. Decrypt\n";
        std::cin >> choice;

        if (choice == 1)
        {
            encrypt_directory(directory_path, key, num_rounds);
        }
        else if (choice == 2)
        {
            decrypt_directory(directory_path, key, num_rounds);
        }
        else
        {
            std::cerr << "Invalid choice\n";
        }

        return 0;
    }