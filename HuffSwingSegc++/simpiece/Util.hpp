#include <iostream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <sstream>
#include <map>
using namespace std;


class FloatEncoder {
public:
    static void write(float number, std::stringstream& outputStream) {
        outputStream.write(reinterpret_cast<const char*>(&number), sizeof(float));
    }
    static float read(std::stringstream& inputStream) {
        float number;
        inputStream.read(reinterpret_cast<char*>(&number), sizeof(float));
        return number;
    }
};

class VariableByteEncoder {
private:
    static unsigned char extract7bits(int i, long val) {
        return (val >> (7 * i)) & ((1 << 7) - 1);
    }
    static unsigned char extract7bitsmaskless(int i, long val) {
        return val >> (7 * i);
    }
public:
    static void write(int number, std::stringstream& outputStream) {
        unsigned long val = number & 0xFFFFFFFF;
        if (val < (1 << 7)) {
            outputStream.put(static_cast<char>(val | (1 << 7)));
        }
        else if (val < (1 << 14)) {
            outputStream.put(static_cast<char>(extract7bits(0, val)));
            outputStream.put(static_cast<char>(extract7bitsmaskless(1, val) | (1 << 7)));
        }
        else if (val < (1 << 21)) {
            outputStream.put(static_cast<char>(extract7bits(0, val)));
            outputStream.put(static_cast<char>(extract7bits(1, val)));
            outputStream.put(static_cast<char>(extract7bitsmaskless(2, val) | (1 << 7)));
        }
        else if (val < (1 << 28)) {
            outputStream.put(static_cast<char>(extract7bits(0, val)));
            outputStream.put(static_cast<char>(extract7bits(1, val)));
            outputStream.put(static_cast<char>(extract7bits(2, val)));
            outputStream.put(static_cast<char>(extract7bitsmaskless(3, val) | (1 << 7)));
        }
        else {
            outputStream.put(static_cast<char>(extract7bits(0, val)));
            outputStream.put(static_cast<char>(extract7bits(1, val)));
            outputStream.put(static_cast<char>(extract7bits(2, val)));
            outputStream.put(static_cast<char>(extract7bits(3, val)));
            outputStream.put(static_cast<char>(extract7bitsmaskless(4, val) | (1 << 7)));
        }
    }
    static int read(std::stringstream& inputStream) {
        unsigned char in;
        int number = 0;
        inputStream.get(reinterpret_cast<char&>(in));
        number = in & 0x7F;
        if (in & 0x80) {
            return number;
        }
        inputStream.get(reinterpret_cast<char&>(in));
        number |= (in & 0x7F) << 7;
        if (in & 0x80) {
            return number;
        }
        inputStream.get(reinterpret_cast<char&>(in));
        number |= (in & 0x7F) << 14;
        if (in & 0x80) {
            return number;
        }
        inputStream.get(reinterpret_cast<char&>(in));
        number |= (in & 0x7F) << 21;
        if (in & 0x80) {
            return number;
        }
        inputStream.get(reinterpret_cast<char&>(in));
        number |= (in & 0x7F) << 28;
        return number;
    }
};

class Point {
private:
    long timestamp;
    double value;

public:
    Point(long timestamp, double value)
        : timestamp(timestamp), value(value) {}

    long getTimestamp()  { return timestamp; }

    double getValue()  { return value; }
};

class TimeSeries {
public:
    std::vector<Point> data;
    double range;
    int size;

public:
    TimeSeries(std::vector<Point> data, double range)
        : data(data), range(range), size(data.size()* (4 + 4)) {}
};

class TimeSeriesReader {
public:
    static TimeSeries getTimeSeries(FILE* inputFile, std::string delimiter) {
        std::vector<Point> ts;
        double value;
        long timestamp = 0;
        double max = std::numeric_limits<double>::lowest();
        double min = std::numeric_limits<double>::max();
        if (inputFile == NULL)
        {
            exit(1);
        }
        while (!feof(inputFile))
        {
            try
            {
                int f = fscanf(inputFile, "%lf", &value);
                ts.push_back(Point(timestamp, value));
                if (f == 0)
                {
                    exit(1);
                }
                if (max < value)
                {
                    max=value;
                }
                if (min > value)
                {
                    min = value;
                }
                timestamp++;
            }
            catch (std::exception e)
            {
                exit(1);
            }
        }
        fclose(inputFile);

        return TimeSeries(ts, max - min);
    }
};


class MixSwingSegment {
private:
    long initTimestamp;
    double aMin;
    double aMax;
    double a;
    double b;
public:
    MixSwingSegment(long initTimestamp, double a, double b) : initTimestamp(initTimestamp), a(a), aMin(a), aMax(a), b(b) {
    }
    MixSwingSegment(long initTimestamp, double aMin, double aMax, double b) : initTimestamp(initTimestamp), aMin(aMin), aMax(aMax), a((aMin + aMax) / 2), b(b) {
    }
    long getInitTimestamp() {
        return initTimestamp;
    }
    double getAMin() {
        return aMin;
    }
    double getAMax() {
        return aMax;
    }
    double getA() {
        return a;
    }
    double getB() {
        return b;
    }
    void setB(double b) {
        this->b = b;
    }
};
union DoubleUnion {
    double value;
    uint64_t bits;
};
double handle_double(double x, double y) {
    DoubleUnion u1, u2, result;
    u1.value = x;
    u2.value = y;
    if (x == y) {
        return x;
    }
    result.value = 0;
    if (x * y < 0) {
        return result.value;
    }
    result.value = x;
    if ((u1.bits & 0x7FF0000000000000ULL) != (u2.bits & 0x7FF0000000000000ULL)) {
        if ((u1.bits & 0x7FF0000000000000ULL) > (u2.bits & 0x7FF0000000000000ULL)) {
            result.bits = u1.bits & 0xFFF0000000000000ULL;
        }
        else {
            result.bits = u2.bits & 0xFFF0000000000000ULL;
        }
    }
    else {
        uint64_t mask = 0x0008000000000000ULL;
        while ((u1.bits & mask) == (u2.bits & mask)) {
            mask >>= 1;
        }
        uint64_t z = (u1.bits & (~mask + 1)) | mask;
        result.bits = z;
    }
    return result.value;
}
class VariableDoublebyteEncoder {
private:
    static unsigned char fromhigh7bits(int i, uint64_t val) {
        return val >> (64 - 7 * i);
    }
public:
    static void write(double number, stringstream& outputStream) {
        DoubleUnion value;
        value.value = number;
        uint64_t val = value.bits;
        for (int i = 1; i <= 9; ++i) {
            if ((val & (0xffffffffffffffffULL >> (7 * i))) == 0) {
                outputStream.put(static_cast<char>(fromhigh7bits(i, val) | (1 << 7)));
                break;
            }
            else {
                outputStream.put(static_cast<char>(fromhigh7bits(i, val) & ((1 << 7) - 1)));
                if (i == 9) {
                    outputStream.put(static_cast<char>(fromhigh7bits(10, val) | (1 << 7)));
                }
            }
        }
    }
    static double read(stringstream& inputStream) {
        DoubleUnion back;
        back.value = 0;
        unsigned char in;
        for (int i = 0; i < 10; ++i) {
            inputStream.get(reinterpret_cast<char&>(in));
            back.bits |= (static_cast<unsigned long long>(in & 0x7F)) << (64 - 7 * (i + 1));
            if (in & 0x80) {
                break;
            }
        }
        return back.value;
    }
};
class Node {
public:
    double val;
    int freq;
    Node* left;
    Node* right;

    Node(double val, int freq) : val(val), freq(freq), left(nullptr), right(nullptr) {}
};

class BitOutputStream {
private:
    std::stringstream out;
    int buffer;
    int bufferLength;

public:
    BitOutputStream() : buffer(0), bufferLength(0) {}

    void writeBits(int code, int length) {
        if (length <= 0) return; 

        for (int i = 0; i < length; i++) {
            int bit = (code >> (length - i - 1)) & 1; 
            buffer = (buffer << 1) | bit; 
            bufferLength++;

            if (bufferLength == 8) {
                flushBuffer();
            }
        }
    }
    void flushBuffer() {

        if (bufferLength > 0) {
            char c = static_cast<char>(buffer << (8 - bufferLength));
            out.write(reinterpret_cast<const char*>(&c), 1);

            buffer = 0;
            bufferLength = 0;
        }
    }

    void close() {

        flushBuffer(); 
    }

    std::string getOutput() {
        return out.str();
    }
    void appendTo(std::stringstream& outStream) {
        std::string outContent = out.str(); 
        outStream << outContent; 
    }
};
void generateHuffmanCodes(Node* root, std::string code, map<int, std::string>& huffmanCodeTable) {
    if (!root) return;
    if (root->val != -10000) {
        huffmanCodeTable[root->val] = code;
    }
    generateHuffmanCodes(root->left, code + "0", huffmanCodeTable);
    generateHuffmanCodes(root->right, code + "1", huffmanCodeTable);
}

void huffmanencode(std::vector<int>& bstream, std::stringstream& outStream) {
    BitOutputStream bitOut1;
    map<int, int> frequencyMap;
    for (int b : bstream) {
        if (frequencyMap.find(b) != frequencyMap.end()) {
            frequencyMap[b]++;
        }
        else {
            frequencyMap[b] = 1;
        }
    }
    std::vector<Node*> nodeList;
    for (auto entry : frequencyMap) {
        nodeList.push_back(new Node(entry.first, entry.second));
    }
    while (nodeList.size() > 1) {
        std::sort(nodeList.begin(), nodeList.end(), [](Node* a, Node* b) { return a->freq < b->freq; });
        Node* left = nodeList[0];
        Node* right = nodeList[1];
        Node* parent = new Node(-10000, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        nodeList.erase(nodeList.begin(), nodeList.begin() + 2);
        nodeList.push_back(parent);
    }
    map<int, std::string> huffmanCodeTable;
    map<int, int> huffmanCodeTable1;
    map<int, int> codeLengthTable;
    generateHuffmanCodes(nodeList[0], "", huffmanCodeTable);
    int key = 0;

    VariableByteEncoder::write(huffmanCodeTable.size(), outStream);

    for (auto entry : huffmanCodeTable) {
        VariableByteEncoder::write(entry.first - key, outStream);
        key = entry.first;
        int length = entry.second.length();
        codeLengthTable[entry.first] = length;
        VariableByteEncoder::write(length, outStream);
        int code = std::stoi(entry.second, nullptr, 2);
        huffmanCodeTable1[entry.first] = code;

        bitOut1.writeBits(code, length);
    }
    bitOut1.close();
    for (int data : bstream) {
        int code = huffmanCodeTable1[data];
        int length = codeLengthTable[data];

        bitOut1.writeBits(code, length);
    }
    bitOut1.close();
    bitOut1.appendTo(outStream);
}



class BitInputStream {
private:
    std::stringstream& inputStream;
    unsigned char currentByte;
    int numBitsRemaining;

public:
    BitInputStream(std::stringstream& inputStream)
        : inputStream(inputStream), currentByte(0), numBitsRemaining(0) {}

    void setbr(int b) {
        numBitsRemaining = b;
    }
    int readBit() {
        if (currentByte == -1) {
            return -1; // End of stream
        }
        if (numBitsRemaining == 0) {
            inputStream.read(reinterpret_cast<char*>(&currentByte), sizeof(currentByte));
            if (currentByte == -1) {
                return -1; 
            }
            numBitsRemaining = 8;
        }
        numBitsRemaining--;
        int bit = (currentByte >> numBitsRemaining) & 1;
        return bit;
    }

    std::string readBits(int numBits) {
        std::string bits;
        for (int i = 0; i < numBits; i++) {
            int bit = readBit();
            if (bit == -1) {
                throw std::runtime_error("End of stream reached");
            }
            bits += std::to_string(bit);
        }
        return bits;
    }
};


void huffmandecode(std::stringstream& inStream, int bcount, std::vector<MixSwingSegment>& segments, double epsilon) {
    map<int, int> Bback;
    int Bnum = VariableByteEncoder::read(inStream);
    int bvalue = 0;
    int shortestlen = 100;
    for (int i = 0; i < Bnum; i++) {
        bvalue += VariableByteEncoder::read(inStream);
        int blength = VariableByteEncoder::read(inStream);
        if (blength < shortestlen) {
            shortestlen = blength;
        }
        Bback[bvalue] = blength;
    }

    BitInputStream inputStream(inStream);
    map<std::string, int> huffmanCodeTable;
    for (auto b : Bback) {
        huffmanCodeTable[inputStream.readBits(b.second)] = b.first;
    }
    inputStream.setbr(0);

    std::string currentCode;
    int i = 0;
    while (bcount > 0) {

        int bit = inputStream.readBit();
        if (bit == -1) {
            break; 
        }

        currentCode += std::to_string(bit);
        if (currentCode.length() < shortestlen) {
            continue;
        }

        if (huffmanCodeTable.find(currentCode) != huffmanCodeTable.end()) {
            int b = huffmanCodeTable[currentCode];
            segments[i].setB(epsilon * b);
            currentCode = "";
            i++;
            bcount--;
        }
    }
}