//
// Created by 厉猛 on 2024-07-17.
//
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

const std::string filepath = "C:\\Users\\limen\\Desktop\\dump01(pure).txt";

int main() {
    std::ifstream fis(filepath);
    std::string line;
    std::vector<unsigned char> bytes;
    while (std::getline(fis, line)) {
        std::istringstream iss(line);
        std::string token;

        while (iss >> std::hex >> token) {
            if (token.length() != 2) {
                continue;
            }
            unsigned char byte = static_cast<unsigned char>(std::stoi(token, nullptr, 16));
            bytes.push_back(byte);
        }
    }

    std::ofstream fos("output.bin", std::ios::out | std::ios::binary);
    if (!fos.is_open()) {
        return -1;
    }
    fos.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    fos.close();
}
