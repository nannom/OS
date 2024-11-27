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
#include <memory.h>
using namespace std;
std::string unsignedCharToHex(unsigned char value) {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << std::hex << (int)value;
    return ss.str();
}
int main() {
    ifstream input("main.cne", ios::binary | std::ios::ate);
    long fileSize = input.tellg();
    input.seekg(0, std::ios::beg);
    unsigned char* memory = new unsigned char[fileSize];
    input.read((char*)memory, fileSize);
    int now = 0;
    int data = 0;
    int header = 0;
    memcpy(&header, memory + 2, 4);
    int code = 0;
    memcpy(&code, memory + 6, 4);
    data = code;
    now = header;
    char result = 0;
    stack<int> calls;
    while (now < fileSize && memory[now] != 0xEE) {
        switch (memory[now])
        {
        case 0x01:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            int n2;
            memcpy(&n2, memory + now, 4);
            int n3;
            now += 4;
            memcpy(&n3, memory + now, 4);
            now += 4;
            int n4 = memory[data + n1] + memory[data + n2];
            memory[data + n3] = n4 & 0xFF;
            result = n4 >> 8;
            break;
        }
        case 0x11:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            memory[data + n1] = memory[data + n1] + 1;
            if (memory[data + n1] == 0) {
                result = 1;
            }
            break;
        }
        case 0x1f:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            int n2;
            memcpy(&n2, memory + now, 4);
            now += 4;
            result = memory[data + n1] & memory[data + n2];
            break;
        }
        case 0x55:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            calls.push(now);
            now = n1;
            break;
        }
        case 0x56:
        {
            now++;
            now = calls.top();
            calls.pop();
            break;
        }
        case 0x60:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            now = n1;
            break;
        }
        case 0x61:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            if (result == 0)
                now = n1;
            break;
        }
        case 0x02:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            int n2;
            memcpy(&n2, memory + now, 4);
            int n3;
            now += 4;
            memcpy(&n3, memory + now, 4);
            now += 4;
            int n4;
            memcpy(&n4, memory + now, 4);
            now += 4;
            int n5 = memory[data + n1] + memory[data + n2] + memory[data + n3];
            memory[data + n4] = n5 & 0xFF;
            result = n5 >> 8;

            //cout << unsignedCharToHex(memory[data + n1]) << " + " << unsignedCharToHex(memory[data + n2]) << " + " << unsignedCharToHex(memory[data + n3]) << " + " << unsignedCharToHex(memory[data + n4]) << endl;
            break;
        }
        case 0xEE:
        {
            now--;
            //cout << "end" << endl;
            break;
        }
        case 0x03:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            memory[data + n1] = ~memory[data + n1];
            break;
        }
        case 0x31:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            int n2;
            memcpy(&n2, memory + now, 4);
            now += 4;
            memory[data + n2] = memory[data + n1];
            break;
        }
        case 0x32:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            int n2;
            memcpy(&n2, memory + now, 4);
            now += 4;
            int n3;
            memcpy(&n3, memory + (data + n1), 4);
            memory[data + n2] = memory[data + n3];
            //cout << (int)memory[data + n1] << endl;
            break;
        }
        case 0x04:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            memory[data + n1] = result;
            break;
        }
        case 0x06:
        {
            int n1;
            now++;
            memcpy(&n1, memory + now, 4);
            now += 4;
            printf("%c", memory[data + n1]);
            break;
        }
        default:
            now++;
            break;
        }
    }
    delete[] memory;
}
