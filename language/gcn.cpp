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
#include <set>
#include <memory.h>
using namespace std;
std::string unsignedCharToHex(unsigned char value) {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << std::hex << (int)value;
    return ss.str();
}
std::string toHexString(int value) {
    std::stringstream ss;
    ss << std::hex << value;
    return ss.str();
}
map<int, unsigned char> memory;
int last_memory = 0;
string ntext = " /+-*=!@#$%^&(){}[]\\\n`~,.|;\t\r";
string math = "+-*/";
set<string> Vars = { "int","char","void" };
int points = 0;
int plus_buffer = 0;
class Code;
map<string, Code> vars;
map<string, int> functions;

void promote(ofstream& output, int addr, int size, int resize,int result) {
    int i = 0;
    for (i = 0; i < (size > resize ? resize : size); i++) {
        output << "mov " << toHexString(addr + i) << "," << toHexString(result + i) << endl;
    }
    int i2 = i;
    output << "test " << toHexString(result + i - 1) << ",80" << endl;
    output << "jz pos" << points << endl;
    for (; i < resize; i++) {
        output << "mov ff," << toHexString(result + i) << endl;
    }
    output << "jmp pos" << points + 1 << endl;
    output << "pos" << points << ":" << endl;
    points++;
    for (i = i2; i < resize; i++) {
        output << "mov 0," << toHexString(result + i) << endl;
    }
    output << "pos" << points << ":" << endl;
    points++;
}
void memory_plus(ofstream& output,int addr1,int addr2,int size,int result) {
    output << "ad2 " << toHexString(addr1) << "," << toHexString(addr2) << "," << toHexString(result) << endl;
    for (int i = 1; i < size; i++) {
        output << "rest " << toHexString(plus_buffer) << endl;
        output << "ad3 " << toHexString(addr1 + i) << "," << toHexString(addr2 + i) << "," << toHexString(plus_buffer) << "," << toHexString(result + i) << endl;
    }
}
void memory_com(ofstream& output, int addr, int size) {
    for (int i = 0; i < size; i++) {
        output << "not " << toHexString(addr + i) << endl;
    }
    output << "add " << toHexString(addr) << endl;
    for (int i = 1; i < size; i++) {
        output << "rest " << toHexString(plus_buffer) << endl;
        output << "ad2 " << toHexString(addr + i) << "," << toHexString(plus_buffer) << "," << toHexString(addr + i) << endl;
    }
}
class Code {
public:
    int type;
    string name;
    int addr;
};
class CodeTree {
public:
    Code code;
    vector<CodeTree*> childs;
    CodeTree* parent;
    void printTree(int depth = 0) {
        // 현재 노드를 depth에 맞춰 출력
        for (int i = 0; i < depth; ++i) {
            cout << "    "; // 들여쓰기 (depth에 따라 변경)
        }
        cout << "- " << code.name << endl;

        // 자식 노드를 재귀적으로 출력
        for (auto child : childs) {
            child->printTree(depth + 1);
        }
    }
    void outcode(ofstream& output) {
        if (code.type == 0) {
            if (vars.find(code.name) != vars.end()) {
                code = vars.find(code.name)->second;
            }
        }
        if (code.type == 6) {
            if (childs.size() == 1) {
                if (childs[0]->childs.size() == 2) {
                    if (childs[0]->childs[0]->code.name == "(") {
                        if (childs[0]->childs[1]->code.name == "{") {
                            output << "_" << childs[0]->code.name << ":" << endl;
                            childs[0]->childs[1]->outcode(output);
                            output << "ret" << endl;
                        }
                        else {
                            //함수 선언
                        }
                    }
                    else {
                        //아마 오류
                    }
                }
                else {
                    for (auto child : childs) {
                        if (code.name == "int") {
                            memory[last_memory] = 0;
                            memory[last_memory + 1] = 0;
                            memory[last_memory + 2] = 0;
                            memory[last_memory + 3] = 0;
                            child->code.addr = last_memory;
                            child->code.type = 1;
                            vars[child->code.name] = child->code;
                            last_memory += 4;
                        }
                        if (code.name == "char") {
                            memory[last_memory] = 0;
                            child->code.addr = last_memory;
                            child->code.type = 3;
                            vars[child->code.name] = child->code;
                            last_memory++;
                        }//변수 종류
                        child->outcode(output);
                    }
                }
            }
            else {
                for (auto child : childs) {
                    if (code.name == "int") {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        child->code.addr = last_memory;
                        child->code.type = 1;
                        vars[child->code.name] = child->code;
                        last_memory += 4;
                    }
                    if (code.name == "char") {
                        memory[last_memory] = 0;
                        child->code.addr = last_memory;
                        child->code.type = 3;
                        vars[child->code.name] = child->code;
                        last_memory++;
                    }//변수 종류
                    child->outcode(output);
                }
            }
        }
        else {
            if (code.name == "(") {
                if (childs.size() > 0) {
                    childs[0]->outcode(output);
                    if (childs[0]->code.type == 1) {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        code.addr = last_memory;
                        output << "mov " << toHexString(childs[0]->code.addr) << "," << toHexString(last_memory) << endl;
                        output << "mov " << toHexString(childs[0]->code.addr + 1) << "," << toHexString(last_memory + 1) << endl;
                        output << "mov " << toHexString(childs[0]->code.addr + 2) << "," << toHexString(last_memory + 2) << endl;
                        output << "mov " << toHexString(childs[0]->code.addr + 3) << "," << toHexString(last_memory + 3) << endl;
                        last_memory += 4;
                        code.type = 1;

                    }
                    else if (childs[0]->code.type == 3) {
                        memory[last_memory] = 0;
                        code.addr = last_memory;
                        output << "mov " << toHexString(childs[0]->code.addr) << "," << toHexString(last_memory) << endl;
                        last_memory++;
                        code.type = 3;
                    }
                }
            }
            else if (code.name == "+") {
                if (childs.size() == 2) {
                    childs[0]->outcode(output);
                    childs[1]->outcode(output);
                    if (childs[0]->code.type == 1 && childs[1]->code.type == 1) {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        code.addr = last_memory;
                        memory_plus(output, childs[0]->code.addr, childs[1]->code.addr, 4, last_memory);
                        last_memory+=4;
                        code.type = 1;
                    }
                    if (childs[0]->code.type == 1 && childs[1]->code.type == 3) {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        memory[last_memory + 4] = 0;
                        memory[last_memory + 5] = 0;
                        memory[last_memory + 6] = 0;
                        memory[last_memory + 7] = 0;
                        code.addr = last_memory;
                        promote(output, childs[1]->code.addr, 1, 4, last_memory + 4);
                        memory_plus(output, childs[0]->code.addr, last_memory + 4, 4, last_memory);
                        last_memory += 8;
                        code.type = 1;
                    }
                    if (childs[0]->code.type == 3 && childs[1]->code.type == 1) {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        memory[last_memory + 4] = 0;
                        memory[last_memory + 5] = 0;
                        memory[last_memory + 6] = 0;
                        memory[last_memory + 7] = 0;
                        code.addr = last_memory;
                        promote(output, childs[0]->code.addr, 1, 4, last_memory + 4);
                        memory_plus(output, childs[1]->code.addr, last_memory + 4, 4, last_memory);
                        last_memory += 8;
                        code.type = 1;
                    }
                    else if (childs[0]->code.type == 3 && childs[0]->code.type == 3) {
                        memory[last_memory] = 0;
                        code.addr = last_memory;
                        output << "ad2 " << toHexString(childs[0]->code.addr) << "," << toHexString(childs[1]->code.addr) << "," << toHexString(last_memory) << endl;
                        last_memory++;
                        code.type = 3;
                    }
                }
            }
            else if (code.name == "-") {
                if (childs.size() == 2) {
                    childs[0]->outcode(output);
                    childs[1]->outcode(output);
                    if (childs[0]->code.type == 1 && childs[1]->code.type == 1) {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        code.addr = last_memory;
                        output << "mov " << toHexString(childs[1]->code.addr) << "," << toHexString(last_memory) << endl;
                        output << "mov " << toHexString(childs[1]->code.addr + 1) << "," << toHexString(last_memory + 1) << endl;
                        output << "mov " << toHexString(childs[1]->code.addr + 2) << "," << toHexString(last_memory + 2) << endl;
                        output << "mov " << toHexString(childs[1]->code.addr + 3) << "," << toHexString(last_memory + 3) << endl;
                        memory_com(output, last_memory,4);
                        memory_plus(output, childs[0]->code.addr, last_memory, 4, last_memory);
                        last_memory += 4;
                        code.type = 1;
                    }
                    if (childs[0]->code.type == 1 && childs[1]->code.type == 3) {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        memory[last_memory + 4] = 0;
                        memory[last_memory + 5] = 0;
                        memory[last_memory + 6] = 0;
                        memory[last_memory + 7] = 0;
                        code.addr = last_memory;
                        promote(output, childs[1]->code.addr, 1, 4, last_memory + 4);
                        memory_com(output, last_memory + 4, 4);
                        memory_plus(output, childs[0]->code.addr, last_memory + 4, 4, last_memory);
                        last_memory += 8;
                        code.type = 1;
                    }
                    if (childs[0]->code.type == 3 && childs[1]->code.type == 1) {
                        memory[last_memory] = 0;
                        memory[last_memory + 1] = 0;
                        memory[last_memory + 2] = 0;
                        memory[last_memory + 3] = 0;
                        memory[last_memory + 4] = 0;
                        memory[last_memory + 5] = 0;
                        memory[last_memory + 6] = 0;
                        memory[last_memory + 7] = 0;
                        code.addr = last_memory;
                        promote(output, childs[0]->code.addr, 1, 4, last_memory + 4);
                        output << "mov " << toHexString(childs[1]->code.addr) << "," << toHexString(last_memory) << endl;
                        output << "mov " << toHexString(childs[1]->code.addr + 1) << "," << toHexString(last_memory + 1) << endl;
                        output << "mov " << toHexString(childs[1]->code.addr + 2) << "," << toHexString(last_memory + 2) << endl;
                        output << "mov " << toHexString(childs[1]->code.addr + 3) << "," << toHexString(last_memory + 3) << endl;
                        memory_com(output, last_memory, 4);
                        memory_plus(output, last_memory, last_memory + 4, 4, last_memory);
                        last_memory += 8;
                        code.type = 1;
                    }
                    else if (childs[0]->code.type == 3 && childs[0]->code.type == 3) {
                        memory[last_memory] = 0;
                        code.addr = last_memory;
                        output << "mov " << toHexString(childs[1]->code.addr) << "," << last_memory << endl;
                        memory_com(output, last_memory, 1);
                        output << "ad2 " << toHexString(childs[0]->code.addr) << "," << toHexString(childs[1]->code.addr) << "," << toHexString(last_memory) << endl;
                        last_memory++;
                        code.type = 3;
                    }
                }
            }
            else if (functions.find(code.name) != functions.end()) {
                if (childs.size() > 0) {
                    if (childs[0]->code.name == "(") {
                        childs[0]->outcode(output);
                        char addr[4] = "";
                        memcpy(addr, &childs[0]->code.addr, sizeof(int));
                        memory[last_memory] = addr[0];
                        memory[last_memory + 1] = addr[1];
                        memory[last_memory + 2] = addr[2];
                        memory[last_memory + 3] = addr[3];
                        int function = functions.find(code.name)->second;
                        output << "mov " << toHexString(last_memory) << "," << toHexString(function) << endl;
                        output << "mov " << toHexString(last_memory + 1) << "," << toHexString(function + 1) << endl;
                        output << "mov " << toHexString(last_memory + 2) << "," << toHexString(function + 2) << endl;
                        output << "mov " << toHexString(last_memory + 3) << "," << toHexString(function + 3) << endl;
                        last_memory += 4;
                        output << "call _" << code.name << endl;
                    }
                }
            }
            else if (code.name == "=") {
                childs[0]->outcode(output);
                if (parent->code.type == 1) {
                    output << "mov " << toHexString(childs[0]->code.addr) << "," << toHexString(parent->code.addr) << endl;
                    output << "mov " << toHexString(childs[0]->code.addr + 1) << "," << toHexString(parent->code.addr + 1) << endl;
                    output << "mov " << toHexString(childs[0]->code.addr + 2) << "," << toHexString(parent->code.addr + 2) << endl;
                    output << "mov " << toHexString(childs[0]->code.addr + 3) << "," << toHexString(parent->code.addr + 3) << endl;
                }
                else if (parent->code.type == 3) {
                    output << "mov " << toHexString(childs[0]->code.addr) << "," << toHexString(parent->code.addr) << endl;
                }
                cout << parent->code.type << endl;
            }
            else {
                for (auto child : childs) {
                    child->outcode(output);
                }
            }
            //여기에 연산 추가
        }
    }
};
int main(int argc,char* argv[]) {
    for (int i = 0; i <= 0xFF; i++) {
        memory[last_memory] = i;
        last_memory++;
    }
    string input_file = "main.ccn";
    string output_file = "out.cn";
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
    ofstream output(output_file);
    vector<Code> codes;
    int nstack = 0;
    string file((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    for (int i = 0; i < file.length(); i++) {
        Code code;
        if (ntext.find(file[i]) != string::npos) {
            if (file[i] != ' ' && file[i] != '\t' && file[i] != '\n') {
                string str;
                str += file[i];
                code.addr = 0;
                code.name = str;
                code.type = 2;
                codes.push_back(code);
            }
        }
        else {
            if (isdigit(file[i])) {
                string number = "";
                while (isdigit(file[i])) {
                    number += file[i];
                    i++;
                }
                i--;
                int num = atoi(number.c_str());
                memory[last_memory + 3] = (num >> 24) & 0xFF;
                memory[last_memory + 2] = (num >> 16) & 0xFF;
                memory[last_memory + 1] = (num >> 8) & 0xFF;
                memory[last_memory] = num & 0xFF;
                code.addr = last_memory;
                code.name = number;
                code.type = 1;
                codes.push_back(code);
                last_memory += 4;
            }
            else if (file[i] == '\"') {
                i++;
                string text = "";
                while (file[i] != '\"') {
                    text += file[i];
                    i++;
                }
                code.name = text;
                code.addr = 0;
                code.type = 5;
                codes.push_back(code);
            }
            else if (file[i] == '\'') {
                i++;
                string text = "";
                while (file[i] != '\'') {
                    text += file[i];
                    i++;
                }
                memory[last_memory] = text[0];
                code.addr = last_memory;
                code.name = text;
                code.type = 3;
                codes.push_back(code);
                last_memory++;
            }
            else {
                string text = "";
                while (ntext.find(file[i]) == string::npos) {
                    text += file[i];
                    i++;
                }
                if (Vars.find(text) != Vars.end()) {
                    code.addr = 0;
                    code.name = text;
                    code.type = 6;
                    codes.push_back(code);
                }
                else {
                    code.addr = 0;
                    code.name = text;
                    code.type = 0;
                    codes.push_back(code);
                }
                i--;
            }
        }
    }
    CodeTree* tree = new CodeTree();
    CodeTree* top = tree;
    tree->code.name = "";
    tree->parent = nullptr;
    tree->code.type = -1;
    for (int i = 0; i < codes.size(); i++) {
        Code code = codes[i];
        CodeTree* child = new CodeTree();
        if (code.name == "{" || Vars.find(code.name) != Vars.end()) {
            child->code = code;
            tree->childs.push_back(child);
            child->parent = tree;
            tree = child;
        }
        else if (code.name == "(") {
            child->code = code;
            if (tree->childs.size() != 0 && math.find(tree->code.name) == string::npos) {
                child->parent = tree->childs.back();
                tree->childs.back()->childs.push_back(child);
            }
            else {
                tree->childs.push_back(child);
                child->parent = tree;
            }
            tree = child;
        }
        else if (code.name == "}") {
            tree = tree->parent;
            if (tree->code.type == 0) {
                tree = tree->parent;
                if (Vars.find(tree->code.name) != Vars.end()) {
                    tree = tree->parent;
                }
            }
        }
        else if (code.name == ")") {
            tree = tree->parent;
            if (tree->childs.size() != 1) {
                tree = tree->parent;
            }
        }
        else if (code.name == ",") {
            if (tree->code.name == "=") {
                tree = tree->parent;
                tree = tree->parent;
            }
        }
        else if (math.find(code.name) != string::npos) {
            if (i != 0) {
                if (i != codes.size() - 1) {
                    CodeTree* ch2 = tree->childs.back();
                    tree->childs.pop_back();
                    ch2->parent = child;
                    child->childs.push_back(ch2);
                    child->code = code;
                    tree->childs.push_back(child);
                    child->parent = tree;
                    tree = child;
                }
            }
            else {
                if (i != codes.size() - 1) {
                    //양수 음수 정할 때
                }
            }
        }
        else if (code.name == "=") {
            child->code = code;
            child->parent = tree->childs.back();
            tree->childs.back()->childs.push_back(child);
            tree = child;
        }
        else {
            child->code = code;
            child->parent = tree;
            tree->childs.push_back(child);
            if (math.find(tree->code.name) != string::npos) {
                if (tree->childs.size() >= 2) {
                    tree = tree->parent;
                }
            }
            else if (code.name == ";") {
                tree->childs.pop_back();
                if (tree->code.type == 2) {
                    if (tree->code.name == "=") {
                        tree = tree->parent;
                        tree = tree->parent;
                        child->parent = tree;
                        if (tree->code.type == 6) {
                            tree = tree->parent;
                        }
                    }
                }
                else if (tree->code.type == 0) {
                    tree = tree->parent;
                    if (Vars.find(tree->code.name) != Vars.end()) {
                        tree = tree->parent;
                    }
                }
                else if (tree->code.type == 6) {
                    tree = tree->parent;
                }
            }
        }
    }
    top->printTree();
    output << "call _main" << endl << "end" << endl;
    memory[last_memory] = 0;
    memory[last_memory + 1] = 0;
    memory[last_memory + 2] = 0;
    memory[last_memory + 3] = 0;
    memory[last_memory + 4] = 0;
    output << "_print:" << endl << "getm " << toHexString(last_memory) << "," << toHexString(last_memory + 4) << endl << "print " << toHexString(last_memory + 4) << endl << "ret" << endl;
    functions.insert({ "print",last_memory });
    last_memory += 5;
    plus_buffer = last_memory;
    memory[last_memory] = 0;
    last_memory++;
    top->outcode(output);
    output << "DATA ";
    for (auto datas : memory) {
        output << unsignedCharToHex(datas.second);
    }
    output.close();
    input.close();
}
