package main.HuffSwingSeg;

import main.util.Encoding.*;
import main.util.Pair;
import main.util.Point;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.*;

import static main.HuffSwingSeg.Huffman.huffmandecode;
import static main.HuffSwingSeg.Huffman.huffmanencode;
import static main.util.Encoding.VariableDoublebyteEncoder.handleDouble;

public class MixSwing {
    private ArrayList<MixSwingSegment> segments;

    private static double epsilon;
    private long lastTimeStamp;

    public MixSwing(List<Point> points, double epsilon) throws IOException {
        if (points.isEmpty()) throw new IOException();

        this.epsilon = epsilon;
        this.lastTimeStamp = points.get(points.size()-1).getTimestamp();
        this.segments = mergePerB(compress(points));
    }

    public MixSwing(byte[] bytes) throws IOException {
        readByteArray(bytes);
    }

    private static double quantization(double value, int mode) {
        if (mode == 1) return (int) Math.ceil(value / epsilon) * epsilon;
        else if (mode == 2) return (int) Math.floor(value / epsilon) * epsilon;
        else return Math.round(value / epsilon) * epsilon;
    }
    private static int createSegment(int startIdx, List<Point> points, ArrayList<MixSwingSegment> segments, int quantizationMode) {
        long initTimestamp = points.get(startIdx).getTimestamp();
        double b = quantization(points.get(startIdx).getValue(), quantizationMode);
        if (startIdx + 1 == points.size()) {
            segments.add(new MixSwingSegment(initTimestamp, -Double.MAX_VALUE, Double.MAX_VALUE, b));
            return startIdx + 1;
        }
        double aMax = ((points.get(startIdx + 1).getValue() + epsilon) - b) / (points.get(startIdx + 1).getTimestamp() - initTimestamp);
        double aMin = ((points.get(startIdx + 1).getValue() - epsilon) - b) / (points.get(startIdx + 1).getTimestamp() - initTimestamp);
        if (startIdx + 2 == points.size()) {
            segments.add(new MixSwingSegment(initTimestamp, aMin, aMax, b));
            return startIdx + 2;
        }

        for (int idx = startIdx + 2; idx < points.size(); idx++) {
            double upValue = points.get(idx).getValue() + epsilon;
            double downValue = points.get(idx).getValue() - epsilon;

            double upLim = aMax * (points.get(idx).getTimestamp() - initTimestamp) + b;
            double downLim = aMin * (points.get(idx).getTimestamp() - initTimestamp) + b;
            if ((downValue > upLim || upValue < downLim)) {
                segments.add(new MixSwingSegment(initTimestamp, aMin, aMax, b));
                return idx;
            }

            if (upValue < upLim)
                aMax = Math.max((upValue - b) / (points.get(idx).getTimestamp() - initTimestamp), aMin);
            if (downValue > downLim)
                aMin = Math.min((downValue - b) / (points.get(idx).getTimestamp() - initTimestamp), aMax);
        }
        segments.add(new MixSwingSegment(initTimestamp, aMin, aMax, b));

        return points.size();
    }

    private static ArrayList<MixSwingSegment> compress(List<Point> points) {
        ArrayList<MixSwingSegment> segments = new ArrayList<>();
        int currentIdx = 0;
        while (currentIdx < points.size()) {
            int currentCeilIdx = createSegment(currentIdx, points, segments, 1);
            int currentFloorIdx = createSegment(currentIdx, points, segments, 2);
            if (currentCeilIdx > currentFloorIdx) {
                segments.remove(segments.size() - 1);
                currentIdx = currentCeilIdx;
            } else if (currentCeilIdx < currentFloorIdx) {
                segments.remove(segments.size() - 2);
                currentIdx = currentFloorIdx;
            } else {
                double firstValue = points.get(currentIdx).getValue();
                if (Math.round(firstValue / epsilon) == Math.ceil(firstValue / epsilon))
                    segments.remove(segments.size() - 1);
                else segments.remove(segments.size() - 2);
                currentIdx = currentFloorIdx;
            }
        }
        return segments;
    }

    private ArrayList<MixSwingSegment> mergePerB(ArrayList<MixSwingSegment> segments) {
        double aMinTemp = -Double.MAX_VALUE;
        double aMaxTemp = Double.MAX_VALUE;
        ArrayList<MixSwingSegment> mergedSegments = new ArrayList<>();
        ArrayList<MixSwingSegment> recordSegments = new ArrayList<>();
        segments.sort(Comparator.comparingDouble(MixSwingSegment::getAMin));

        MixSwingSegment currentdSeg;
        for (int i = 0; i < segments.size(); i++) {
            currentdSeg=segments.get(i);
            if (currentdSeg.getAMin() <= aMaxTemp && currentdSeg.getAMax() >= aMinTemp) {
                recordSegments.add(segments.get(i));
                aMinTemp = Math.max(aMinTemp, currentdSeg.getAMin());
                aMaxTemp = Math.min(aMaxTemp, currentdSeg.getAMax());
            }
            else {
                for(MixSwingSegment seg:recordSegments)
                    mergedSegments.add(new MixSwingSegment(seg.getInitTimestamp(),aMinTemp, aMaxTemp,seg.getB()));
                aMinTemp = currentdSeg.getAMin();
                aMaxTemp = currentdSeg.getAMax();
                recordSegments.clear();
                recordSegments.add(currentdSeg);
            }
        }
        if (!recordSegments.isEmpty()) {
            for(MixSwingSegment seg:recordSegments)
                mergedSegments.add(new MixSwingSegment(seg.getInitTimestamp(),aMinTemp, aMaxTemp,seg.getB()));
            recordSegments.clear();
        }
        mergedSegments.sort(Comparator.comparingLong(MixSwingSegment::getInitTimestamp));

        return mergedSegments;
    }

    public List<Point> decompress() {
        segments.sort(Comparator.comparingLong(MixSwingSegment::getInitTimestamp));

        List<Point> points = new ArrayList<>();
        long currentTimeStamp = segments.get(0).getInitTimestamp();


        for (int i = 0; i < segments.size() - 1; i++) {
            while (currentTimeStamp < segments.get(i + 1).getInitTimestamp()) {
                points.add(new Point(currentTimeStamp, segments.get(i).getA() * (currentTimeStamp - segments.get(i).getInitTimestamp()) + segments.get(i).getB()));
                currentTimeStamp++;
            }
        }

        while (currentTimeStamp <= lastTimeStamp) {
            points.add(new Point(currentTimeStamp, segments.get(segments.size() - 1).getA() * (currentTimeStamp - segments.get(segments.size() - 1).getInitTimestamp()) + segments.get(segments.size() - 1).getB()));
            currentTimeStamp++;
        }

        return points;
    }

    private void toByteArrayPerBSegments(ArrayList<MixSwingSegment> segments, ByteArrayOutputStream outStream,ByteArrayOutputStream outStream1,ByteArrayOutputStream outStream2) throws IOException {
        LinkedHashMap<Double, ArrayList<Pair<Integer, Integer>>> input = new LinkedHashMap<>();
        ArrayList<Integer> bstream=new ArrayList<>();
        for (MixSwingSegment segment : segments) {
            double a = handleDouble(segment.getAMin(),segment.getAMax());

            int b = (int) Math.round(segment.getB() / epsilon);
            int t = (int) segment.getInitTimestamp();

            if (!input.containsKey(a)) {
                input.put(a, new ArrayList<>());
            }

            List<Pair<Integer, Integer>> pairs = input.get(a);
            pairs.add(new Pair<>(t, b));
        }

        VariableByteEncoder.write(input.size(), outStream);//a
        if (input.isEmpty()) return;
        int preT=0;

        for (Map.Entry<Double, ArrayList<Pair<Integer, Integer>>> aSegments : input.entrySet()) {
            VariableDoublebyteEncoder.write(aSegments.getKey(), outStream);

            VariableByteEncoder.write(aSegments.getValue().size(), outStream);//b/t
            VariableByteEncoder.write((aSegments.getValue().get(0).getFirst()- preT), outStream);//first t
            preT=aSegments.getValue().get(0).getFirst();
            int previousTS =preT;
            int count=0;
            for(Pair<Integer, Integer> list: aSegments.getValue()) {
                if(count>0){
                    VariableByteEncoder.write((list.getFirst() - previousTS), outStream);
                }
                count=1;
                previousTS = list.getFirst();
                bstream.add(list.getSecond());//b
            }
        }
        VariableByteEncoder.write((int) lastTimeStamp, outStream);

        huffmanencode(bstream,outStream,outStream1,outStream2);

    }

    public byte[] toByteArray() throws IOException {
        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        ByteArrayOutputStream outStream1 = new ByteArrayOutputStream();
        ByteArrayOutputStream outStream2 = new ByteArrayOutputStream();

        byte[] bytes;

        FloatEncoder.write((float) epsilon, outStream);

        toByteArrayPerBSegments(segments, outStream,outStream1,outStream2);


        outStream.write(outStream1.toByteArray());
        outStream.write(outStream2.toByteArray());

        bytes = outStream.toByteArray();

        outStream.close();

        return bytes;
    }

    private ArrayList<MixSwingSegment> readMergedPerBSegments(Double epsilon, ByteArrayInputStream inStream) throws IOException {
        ArrayList<MixSwingSegment> segments = new ArrayList<>();
        int numA = VariableByteEncoder.read(inStream);
        if (numA == 0) return segments;
        int preT=0;
        int bcount=0;
        for (int i = 0; i < numA; i++) {
            double a = VariableDoublebyteEncoder.read(inStream);
            int numBort = VariableByteEncoder.read(inStream);
            bcount+=numBort;
            int t=0;
            t = VariableByteEncoder.read(inStream) + preT;
            preT=t;
            int counter = 0;
            for (int j = 0; j < numBort; j++) {
                if (counter > 0) t += VariableByteEncoder.read(inStream);
                counter = 1;
                segments.add(new MixSwingSegment(t, a, 0));
            }
        }
        this.lastTimeStamp = VariableByteEncoder.read(inStream);
        huffmandecode(inStream,bcount,segments,epsilon);

        return segments;
    }
    private void readByteArray(byte[] input) throws IOException {
        byte[] binary;
        binary = input;
        ByteArrayInputStream inStream = new ByteArrayInputStream(binary);

        this.epsilon = FloatEncoder.read(inStream);
        this.segments = readMergedPerBSegments(epsilon, inStream);

        inStream.close();
    }
}
