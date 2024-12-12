package main.util.Encoding;

public class DoubleUnion {
    public double value;
    public long bits;

    public DoubleUnion(double value) {
        this.value = value;
        this.bits = Double.doubleToLongBits(value);
    }
}