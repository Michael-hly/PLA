package main.HuffSwingSeg;

import main.util.Encoding.VariableByteEncoder;
import main.util.Node;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.*;

public class Huffman {
    public static class BitInputStream {
        private InputStream inputStream;
        private int currentByte;
        private int numBitsRemaining;
        public BitInputStream(InputStream inputStream) {
            this.inputStream = inputStream;
            this.currentByte = 0;
            this.numBitsRemaining = 0;
        }
        public int readBit() throws IOException {
            if (currentByte == -1) {
                return -1;
            }
            if (numBitsRemaining == 0) {
                currentByte = inputStream.read();
                if (currentByte == -1) {
                    return -1;
                }
                numBitsRemaining = 8;
            }
            numBitsRemaining--;
            int bit = (currentByte >> numBitsRemaining) & 1;
            return bit;
        }
        public String readBits(int numBits) throws IOException {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < numBits; i++) {
                int bit = readBit();
                if (bit == -1) {
                    throw new IOException("End of stream reached");
                }
                sb.append(bit);
            }
            return sb.toString();
        }
    }
    public static class BitOutputStream {
        private ByteArrayOutputStream out;
        private int buffer;
        private int bufferLength;
        int getfull=0;
        public BitOutputStream(ByteArrayOutputStream out) {
            this.out = out;
            this.buffer = 0;
            this.bufferLength = 0;
        }
        public void writeBits(int code, int length) {
            if (length <= 0) {
                return;
            }


            for (int i = 0; i < length; i++) {
                int bit = (code >> (length - i - 1)) & 1;
                buffer = (buffer << 1) | bit;
                bufferLength++;


                if (bufferLength == 8) {
                    flushBuffer();
                }
            }
        }

        private void flushBuffer() {
            if(getfull==1&bufferLength > 0){
                while(bufferLength<8){
                    buffer = (buffer << 1) ;
                    bufferLength++;
                }
            }
            if (bufferLength > 0) {
                out.write(buffer);
                buffer = 0;
                bufferLength = 0;
            }
        }
        public void close() {
            getfull=1;
            flushBuffer();
            try {
                out.close();
            } catch (IOException e) {

                e.printStackTrace();
            }
        }
    }
    public static void huffmanencode(ArrayList<Integer> bstream, ByteArrayOutputStream outStream, ByteArrayOutputStream outStream1, ByteArrayOutputStream outStream2) throws IOException {
        BitOutputStream bitOut1 = new BitOutputStream(outStream1);
        BitOutputStream bitOut2 = new BitOutputStream(outStream2);
        Map<Integer, Integer> frequencyMap = new HashMap<>();
        for (int b : bstream) {
            if (frequencyMap.containsKey(b)) {
                frequencyMap.put(b, frequencyMap.get(b) + 1);
            } else {
                frequencyMap.put(b, 1);
            }
        }
        // 构建哈夫曼树和生成哈夫曼编码表
        List<Node> nodeList = new ArrayList<>();
        for (Map.Entry<Integer, Integer> entry : frequencyMap.entrySet()) {
            nodeList.add(new Node(entry.getKey(), entry.getValue()));
        }
        while (nodeList.size() > 1) {
            Collections.sort(nodeList, Comparator.comparingInt(o -> o.freq));
            Node left = nodeList.remove(0);
            Node right = nodeList.remove(0);
            Node parent = new Node(-10000, left.freq + right.freq);
            parent.left = left;
            parent.right = right;
            nodeList.add(parent);
        }
        TreeMap<Integer, String> huffmanCodeTable = new TreeMap<>();
        TreeMap<Integer, Integer> huffmanCodeTable1 = new TreeMap<>();
        TreeMap<Integer, Integer> codeLengthTable = new TreeMap<>();
        generateHuffmanCodes(nodeList.get(0), "", huffmanCodeTable);
        int key=0;
        VariableByteEncoder.write(huffmanCodeTable.size(), outStream);
        for (Map.Entry<Integer, String> entry : huffmanCodeTable.entrySet()) {
            VariableByteEncoder.write(entry.getKey()-key, outStream);
            key=entry.getKey();
            int length=entry.getValue().length();
            codeLengthTable.put(entry.getKey(),length);
            VariableByteEncoder.write(length, outStream);
            int code = Integer.parseInt(entry.getValue(), 2);
            huffmanCodeTable1.put(entry.getKey(),code);
            bitOut1.writeBits(code,length);
        }
        bitOut1.close();
        for (int data : bstream) {
            int code = huffmanCodeTable1.get(data);
            int length=codeLengthTable.get(data);
            bitOut2.writeBits(code,length);
        }
        bitOut2.close();

    }
    public static void huffmandecode(ByteArrayInputStream inStream, Integer bcount, ArrayList<MixSwingSegment> segments, Double epsilon) throws IOException {
        TreeMap<Integer, Integer> Bback = new TreeMap<>();
        int Bnum=VariableByteEncoder.read(inStream);
        int bvalue=0;
        int shortestlen=100;
        for(int i=0;i<Bnum;i++){
            bvalue+=VariableByteEncoder.read(inStream);
            int blength=VariableByteEncoder.read(inStream);
            if(blength<shortestlen)shortestlen=blength;
            Bback.put(bvalue,blength);
        }
        BitInputStream inputStream=new BitInputStream(inStream);
        Map<String,Integer> huffmanCodeTable = new HashMap<>();
        for(Map.Entry<Integer,Integer> b: Bback.entrySet()){
            huffmanCodeTable.put(inputStream.readBits(b.getValue()),b.getKey());
        }

        inputStream.numBitsRemaining=0;
        StringBuilder currentCode = new StringBuilder();
        int i=0;
        while (bcount>0) {

            int bit = inputStream.readBit();
            if (bit == -1) {
                break;
            }
            currentCode.append(bit);
            if(currentCode.length()<shortestlen)continue;
            if (huffmanCodeTable.containsKey(currentCode.toString())) {
                int b=huffmanCodeTable.get(currentCode.toString());
                segments.get(i).setB(epsilon*b);
                currentCode.setLength(0);
                i++;
                bcount--;
            }

        }
    }
    private static void generateHuffmanCodes(Node root, String code, Map<Integer, String> huffmanCodeTable) {
        if (root == null) {
            return;
        }
        if (root.val != -10000) {
            huffmanCodeTable.put(root.val, code);
        }
        generateHuffmanCodes(root.left, code + "0", huffmanCodeTable);
        generateHuffmanCodes(root.right, code + "1", huffmanCodeTable);
    }
}
