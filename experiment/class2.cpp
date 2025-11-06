#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector>
#include <bitset>

struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

// 递归保存哈夫曼树到文件
void saveTreeToFileRec(HuffmanNode* node, std::ofstream& out) {
    if (!node) return;

    if (node->ch != '\0') {
        out << node->ch << ":" << node->freq << "\n";
    } else {
        out << "InternalNode:" << node->freq << "\n";
    }

    saveTreeToFileRec(node->left, out);
    saveTreeToFileRec(node->right, out);
}

// 保存哈夫曼树到文件
void saveTreeToFile(HuffmanNode* root, const std::string& filename) {
    std::ofstream out(filename);
    if (out.is_open()) {
        saveTreeToFileRec(root, out); // 使用ofstream写入树到文件
    } else {
        std::cerr << "无法打开文件: " << filename << std::endl;
    }
}

// 生成哈夫曼编码
void generateHuffmanCode(HuffmanNode* root, const std::string& code, std::unordered_map<char, std::string>& huffmanCode) {
    if (!root) return;

    if (root->ch != '\0') {
        huffmanCode[root->ch] = code;
    }

    generateHuffmanCode(root->left, code + "0", huffmanCode);
    generateHuffmanCode(root->right, code + "1", huffmanCode);
}

// 编码文件
void encodeFile(const std::string& inputFile, const std::string& outputFile, HuffmanNode* root) {
    std::unordered_map<char, std::string> huffmanCode;
    generateHuffmanCode(root, "", huffmanCode);

    std::ifstream file(inputFile);
    std::ofstream out(outputFile, std::ios::binary);

    char ch;
    std::string encodedText = "";
    while (file.get(ch)) {
        encodedText += huffmanCode[ch];
    }

    while (encodedText.length() % 8 != 0) { // 补充到8位
        encodedText += "0";
    }

    for (size_t i = 0; i < encodedText.length(); i += 8) {
        std::bitset<8> byte(encodedText.substr(i, 8));
        out.put(static_cast<unsigned char>(byte.to_ulong()));
    }
}

// 译码文件
void decodeFile(const std::string& inputFile, const std::string& outputFile, HuffmanNode* root) {
    std::ifstream file(inputFile, std::ios::binary);
    std::ofstream out(outputFile);

    HuffmanNode* current = root;
    unsigned char byte;
    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        for (int i = 7; i >= 0; --i) {
            if ((byte >> i) & 1) {
                current = current->right;
            } else {
                current = current->left;
            }

            if (!current->left && !current->right) { // 到达叶节点
                out.put(current->ch);
                current = root;
            }
        }
    }
}

// 打印编码文件
void printCodeFile(const std::string& codeFile, const std::string& printFile) {
    std::ifstream file(codeFile);
    std::ofstream out(printFile);

    std::string line;
    std::string code;
    while (file >> code) {
        line += code;
        if (line.size() >= 50) {
            out << line.substr(0, 50) << "\n";
            line = line.substr(50);
        }
    }

    if (!line.empty()) {
        out << line << "\n";
    }
}

// 打印哈夫曼树
void printHuffmanTree(HuffmanNode* root) {
    if (!root) return;
    if (root->ch != '\0') {
        std::cout << root->ch << " : " << root->freq << std::endl;
    } else {
        std::cout << "Internal Node : " << root->freq << std::endl;
    }

    printHuffmanTree(root->left);
    printHuffmanTree(root->right);
}

// 显示菜单
void displayMenu() {
    std::cout << "1. 初始化\n";
    std::cout << "2. 编码\n";
    std::cout << "3. 译码\n";
    std::cout << "4. 打印编码文件\n";
    std::cout << "5. 打印哈夫曼树\n";
    std::cout << "6. 退出\n";
}

// 创建示例输入文件 input.txt
void createInputFile() {
    std::ofstream out("input.txt");
    if (out.is_open()) {
        out << "hello huffman encoding example!";
        out.close();
    } else {
        std::cerr << "无法创建 input.txt 文件。" << std::endl;
    }
}

// 统计字符频率并构建哈夫曼树
void buildHuffmanTree(const std::string& filename, HuffmanNode*& root) {
    // 读取文件并统计字符频率
    std::ifstream file(filename);
    std::unordered_map<char, int> freqMap;

    char ch;
    while (file.get(ch)) {
        freqMap[ch]++;
    }

    // 使用优先队列构建哈夫曼树
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;

    // 将字符及其频率放入优先队列
    for (const auto& entry : freqMap) {
        pq.push(new HuffmanNode(entry.first, entry.second));
    }

    // 构建哈夫曼树
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();

        HuffmanNode* newNode = new HuffmanNode('\0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    // 最终队列中只有一个节点，即树的根节点
    root = pq.top();
}

int main() {
    // 如果没有 input.txt 文件，则创建它
    std::ifstream infile("input.txt");
    if (!infile) {
        std::cout << "input.txt 文件不存在，创建它..." << std::endl;
        createInputFile();
    }

    HuffmanNode* root = nullptr;

    while (true) {
        displayMenu();
        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1:
            buildHuffmanTree("input.txt", root); // 调用 buildHuffmanTree
            std::cout << "哈夫曼树已构建并保存到 hfmTree 文件。\n";
            break;
        case 2:
            encodeFile("input.txt", "CodeFile", root);
            std::cout << "编码文件已生成。\n";
            break;
        case 3:
            decodeFile("CodeFile", "TextFile", root);
            std::cout << "译码文件已生成。\n";
            break;
        case 4:
            printCodeFile("CodeFile", "CodePrint");
            std::cout << "编码文件已打印到 CodePrint。\n";
            break;
        case 5:
            printHuffmanTree(root);
            saveTreeToFile(root, "TreePrint");
            std::cout << "哈夫曼树已打印并保存到 TreePrint。\n";
            break;
        case 6:
            return 0;
        default:
            std::cout << "无效选项，请重新选择。\n";
        }
    }
}
