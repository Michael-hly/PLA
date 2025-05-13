package main.test.util;

import main.util.Point;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.zip.GZIPInputStream;

public class TimeSeriesReader {
    public static TimeSeries getTimeSeries(InputStream inputStream, String delimiter,boolean gzip) {
        ArrayList<Point> ts = new ArrayList<>();
        double max = Double.MIN_VALUE;
        double min = Double.MAX_VALUE;
        long timestamp = 0;

        try {
            InputStream gzipStream = new GZIPInputStream(inputStream);
            Reader decoder = new InputStreamReader(gzipStream, StandardCharsets.UTF_8);
            BufferedReader bufferedReader = new BufferedReader(decoder);

            String line;
            while ((line = bufferedReader.readLine()) != null) {
                line = line.replace(' ',',');
                String[] elements = line.split(delimiter);
                for (int i = 0; i < elements.length; i++) {
                    if (elements[i].isEmpty())
                        continue;
                    double value = Double.parseDouble(elements[i]);
                    ts.add(new Point(timestamp, value));
                    timestamp++;

                    max = Math.max(max, value);
                    min = Math.min(min, value);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return new TimeSeries(ts, max - min);
    }
}
