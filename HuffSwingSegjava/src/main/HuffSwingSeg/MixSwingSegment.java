package main.HuffSwingSeg;

public class MixSwingSegment {

    private final long initTimestamp;
    private final double aMin;
    private final double aMax;
    private final double a;
    private double b;

    public MixSwingSegment(long initTimestamp, double a, double b) {
        this(initTimestamp, a, a, b);
    }

    public MixSwingSegment(long initTimestamp, double aMin, double aMax, double b) {
        this.initTimestamp = initTimestamp;
        this.aMin = aMin;
        this.aMax = aMax;
        this.a = (aMin + aMax) / 2;
        this.b = b;
    }

    public long getInitTimestamp() {
        return initTimestamp;
    }

    public double getAMin() {
        return aMin;
    }

    public double getAMax() {
        return aMax;
    }

    public double getA() {
        return a;
    }

    public double getB() {
        return b;
    }
    public void setB(double b) {
        this.b=b;
    }
}
