package main.test.TestPLA;


import main.HuffSwingSeg.MixSwing;

import main.test.util.TimeSeries;
import main.util.Point;
import main.test.util.TimeSeriesReader;
import org.junit.jupiter.api.Test;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.time.Duration;
import java.time.Instant;
import java.util.List;

import static main.util.ErrorCalculator.calculateMAE;
import static main.util.ErrorCalculator.calculateRMSE;
import static org.junit.jupiter.api.Assertions.assertEquals;

public class TestPLA {
    private Duration duration,duration1;

    private long HuffSwingSeg(List<Point> ts, double epsilon) throws IOException {
        duration = Duration.ZERO;duration1 = Duration.ZERO;
        Instant start = Instant.now();
        MixSwing ms = new MixSwing(ts, epsilon);
        duration = Duration.between(start, Instant.now());
        byte[] binary = ms.toByteArray();
        long compressedSize = binary.length;
        ms = new MixSwing(binary);
        List<Point> tsDecompressed = ms.decompress();
        duration1 = Duration.between(start, Instant.now());
        int idx = 0;
        double MAE=calculateMAE(tsDecompressed,ts);
        double RMSE=calculateRMSE(tsDecompressed,ts);
        //System.out.println("M "+MAE+" "+" "+RMSE);

        for (Point expected : tsDecompressed) {
            Point actual = ts.get(idx);
            if (expected.getTimestamp() != actual.getTimestamp()) continue;
            idx++;
            assertEquals(actual.getValue(), expected.getValue(), 1.1 * epsilon, "Value did not match for timestamp " + actual.getTimestamp());

        }
        assertEquals(idx, ts.size());

        return compressedSize;
    }


    private void run(String[] filenames, double epsilon) throws IOException {
        for (String filename : filenames) {
            System.out.println(filename);
            String delimiter = ",";
            InputStream inputStream = new FileInputStream("src/main/resources"+filename);
            TimeSeries ts = TimeSeriesReader.getTimeSeries(inputStream, delimiter, true);

            System.out.printf("Epsilon: %.2f%%\tCompression Ratio: %.3f\tExecution Time: %dms\n", epsilon * 100, (double)ts.size /HuffSwingSeg(ts.data, ts.range * epsilon),duration1.toMillis());

            System.out.println();
        }
    }


    @Test
    public void TestCRAndTime() throws IOException {
        double epsilon = 0.005;

        String[] filenames = {
                "/50words.csv.gz",
                "/Adiac.csv.gz",
                "/Beef.csv.gz",
                "/CBF.csv.gz",
                "/ChlorineConcentration.csv.gz",
                "/CinC_ECG_torso.gz",
                "/Coffee.csv.gz",
                "/Cricket_X.csv.gz",
                "/Cricket_Y.csv.gz",
                "/Cricket_Z.csv.gz",
                "/DiatomSizeReduction.csv.gz",
                "/ECG200.csv.gz",
                "/ECGFiveDays.csv.gz",
                "/FaceAll.csv.gz",
                "/FaceFour.csv.gz",
                "/FacesUCR.csv.gz",
                "/FISH.csv.gz",
                "/Gun_Point.csv.gz",
                "/Haptics.csv.gz",
                "/InlineSkate.csv.gz",
                "/ItalyPowerDemand.csv.gz",
                "/Lighting2.csv.gz",
                "/Lighting7.csv.gz",
                "/MALLAT.gz",
                "/MedicalImages.csv.gz",
                "/MoteStrain.csv.gz",
                "/OliveOil.csv.gz",
                "/OSULeaf.csv.gz",
                "/SonyAIBORobotSurface.csv.gz",
                "/SonyAIBORobotSurfaceII.csv.gz",
                "/StarLightCurves.gz",
                "/SwedishLeaf.csv.gz",
                "/Symbols.csv.gz",
                "/synthetic_control.csv.gz",
                "/Trace.csv.gz",
                "/TwoLeadECG.csv.gz",
                "/Two_Patterns.csv.gz",
                "/uWaveGestureLibrary_X.csv.gz",
                "/uWaveGestureLibrary_Y.csv.gz",
                "/uWaveGestureLibrary_Z.csv.gz",
                "/Wafer.csv.gz",
                "/WordsSynonyms.csv.gz",
                "/yoga.gz"
                };

        run(filenames, epsilon);
    }
}
