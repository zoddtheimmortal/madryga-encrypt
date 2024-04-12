#include<iostream>
#include<fstream>
#include<vector>
#include<filesystem>

uint32_t madryga_encrypt(uint32_t plain_text, uint32_t key,int num_rounds){
    return NULL;
}

uint32_t madryga_decrypt(uint32_t cipher_text, uint32_t key,int num_rounds){
    return NULL;
}

void encrypt_directory(std::string directory_path, std::string key, int num_rounds){
    return;
}

void decrypt_directory(std::string directory_path, std::string key, int num_rounds){
    return;
}

int main(char* argv[], int argc){
    std::string directory_path,key;
    int num_rounds;

    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <key> <num_rounds>\n";
        return 1;
    }

    directory_path = argv[1];
    key = argv[2];
    num_rounds = std::stoi(argv[3]);

    int choice;
    std::cout << "1. Encrypt\n2. Decrypt\n";
    std::cin >> choice;

    if(choice==1){
        encrypt_directory(directory_path,key,num_rounds);
    }else if(choice==2){
        decrypt_directory(directory_path,key,num_rounds);
    }else{
        std::cerr << "Invalid choice\n";
    }

    return 0;
}