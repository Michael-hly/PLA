package main.util;

import java.util.List;

public class ErrorCalculator {
    public static double calculateMAE(List<Point> expected, List<Point> actual) {
        double sum = 0;
        int n = expected.size();

        for (int i = 0; i < n; i++) {
            double error = expected.get(i).getValue() - actual.get(i).getValue();
            sum += Math.abs(error);
        }

        return sum / n;
    }

    public static double calculateRMSE(List<Point> expected, List<Point> actual) {
        double sumSquares = 0;
        int n = expected.size();

        for (int i = 0; i < n; i++) {
            double error = expected.get(i).getValue() - actual.get(i).getValue();
            sumSquares += Math.pow(error, 2);
        }

        return Math.sqrt(sumSquares / n);
    }
}