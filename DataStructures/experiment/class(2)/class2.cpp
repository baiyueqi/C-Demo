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

// 哈夫曼编码器类
class HuffmanCoder {
private:
    shared_ptr<HuffmanNode> root;           // 哈夫曼树根节点
    map<char, string> huffmanCodes;         // 字符到哈夫曼编码的映射(编码时使用)
    map<string, char> reverseHuffmanCodes;  // 哈夫曼编码到字符的映射(解码时使用)
    
    // 工具方法：读取二进制文件为字符串
    string readBinaryFileToString(const string& filename) {
        ifstream inFile(filename, ios::binary);
        if (!inFile) {
            cerr << "错误：无法打开文件 " << filename << endl;
            return "";
        }
        // 读取文件内容并转换为二进制字符串
        string bitString;
        char byte;
        while (inFile.get(byte)) {
            for (int i = 7; i >= 0; i--) {//从高位到低位
                bitString += (byte & (1 << i)) ? '1' : '0';
            }
        }
        inFile.close();
        return bitString;
    }
    
    // 格式化输出代码字符串
    void writeFormattedCode(const string& bitString, ostream& os, const string& printFile = "") {
        int lineCount = 0;
        ofstream outFile;
        if (!printFile.empty()) {
            outFile.open(printFile);
        }
        
        // 每行输出50位代码
        for (size_t i = 0; i < bitString.length(); i++) {//(每个位)
            os << bitString[i];
            //输出位
            if (outFile.is_open()) {
                outFile << bitString[i];
            }
            
            //已输出50位,换行并显示位置标记
            if ((i + 1) % 50 == 0) {
                os << " [" << setw(3) << (lineCount + 1) * 50 << "]" << endl;
                if (outFile.is_open()) {
                    outFile << endl;
                }
                lineCount++;
            }
        }
        
        // 处理最后一行不足50位的情况
        if (bitString.length() % 50 != 0) {
            int remaining = bitString.length() % 50;
            os << " [" << setw(3) << (lineCount * 50 + remaining) << "]" << endl;
            if (outFile.is_open()) {
                outFile << endl;
            }
        }
        
        // 关闭文件
        if (outFile.is_open()) {
            outFile.close();
        }
    }
    
    // 格式化字符显示
    string formatCharDisplay(char ch) {
        if (ch == '\n') return "\\n";
        else if (ch == ' ') return "空格";
        else if (ch == '\t') return "\\t";
        else return string(1, ch);
    }
    
public:
    // 构造函数
    HuffmanCoder() : root(nullptr) {}
    
    // 创建示例文件（只在需要时创建）
    bool createSampleFileIfNeeded(const string& filename) {
        ifstream testFile(filename);
        if (testFile.good()) {
            testFile.close();
            return false; // 文件已存在
        }
        
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "错误：无法创建文件 " << filename << endl;
            return false;
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
        return true;
    }
    
    // 1. 初始化：读取文本文件，统计字符频率，构建哈夫曼树
    void initialize(const string& filename) {
        // 检查并创建文件（如果不存在）
        bool created = createSampleFileIfNeeded(filename);
        if (created) {
            cout << "使用新创建的示例文件进行初始化..." << endl;
        } else {
            cout << "使用现有文件进行初始化..." << endl;
        }
        
        // 统计字符频率
        map<char, int> freqMap;
        ifstream inFile(filename);
        if (!inFile) {
            cerr << "错误：无法打开文件 " << filename << endl;
            return;
        }
        
        // 读取文件并统计频率
        char ch;
        while (inFile.get(ch)) {
            freqMap[ch]++;
        }
        inFile.close();
        
        cout << "字符频率统计：" << endl;
        cout << "----------------------------------------" << endl;
        for (const auto& pair : freqMap) {
            cout << formatCharDisplay(pair.first) << ": " << pair.second << endl;
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
    
    // 构建哈夫曼树
    void buildHuffmanTree(const map<char, int>& freqMap) {
        priority_queue<shared_ptr<HuffmanNode>, vector<shared_ptr<HuffmanNode>>, Compare> pq;
        
        // 创建叶子节点并加入优先队列
        for (const auto& pair : freqMap) {
            pq.push(make_shared<HuffmanNode>(pair.first, pair.second));//(字符, 频率)
        }
        
        // 构建哈夫曼树
        while (pq.size() > 1) {//队列中有多个节点
            auto left = pq.top(); pq.pop();//取出频率最小的两个节点
            auto right = pq.top(); pq.pop();//取出频率次小的节点
                        
            int sumFreq = left->freq + right->freq;//计算新节点频率
            auto parent = make_shared<HuffmanNode>(sumFreq, left, right);//创建新节点
            pq.push(parent);//将新节点加入队列
        }
        
        // 设置根节点
        if (!pq.empty()) {
            root = pq.top();//队列中最后一个节点为根节点
        }
    }
    
    // 生成哈夫曼编码
    void generateCodes(shared_ptr<HuffmanNode> node, string code) {//(节点, 当前编码)
        if (node == nullptr) return;
        // 叶子节点，保存编码
        if (node->isLeaf()) {//判断是叶子节点
            huffmanCodes[node->ch] = code;// 记录编码: 字符->当前编码;
            reverseHuffmanCodes[code] = node->ch;//记录反向映射: 当前编码->字符
        } else {
            generateCodes(node->left, code + "0");//左子节点, 当前编码 + "0"
            generateCodes(node->right, code + "1");//右子节点, 当前编码 + "1"
        }
    }
    
    // 显示编码表
    void displayCodeTable() {
        cout << "\n哈夫曼编码表：" << endl;
        cout << "----------------------------------------" << endl;
        for (const auto& pair : huffmanCodes) {
            cout << formatCharDisplay(pair.first) << ": " << pair.second << endl;
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
        collectCharFrequencies(root, charFreqs);//文件写入: 数据大小 + (字符,频率)对数组
        
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
        
        // 读取字符和频率信息
        map<char, int> freqMap;
        int size;
        inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
        
        // 读取(size)个(字符,频率)对
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
        
        // 读取输入文件并进行编码
        char ch;
        string encodedText;
        while (inFile.get(ch)) {
            encodedText += huffmanCodes[ch];//// 查表获取编码
        }
        
        // 计算压缩率
        int originalBits = encodedText.length();
        int originalBytes = originalBits / 8;
        int compressedBytes = (originalBits + 7) / 8;
        double compressionRatio = 0.0;
        if (originalBytes > 0) {
            compressionRatio = (1.0 - (double)compressedBytes / originalBytes) * 100;
        }
        
        // 将编码后的字符串转换为字节写入文件
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
        cout << "原始数据大小: " << originalBytes << " 字节" << endl;
        cout << "压缩后大小: " << compressedBytes << " 字节" << endl;
        cout << "压缩率: " << fixed << setprecision(2) << compressionRatio << "%" << endl;
    }
    
    // 3. 译码：对编码文件进行译码
    void decode(const string& inputFile, const string& outputFile) {
        if (root == nullptr) {
            loadHuffmanTree("hfmTree.dat");
        }
        
        // 使用工具方法读取二进制文件
        string bitString = readBinaryFileToString(inputFile);
        if (bitString.empty()) return;
        
        ofstream outFile(outputFile);
        if (!outFile) {
            cerr << "错误：无法创建文件 " << outputFile << endl;
            return;
        }
        
        // 解码(前缀匹配解码)
        string decodedText;
        string currentCode;
        for (char bit : bitString) {//每个位
            currentCode += bit;//currentCode += 当前位;
            if (reverseHuffmanCodes.find(currentCode) != reverseHuffmanCodes.end()) {//currentCode 在编码表中
                decodedText += reverseHuffmanCodes[currentCode];//输出对应字符
                currentCode.clear();// currentCode清空
            }
        }
        
        outFile << decodedText;
        outFile.close();
        
        cout << "译码完成！结果已保存到: " << outputFile << endl;
        cout << "译码字符数: " << decodedText.length() << " 个字符" << endl;
    }
    
    // 4. 打印代码文件
    void printCodeFile(const string& codeFile, const string& printFile) {
        // 使用工具方法读取二进制文件
        string bitString = readBinaryFileToString(codeFile);
        if (bitString.empty()) return;
        
        cout << "代码文件内容（紧凑格式，每行50个代码）：" << endl;
        cout << "----------------------------------------" << endl;
        
        // 使用工具方法格式化输出
        writeFormattedCode(bitString, cout, printFile);
        
        cout << "----------------------------------------" << endl;
        cout << "总位数: " << bitString.length() << endl;
        
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
    void printTree(shared_ptr<HuffmanNode> node, int level, ostream& os) {//(节点, 缩进层级, 输出流)
        if (node == nullptr) return;
        
        // 打印右子树（显示在上方）
        printTree(node->right, level + 1, os);//(右子树, 层级+1, 输出流)
        
        // 打印当前节点(输出缩进 + 节点信息)
        for (int i = 0; i < level; i++) {
            os << "    ";
        }
        
        if (node->isLeaf()) {
            os << formatCharDisplay(node->ch) << ":" << node->freq << endl;
        } else {
            os << "*:" << node->freq << endl;
        }
        
        // 打印左子树（显示在下方）
        printTree(node->left, level + 1, os);//(左子树, 层级+1, 输出流)
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

int main() {
    HuffmanCoder huffmanCoder;
    int choice;
    string inputFile, outputFile;
    
    cout << "欢迎使用哈夫曼编/译码器！" << endl;
    
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