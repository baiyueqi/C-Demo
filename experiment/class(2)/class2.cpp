#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <iomanip>
#include <memory>

using namespace std;

// 哈夫曼树节点类
class HuffmanNode {
public:
    char ch;           // 字符
    int freq;          // 频率
    shared_ptr<HuffmanNode> left;   // 左子节点
    shared_ptr<HuffmanNode> right;  // 右子节点
    
    HuffmanNode(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    HuffmanNode(int f, shared_ptr<HuffmanNode> l, shared_ptr<HuffmanNode> r) 
        : ch('\0'), freq(f), left(l), right(r) {}
    
    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

// 比较函数，用于优先队列
struct Compare {
    bool operator()(const shared_ptr<HuffmanNode>& a, const shared_ptr<HuffmanNode>& b) {
        return a->freq > b->freq;
    }
};

class HuffmanCoder {
private:
    shared_ptr<HuffmanNode> root;           // 哈夫曼树根节点
    map<char, string> huffmanCodes;         // 字符到哈夫曼编码的映射
    map<string, char> reverseHuffmanCodes;  // 哈夫曼编码到字符的映射
    
public:
    // 构造函数
    HuffmanCoder() : root(nullptr) {}
    
    // 1. 初始化：读取文本文件，统计字符频率，构建哈夫曼树
    void initialize(const string& filename) {
        // 创建示例文本文件（如果不存在）
        createSampleFile(filename);
        
        // 统计字符频率
        map<char, int> freqMap;
        ifstream inFile(filename);
        if (!inFile) {
            cerr << "错误：无法打开文件 " << filename << endl;
            return;
        }
        
        char ch;
        while (inFile.get(ch)) {
            freqMap[ch]++;
        }
        inFile.close();
        
        cout << "字符频率统计：" << endl;
        cout << "----------------------------------------" << endl;
        for (const auto& pair : freqMap) {
            if (pair.first == '\n') {
                cout << "\\n: " << pair.second << endl;
            } else if (pair.first == ' ') {
                cout << "空格: " << pair.second << endl;
            } else if (pair.first == '\t') {
                cout << "\\t: " << pair.second << endl;
            } else {
                cout << pair.first << ": " << pair.second << endl;
            }
        }
        cout << "----------------------------------------" << endl;
        
        // 构建哈夫曼树
        buildHuffmanTree(freqMap);
        
        // 生成哈夫曼编码
        generateCodes(root, "");
        
        // 将哈夫曼树保存到文件
        saveHuffmanTree("hfmTree.dat");
        
        // 显示编码表
        displayCodeTable();
        
        cout << "初始化完成！共统计 " << freqMap.size() << " 种字符。" << endl;
    }
    
    // 创建示例文本文件
    void createSampleFile(const string& filename) {
        ifstream testFile(filename);
        if (testFile.good()) {
            testFile.close();
            return; // 文件已存在
        }
        
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "错误：无法创建文件 " << filename << endl;
            return;
        }
        
        // 写入示例文本
        outFile << "this is a sample text for huffman coding\n";
        outFile << "hello world! this is huffman coder implementation.\n";
        outFile << "abcdefghijklmnopqrstuvwxyz\n";
        outFile << "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
        outFile << "1234567890\n";
        outFile << "!@#$%^&*()_+-=[]{}|;:,.<>?";
        
        outFile.close();
        cout << "已创建示例文件: " << filename << endl;
    }
    
    // 构建哈夫曼树
    void buildHuffmanTree(const map<char, int>& freqMap) {
        priority_queue<shared_ptr<HuffmanNode>, vector<shared_ptr<HuffmanNode>>, Compare> pq;
        
        // 创建叶子节点并加入优先队列
        for (const auto& pair : freqMap) {
            pq.push(make_shared<HuffmanNode>(pair.first, pair.second));
        }
        
        // 构建哈夫曼树
        while (pq.size() > 1) {
            auto left = pq.top(); pq.pop();
            auto right = pq.top(); pq.pop();
            
            int sumFreq = left->freq + right->freq;
            auto parent = make_shared<HuffmanNode>(sumFreq, left, right);
            pq.push(parent);
        }
        
        if (!pq.empty()) {
            root = pq.top();
        }
    }
    
    // 生成哈夫曼编码
    void generateCodes(shared_ptr<HuffmanNode> node, string code) {
        if (node == nullptr) return;
        
        if (node->isLeaf()) {
            huffmanCodes[node->ch] = code;
            reverseHuffmanCodes[code] = node->ch;
        } else {
            generateCodes(node->left, code + "0");
            generateCodes(node->right, code + "1");
        }
    }
    
    // 显示编码表
    void displayCodeTable() {
        cout << "\n哈夫曼编码表：" << endl;
        cout << "----------------------------------------" << endl;
        for (const auto& pair : huffmanCodes) {
            if (pair.first == '\n') {
                cout << "\\n: " << pair.second << endl;
            } else if (pair.first == ' ') {
                cout << "空格: " << pair.second << endl;
            } else if (pair.first == '\t') {
                cout << "\\t: " << pair.second << endl;
            } else {
                cout << pair.first << ": " << pair.second << endl;
            }
        }
        cout << "----------------------------------------" << endl;
    }
    
    // 保存哈夫曼树到文件
    void saveHuffmanTree(const string& filename) {
        ofstream outFile(filename, ios::binary);
        if (!outFile) {
            cerr << "错误：无法创建文件 " << filename << endl;
            return;
        }
        
        // 保存字符和频率信息
        vector<pair<char, int>> charFreqs;
        collectCharFrequencies(root, charFreqs);
        
        int size = charFreqs.size();
        outFile.write(reinterpret_cast<char*>(&size), sizeof(size));
        
        for (const auto& cf : charFreqs) {
            outFile.write(reinterpret_cast<const char*>(&cf.first), sizeof(cf.first));
            outFile.write(reinterpret_cast<const char*>(&cf.second), sizeof(cf.second));
        }
        
        outFile.close();
        cout << "哈夫曼树已保存到文件: " << filename << endl;
    }
    
    // 收集字符频率信息
    void collectCharFrequencies(shared_ptr<HuffmanNode> node, vector<pair<char, int>>& result) {
        if (node == nullptr) return;
        
        if (node->isLeaf()) {
            result.push_back({node->ch, node->freq});
        } else {
            collectCharFrequencies(node->left, result);
            collectCharFrequencies(node->right, result);
        }
    }
    
    // 从文件加载哈夫曼树
    void loadHuffmanTree(const string& filename) {
        ifstream inFile(filename, ios::binary);
        if (!inFile) {
            cerr << "错误：无法打开文件 " << filename << endl;
            return;
        }
        
        map<char, int> freqMap;
        int size;
        inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
        
        for (int i = 0; i < size; i++) {
            char ch;
            int freq;
            inFile.read(reinterpret_cast<char*>(&ch), sizeof(ch));
            inFile.read(reinterpret_cast<char*>(&freq), sizeof(freq));
            freqMap[ch] = freq;
        }
        
        inFile.close();
        
        // 重建哈夫曼树和编码
        buildHuffmanTree(freqMap);
        generateCodes(root, "");
        
        cout << "哈夫曼树已从文件加载。" << endl;
    }
    
    // 2. 编码：对文本文件进行编码
    void encode(const string& inputFile, const string& outputFile) {
        if (root == nullptr) {
            loadHuffmanTree("hfmTree.dat");
        }
        
        ifstream inFile(inputFile);
        ofstream outFile(outputFile, ios::binary);
        
        if (!inFile || !outFile) {
            cerr << "错误：无法打开文件" << endl;
            return;
        }
        
        char ch;
        string encodedText;
        while (inFile.get(ch)) {
            encodedText += huffmanCodes[ch];
        }
        
        // 计算压缩率
        int originalBits = encodedText.length();
        int compressedBytes = (originalBits + 7) / 8;
        double compressionRatio = (1.0 - (double)compressedBytes / (originalBits / 8.0)) * 100;
        
        // 将编码后的字符串转换为字节写入文件
        string byteString;
        for (size_t i = 0; i < encodedText.length(); i += 8) {
            string byte = encodedText.substr(i, 8);
            while (byte.length() < 8) {
                byte += "0"; // 填充
            }
            unsigned char b = 0;
            for (int j = 0; j < 8; j++) {
                b = (b << 1) | (byte[j] - '0');
            }
            outFile.put(b);
        }
        
        inFile.close();
        outFile.close();
        
        cout << "编码完成！结果已保存到: " << outputFile << endl;
        cout << "原始数据大小: " << (originalBits / 8) << " 字节" << endl;
        cout << "压缩后大小: " << compressedBytes << " 字节" << endl;
        cout << "压缩率: " << fixed << setprecision(2) << compressionRatio << "%" << endl;
    }
    
    // 3. 译码：对编码文件进行译码
    void decode(const string& inputFile, const string& outputFile) {
        if (root == nullptr) {
            loadHuffmanTree("hfmTree.dat");
        }
        
        ifstream inFile(inputFile, ios::binary);
        ofstream outFile(outputFile);
        
        if (!inFile || !outFile) {
            cerr << "错误：无法打开文件" << endl;
            return;
        }
        
        // 读取二进制数据并转换为位字符串
        string bitString;
        char byte;
        while (inFile.get(byte)) {
            for (int i = 7; i >= 0; i--) {
                bitString += (byte & (1 << i)) ? '1' : '0';
            }
        }
        
        // 解码
        string decodedText;
        string currentCode;
        for (char bit : bitString) {
            currentCode += bit;
            if (reverseHuffmanCodes.find(currentCode) != reverseHuffmanCodes.end()) {
                decodedText += reverseHuffmanCodes[currentCode];
                currentCode.clear();
            }
        }
        
        outFile << decodedText;
        
        inFile.close();
        outFile.close();
        
        cout << "译码完成！结果已保存到: " << outputFile << endl;
        cout << "译码字符数: " << decodedText.length() << " 个字符" << endl;
    }
    
    // 4. 打印代码文件
    void printCodeFile(const string& codeFile, const string& printFile) {
        ifstream inFile(codeFile, ios::binary);
        ofstream outFile(printFile);
        
        if (!inFile || !outFile) {
            cerr << "错误：无法打开文件" << endl;
            return;
        }
        
        cout << "代码文件内容（紧凑格式，每行50个代码）：" << endl;
        cout << "----------------------------------------" << endl;
        
        string bitString;
        char byte;
        while (inFile.get(byte)) {
            for (int i = 7; i >= 0; i--) {
                bitString += (byte & (1 << i)) ? '1' : '0';
            }
        }
        
        // 输出到屏幕和文件
        int lineCount = 0;
        for (size_t i = 0; i < bitString.length(); i++) {
            cout << bitString[i];
            outFile << bitString[i];
            
            if ((i + 1) % 50 == 0) {
                cout << " [" << setw(3) << (lineCount + 1) * 50 << "]" << endl;
                outFile << endl;
                lineCount++;
            }
        }
        
        if (bitString.length() % 50 != 0) {
            int remaining = bitString.length() % 50;
            cout << " [" << setw(3) << (lineCount * 50 + remaining) << "]" << endl;
            outFile << endl;
        }
        
        cout << "----------------------------------------" << endl;
        cout << "总位数: " << bitString.length() << endl;
        inFile.close();
        outFile.close();
        
        cout << "代码打印完成！结果已保存到: " << printFile << endl;
    }
    
    // 5. 打印哈夫曼树
    void printHuffmanTree(const string& printFile) {
        ofstream outFile(printFile);
        if (!outFile) {
            cerr << "错误：无法创建文件 " << printFile << endl;
            return;
        }
        
        cout << "哈夫曼树结构（顺时针旋转90度）：" << endl;
        cout << "----------------------------------------" << endl;
        
        printTree(root, 0, cout);
        printTree(root, 0, outFile);
        
        cout << "----------------------------------------" << endl;
        outFile.close();
        
        cout << "哈夫曼树打印完成！结果已保存到: " << printFile << endl;
    }
    
    // 6. 显示编码表
    void showCodeTable() {
        if (huffmanCodes.empty()) {
            cout << "请先初始化哈夫曼树！" << endl;
            return;
        }
        
        displayCodeTable();
    }
    
private:
    // 递归打印树结构
    void printTree(shared_ptr<HuffmanNode> node, int level, ostream& os) {
        if (node == nullptr) return;
        
        // 打印右子树
        printTree(node->right, level + 1, os);
        
        // 打印当前节点
        for (int i = 0; i < level; i++) {
            os << "    ";
        }
        
        if (node->isLeaf()) {
            if (node->ch == '\n') {
                os << "\\n:" << node->freq << endl;
            } else if (node->ch == ' ') {
                os << "space:" << node->freq << endl;
            } else if (node->ch == '\t') {
                os << "\\t:" << node->freq << endl;
            } else {
                os << node->ch << ":" << node->freq << endl;
            }
        } else {
            os << "*:" << node->freq << endl;
        }
        
        // 打印左子树
        printTree(node->left, level + 1, os);
    }
};

// 显示菜单
void showMenu() {
    cout << "\n========== 哈夫曼编/译码器 ==========" << endl;
    cout << "1. 初始化（构建哈夫曼树）" << endl;
    cout << "2. 编码" << endl;
    cout << "3. 译码" << endl;
    cout << "4. 打印代码文件" << endl;
    cout << "5. 打印哈夫曼树" << endl;
    cout << "6. 显示哈夫曼编码表" << endl;
    cout << "0. 退出" << endl;
    cout << "======================================" << endl;
    cout << "请选择操作: ";
}

// 创建测试文件
void createTestFiles() {
    cout << "创建测试文件中..." << endl;
    
    // 创建示例文本文件
    ofstream sampleFile("sample.txt");
    sampleFile << "this is a sample text for huffman coding\n";
    sampleFile << "hello world! this is huffman coder implementation.\n";
    sampleFile << "abcdefghijklmnopqrstuvwxyz\n";
    sampleFile << "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
    sampleFile << "1234567890\n";
    sampleFile << "!@#$%^&*()_+-=[]{}|;:,.<>?";
    sampleFile.close();
    
    cout << "测试文件 'sample.txt' 已创建！" << endl;
}

int main() {
    HuffmanCoder huffmanCoder;
    int choice;
    string inputFile, outputFile;
    
    cout << "欢迎使用哈夫曼编/译码器！" << endl;
    createTestFiles();
    
    do {
        showMenu();
        cin >> choice;
        cin.ignore(); // 清除输入缓冲区
        
        switch (choice) {
            case 1:
                cout << "请输入要统计的文本文件名 (默认: sample.txt): ";
                getline(cin, inputFile);
                if (inputFile.empty()) inputFile = "sample.txt";
                huffmanCoder.initialize(inputFile);
                break;
                
            case 2:
                cout << "请输入要编码的文本文件名 (默认: sample.txt): ";
                getline(cin, inputFile);
                if (inputFile.empty()) inputFile = "sample.txt";
                cout << "请输入编码结果文件名 (默认: CodeFile): ";
                getline(cin, outputFile);
                if (outputFile.empty()) outputFile = "CodeFile";
                huffmanCoder.encode(inputFile, outputFile);
                break;
                
            case 3:
                cout << "请输入要译码的编码文件名 (默认: CodeFile): ";
                getline(cin, inputFile);
                if (inputFile.empty()) inputFile = "CodeFile";
                cout << "请输入译码结果文件名 (默认: TextFile): ";
                getline(cin, outputFile);
                if (outputFile.empty()) outputFile = "TextFile";
                huffmanCoder.decode(inputFile, outputFile);
                break;
                
            case 4:
                cout << "请输入代码文件名 (默认: CodeFile): ";
                getline(cin, inputFile);
                if (inputFile.empty()) inputFile = "CodeFile";
                huffmanCoder.printCodeFile(inputFile, "CodePrint.txt");
                break;
                
            case 5:
                huffmanCoder.printHuffmanTree("TreePrint.txt");
                break;
                
            case 6:
                huffmanCoder.showCodeTable();
                break;
                
            case 0:
                cout << "感谢使用哈夫曼编/译码器！" << endl;
                break;
                
            default:
                cout << "无效选择，请重新输入！" << endl;
                break;
        }
        
    } while (choice != 0);
    
    return 0;
}