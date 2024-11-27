#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <queue>
#include <algorithm>
#include <stack>
#include <cctype>
#include <sstream>
#include <iomanip>
using namespace std;
int hexStringToInt(const std::string& hexStr) {
    return std::stoi(hexStr, nullptr, 16);
}
std::vector<std::pair<std::string, std::vector<std::string>>> parseString(const std::string& input) {
    std::vector<std::pair<std::string, std::vector<std::string>>> result;
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        std::string key;
        std::vector<std::string> values;

        // 첫 번째 단어를 키로 사용
        lineStream >> key;

        // 키 다음의 부분을 한 번에 읽고, ','로 나눕니다.
        std::string restOfLine;
        if (std::getline(lineStream, restOfLine)) {
            std::istringstream valueStream(restOfLine);
            std::string token;

            // 나머지 숫자들을 ','를 구분자로 구분해서 벡터에 추가
            while (std::getline(valueStream, token, ',')) {
                // 공백 제거
                token.erase(0, token.find_first_not_of(" \t\n\r"));
                token.erase(token.find_last_not_of(" \t\n\r") + 1);
                if (!token.empty()) {
                    values.push_back(token);
                }
            }
        }

        result.emplace_back(key, values);
    }

    return result;
}
int main(int argc,char* argv[]) {
    string input_file = "out.cn";
    string output_file = "main.cne";
    for (int i = 1; i < argc; i++) {
        string str = argv[i];
        if (str == "-o") {
            output_file = argv[i + 1];
            i++;
        }
        else {
            input_file = str;
        }
    }
    ifstream input(input_file);
    ofstream output(output_file, ios::binary);
    output << 'C' << 'N' << (char)10 << (char)0 << (char)0 << (char)0 << (char)0 << (char)0 << (char)0 << (char)0;
    map<string, int> functions;
    map<int, string> calls;
    string file((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    auto datas = parseString(file);
    int code_size = 0;
    for (auto data : datas) {
        if (data.first == "ad2") {
            output << (char)0x01;
        }
        else if (data.first == "add") {
            output << (char)0x11;
        }
        else if (data.first == "mov") {
            output << (char)0x31;
        }
        else if (data.first == "ad3") {
            output << (char)0x02;
        }
        else if (data.first == "rest") {
            output << (char)0x04;
        }
        else if (data.first == "not") {
            output << (char)0x03;
        }
        else if (data.first == "print") {
            output << (char)0x06;
        }
        else if (data.first == "end") {
            output << (char)0xEE;
        }
        else if (data.first == "ret") {
            output << (char)0x56;
        }
        else if (data.first == "getm") {
            output << (char)0x32;
        }
        else if (data.first == "test") {
            output << (char)0x1f;
        }
        if (data.first == "DATA") {
            code_size = output.tellp();
            for (size_t i = 0; i < data.second[0].length(); i += 2) {
                // 두 문자씩 읽어와서 16진수로 변환하여 저장
                std::string byteString = data.second[0].substr(i, 2);
                unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
                output.write(reinterpret_cast<const char*>(&byte), sizeof(byte));  // 바이트를 파일에 기록
            }
        }
        else if (data.first.back() == ':') {
            data.first.pop_back();
            functions[data.first] = output.tellp();
        }
        else if (data.first == "call") {
            output << (char)0x55;
            calls[output.tellp()] = data.second[0];
            output << (char)0x00;
            output << (char)0x00;
            output << (char)0x00;
            output << (char)0x00;
        }
        else if (data.first == "jmp") {
            output << (char)0x60;
            calls[output.tellp()] = data.second[0];
            output << (char)0x00;
            output << (char)0x00;
            output << (char)0x00;
            output << (char)0x00;
        }
        else if (data.first == "jz") {
            output << (char)0x61;
            calls[output.tellp()] = data.second[0];
            output << (char)0x00;
            output << (char)0x00;
            output << (char)0x00;
            output << (char)0x00;
        }
        else {
            for (auto arg : data.second) {
                int number = hexStringToInt(arg);
                output.write(reinterpret_cast<const char*>(&number), sizeof(number));
            }
        }
    }
    for (auto call : calls) {
        if (functions.find(call.second) != functions.end()) {
            output.seekp(call.first);
            auto function = functions.find(call.second);
            output.write(reinterpret_cast<const char*>(&function->second), sizeof(function->second));
        }
        cout << call.first << " " << call.second << endl;
    }
    for (auto function : functions) {
        cout << function.first << " " << function.second << endl;
    }
    output.seekp(6);
    output.write(reinterpret_cast<const char*>(&code_size), sizeof(code_size));
    input.close();
    output.close();
}
