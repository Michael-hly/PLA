#define _CRT_SECURE_NO_WARNINGS
#include "Util.hpp"
#include <map>
#include <cassert>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include <fstream>


class MixSwing {
public:

    std::vector<MixSwingSegment> Segments;
    double epsilon;
    long lastTimeStamp;
    double quantization(double value, int mode) {
        if (mode == 1) {
            return std::ceil(value / epsilon) * epsilon;
        } else if (mode == 2) {
            return std::floor(value / epsilon) * epsilon;
        } else {
            return std::round(value / epsilon) * epsilon;
        }
    }
    int createSegment(int startIdx,  std::vector<Point>& points, std::vector<MixSwingSegment>& segments, int quantizationMode) {
        long initTimestamp = points[startIdx].getTimestamp();
        double b = quantization(points[startIdx].getValue(), quantizationMode);
        if (startIdx + 1 == points.size() ) {
            segments.push_back(MixSwingSegment(initTimestamp, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), b));
            return startIdx + 1;
        }
        double aMax = ((points[startIdx + 1].getValue() + epsilon) - b) / (points[startIdx + 1].getTimestamp() - initTimestamp);
        double aMin = ((points[startIdx + 1].getValue() - epsilon) - b) / (points[startIdx + 1].getTimestamp() - initTimestamp);
        if (startIdx + 2 == points.size()) {
            segments.push_back(MixSwingSegment(initTimestamp, aMin, aMax, b));
            return startIdx + 2;
        }
        for (size_t idx = startIdx + 2; idx < points.size() ; ++idx) {
            double upValue = points[idx].getValue() + epsilon;
            double downValue = points[idx].getValue() - epsilon;
            double upLim = aMax * (points[idx].getTimestamp() - initTimestamp) + b;
            double downLim = aMin * (points[idx].getTimestamp() - initTimestamp) + b;
            if ((downValue > upLim || upValue < downLim)) {
                segments.push_back(MixSwingSegment(initTimestamp, aMin, aMax, b));
                return static_cast<int>(idx);
            }
            if (upValue < upLim)
                aMax = std::max((upValue - b) / (points[idx].getTimestamp() - initTimestamp), aMin);
            if (downValue > downLim)
                aMin = std::min((downValue - b) / (points[idx].getTimestamp() - initTimestamp), aMax);
        }
        segments.push_back(MixSwingSegment(initTimestamp, aMin, aMax, b));
        return static_cast<int>(points.size());
    }
    std::vector<MixSwingSegment> compress( std::vector<Point>& points) {
         std::vector<MixSwingSegment> segments;
         int currentIdx = 0;
         while (currentIdx < points.size() ) {
             int currentCeilIdx = createSegment(currentIdx, points, segments, 1);
             int currentFloorIdx = createSegment(currentIdx, points, segments, 2);
             if (currentCeilIdx > currentFloorIdx) {
                 if (!segments.empty()) {
                     segments.pop_back();
                 }
                 currentIdx = currentCeilIdx;
             }
             else if (currentCeilIdx < currentFloorIdx) {
                 if (segments.size() >= 2) {
                     segments.erase(segments.end() - 2);
                 }
                 currentIdx = currentFloorIdx;
             }
             else {
                 double firstValue = points[currentIdx].getValue();
                 if (std::round(firstValue / epsilon) == std::ceil(firstValue / epsilon)) {
                     if (!segments.empty()) {
                         segments.pop_back();
                     }
                 }
                 else {
                     if (segments.size() >= 2) {
                         segments.erase(segments.end() - 2);
                     }
                 }
                 currentIdx = currentFloorIdx;
             }
         }

         return segments;
    }

    std::vector<MixSwingSegment> mergePerB(std::vector<MixSwingSegment>& segments) {
        double aMinTemp = -std::numeric_limits<double>::max();
        double aMaxTemp = std::numeric_limits<double>::max();
        std::vector<MixSwingSegment> mergedSegments;
        std::vector<MixSwingSegment> recordSegments;
        auto compare = []( MixSwingSegment& lhs,  MixSwingSegment& rhs) {
            return lhs.getAMin() < rhs.getAMin();
            };
        std::sort(segments.begin(), segments.end(), compare);
        MixSwingSegment currentdSeg= MixSwingSegment(0,0,0,0);
        for (size_t i = 0; i < segments.size(); ++i) {
            currentdSeg = segments[i];
            if (currentdSeg.getAMin() <= aMaxTemp && currentdSeg.getAMax() >= aMinTemp) {
                recordSegments.push_back(segments[i]);
                aMinTemp = std::max(aMinTemp, currentdSeg.getAMin());
                aMaxTemp = std::min(aMaxTemp, currentdSeg.getAMax());
            }
            else {
                for ( auto& seg : recordSegments) {
                    mergedSegments.push_back(MixSwingSegment(seg.getInitTimestamp(), aMinTemp, aMaxTemp, seg.getB()));
                }
                aMinTemp = currentdSeg.getAMin();
                aMaxTemp = currentdSeg.getAMax();
                recordSegments.clear();
                recordSegments.push_back(currentdSeg);
            }
        }
        if (!recordSegments.empty()) {
            for ( auto& seg : recordSegments) {
                mergedSegments.push_back(MixSwingSegment(seg.getInitTimestamp(), aMinTemp, aMaxTemp, seg.getB()));
            }
            recordSegments.clear();
        }

        auto compare2 = []( MixSwingSegment& lhs,  MixSwingSegment& rhs) {
            return lhs.getInitTimestamp() < rhs.getInitTimestamp();
            };
        std::sort(mergedSegments.begin(), mergedSegments.end(), compare2);
        return mergedSegments;
    }

    
    void toByteArrayPerBSegments(std::vector<MixSwingSegment>& segments, std::stringstream& outStream) {
        std::map<double, std::vector<pair<int, int>>> input;
        std::vector<int> bstream;
        for (auto& segment : segments) {
            double a = handle_double(segment.getAMin(), segment.getAMax());
            int b = static_cast<int>(std::round(segment.getB() / epsilon));
            int t = static_cast<int>(segment.getInitTimestamp());
            if (input.find(a) == input.end()) {
                input[a] = std::vector<pair<int, int>>();
            }
            input[a].push_back(pair<int, int>(t, b));
        }
        VariableByteEncoder::write(static_cast<int>(input.size()), outStream);
        if (input.empty()) return;
        for (auto it = input.begin(); it != input.end(); ++it) {
            VariableDoublebyteEncoder::write(it->first, outStream);
            VariableByteEncoder::write(static_cast<int>(it->second.size()), outStream);
            int previousTS =0;
            for (const auto& list : it->second) {

                VariableByteEncoder::write((list.first - previousTS), outStream);
                previousTS = list.first;
                bstream.push_back(list.second);
            }
        }

        VariableByteEncoder::write(lastTimeStamp, outStream);

        huffmanencode(bstream, outStream);
    }
    vector<unsigned char> toByteArray(std::vector<MixSwingSegment>& segments) {
        std::stringstream outStream;
        FloatEncoder::write(static_cast<float>(epsilon), outStream);
        toByteArrayPerBSegments(segments,  outStream);

        std::string outData = outStream.str();
        vector<unsigned char> bytes;

        bytes.assign(outData.begin(), outData.end());
        
        return bytes;
    }

    void compress( std::vector<Point>& points, double error) {
        if (points.empty() || error <= 0) {
            throw std::runtime_error("Invalid input");
        }
        epsilon = error;
        lastTimeStamp = points[points.size() - 1].getTimestamp();
        Segments = compress(points);
        Segments = mergePerB(Segments);
    }
    
    std::vector<MixSwingSegment> readMergedPerBSegments(double epsilon, std::stringstream& inStream) {
        std::vector<MixSwingSegment> segments;
        int numA = VariableByteEncoder::read(inStream);
        if (numA == 0) return segments;
        int bcount = 0;
        for (int i = 0; i < numA; ++i) {
            double a = VariableDoublebyteEncoder::read(inStream);

            int numBort = VariableByteEncoder::read(inStream);
            bcount += numBort;
            int t = 0;
            for (int j = 0; j < numBort; ++j) {
                t += VariableByteEncoder::read(inStream);

                segments.push_back(MixSwingSegment(t, a, 0));
            }
        }

        this->lastTimeStamp = VariableByteEncoder::read(inStream);

        huffmandecode(inStream, bcount, segments, epsilon);
        return segments;
    }

    void readByteArray(stringstream& outputStream) {

        this->epsilon = FloatEncoder::read(outputStream);
        this->Segments = readMergedPerBSegments(epsilon, outputStream);

    }

    std::vector<Point> decompress(std::vector<MixSwingSegment> segments) {
        std::sort(segments.begin(), segments.end(), [](MixSwingSegment& a, MixSwingSegment& b) {
            return a.getInitTimestamp() < b.getInitTimestamp();
            });

        std::vector<Point> points;
        long currentTimeStamp = segments[0].getInitTimestamp();

        for (size_t i = 0; i < segments.size() - 1; i++) {
            while (currentTimeStamp < segments[i + 1].getInitTimestamp()) {
                points.push_back(Point(currentTimeStamp, segments[i].getA() * (currentTimeStamp - segments[i].getInitTimestamp()) + segments[i].getB()));
                currentTimeStamp++;
            }
        }

        while (currentTimeStamp <= lastTimeStamp) {
            points.push_back(Point(currentTimeStamp, segments[segments.size() - 1].getA() * (currentTimeStamp - segments[segments.size() - 1].getInitTimestamp()) + segments[segments.size() - 1].getB()));
            currentTimeStamp++;
        }

        return points;
    }
    std::vector<Point> decompress(vector<unsigned char>& bytes) {
        stringstream inStream;
        for (unsigned char c : bytes) {
            inStream << c;
        }
        readByteArray(inStream);
        return decompress(Segments);
    }
};

class HuffSwingSeg {
private:
    std::chrono::duration<double, std::milli> duration,duration1;
    long testHuffSwingSeg(std::vector<Point>& ts, double epsilon) {

        auto start = std::chrono::high_resolution_clock::now();
        MixSwing m = MixSwing();
        m.compress(ts, epsilon);
        //duration = std::chrono::high_resolution_clock::now() - start;
        std::vector<unsigned char> binary = m.toByteArray(m.Segments);
        long compressedSize = binary.size();
        std::vector<Point> tsDecompressed = m.decompress(binary);
        duration1 = std::chrono::high_resolution_clock::now() - start;
        long datacout = ts.size();
        double maxError = 0;

        for (int i = 0; i < datacout; i++) {
            if (maxError < abs(ts[i].getValue() - tsDecompressed[i].getValue())) {
                maxError = abs(ts[i].getValue() - tsDecompressed[i].getValue());
            }
        }

        if (maxError > 1.00001 * epsilon) {
            std::cout << "maxError - delta = " << maxError - epsilon << std::endl;
            std::cout << "delta: " << epsilon << std::endl;
        }
        else
            std::cout << "fit error bound" << std::endl;
        
        return compressedSize;
    }
public:
    void testHuffSwingSeg() {
        double epsilon = 0.005;

        std::string delimiter = ",";
        const std::vector<std::string> filenames = {
            "time_series/ETHUSD",
            "time_series/50words",
            "time_series/Adiac",
            "time_series/Beef",
            "time_series/CBF",
            "time_series/ChlorineConcentration",
            "time_series/CinC_ECG_torso",
            "time_series/Coffee",
            "time_series/Cricket_X",
            "time_series/Cricket_Y",
            "time_series/Cricket_Z",
            "time_series/DiatomSizeReduction",
            "time_series/ECG200",
            "time_series/ECGFiveDays",
            "time_series/FaceAll",
            "time_series/FaceFour",
            "time_series/FacesUCR",
            "time_series/FISH",
            "time_series/Gun_Point",
            "time_series/Haptics",
            "time_series/InlineSkate",
            "time_series/ItalyPowerDemand",
            "time_series/Lighting2",
            "time_series/Lighting7",
            "time_series/MALLAT",
            "time_series/MedicalImages",
            "time_series/MoteStrain",
            "time_series/OliveOil",
            "time_series/OSULeaf",
            "time_series/SonyAIBORobotSurface",
            "time_series/SonyAIBORobotSurfaceII",
            "time_series/StarLightCurves",
            "time_series/SwedishLeaf",
            "time_series/Symbols",
            "time_series/synthetic_control",
            "time_series/Trace",
            "time_series/TwoLeadECG",
            "time_series/Two_Patterns",
            "time_series/uWaveGestureLibrary_X",
            "time_series/uWaveGestureLibrary_Y",
            "time_series/uWaveGestureLibrary_Z",
            "time_series/wafer",
            "time_series/WordsSynonyms",
            "time_series/yoga",
        };

        for (const auto& filename : filenames) {
            std::cout << filename << std::endl;
            TimeSeries ts = TimeSeriesReader::getTimeSeries(fopen(filename.c_str(), "rb"), delimiter);
            long size = testHuffSwingSeg(ts.data, ts.range * epsilon);
            std::cout <<"Epsilon: " <<epsilon * 100 << "%, Compression ratio: " << (double)ts.size /size  << std::endl;
            //std::cout << "Time cost: " <<duration1.count()<< "ms"<< std::endl;
            std::cout << std::endl;
        }
    }
};