package main.util.Encoding;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

public class VariableDoublebyteEncoder {
    public static double handleDouble(double x, double y) {
        DoubleUnion u1 = new DoubleUnion(x);
        DoubleUnion u2 = new DoubleUnion(y);
        DoubleUnion result = new DoubleUnion(0);

        if (x == y) {
            return x;
        }

        if (x*y<0) {
            return result.value;
        }

        result.value = x;

        if ((u1.bits & 0x7FF0000000000000L) != (u2.bits & 0x7FF0000000000000L)) {
            if ((u1.bits & 0x7FF0000000000000L) > (u2.bits & 0x7FF0000000000000L)) {
                result.bits = u1.bits & 0xFFF0000000000000L;
            } else {
                result.bits = u2.bits & 0xFFF0000000000000L;
            }
        } else {
            long mask = 0x8000000000000000L;
            while ((u1.bits & mask) == (u2.bits & mask)) {
                mask >>= 1;
            }
            long z = u1.bits & mask|(~mask+1) ;
            result.bits = z;
        }
        return Double.longBitsToDouble(result.bits);
    }

    public static void write(double number, ByteArrayOutputStream outputStream) throws IOException {
        DoubleUnion value = new DoubleUnion(number);
        long val = value.bits;
        for (int i = 1; i <= 9; ++i) {
            if ((val & (0x7fffffffffffffffL >> (7 * i-1))) == 0) {
                outputStream.write((int) (fromHigh7Bits(i, val) | (1 << 7)));
                break;
            } else {
                outputStream.write((int) (fromHigh7Bits(i, val) & ((1 << 7) - 1)));
                if (i == 9) {
                    outputStream.write((int) (fromHigh7Bits(10, val) | (1 << 7)));
                }
            }
        }
    }

    private static long fromHigh7Bits(int i, long val) {
        return (val >> (64 - 7 * i)) & 0x7F;
    }

    public static double read(ByteArrayInputStream inputStream) throws IOException {
        DoubleUnion back = new DoubleUnion(0);
        for (int i = 0; i < 10; ++i) {
            int in = inputStream.read() & 0xFF;
            back.bits |= ((long) (in & 0x7F)) << (64 - 7 * (i + 1));
            if ((in & 0x80) != 0) {
                break;
            }
        }
        return Double.longBitsToDouble(back.bits);
    }
}