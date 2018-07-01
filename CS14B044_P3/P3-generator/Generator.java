import java.util.Random;

public class Generator {
	public static void main(String[] args) {
		if (args.length != 6 && args.length != 8) {
			System.out.println("usage: java Generator -s <size> -min-val <min> -max-val <max> [-seed <seed>]");
			System.exit(0);
		}
		int size = 0;
		double minVal = 0.0;
		double maxVal = 0.0;
		long seed = 179;
		try {
			size = Integer.parseInt(args[1]);
			minVal = Double.parseDouble(args[3]);
			maxVal = Double.parseDouble(args[5]);
			if (minVal > maxVal) {
				System.out.println("Invalid range [" + minVal + ", " + maxVal + "[.");
				System.exit(0);
			}
			if (args.length == 8) {
				seed = Long.parseLong(args[7]);
			}
		} catch (NumberFormatException e) {
			System.out.println("usage: java Generator -s <size> -min-val <min> -max-val <max>");
			System.exit(0);
		}
		System.err.println("Creating a random input in stdout of size " + size + "x" + size + " with values in range ["
				+ minVal + ", " + maxVal + "[.");
		Random rand = new Random(seed);
		System.out.println(size);
		for (int i = 0; i < size; i++) {
			if (i%50 == 0 || i == 0) {
				System.err.println("Row: " + i);
			}
			System.out.printf("%.3f", getNext(rand, minVal, maxVal));
			for (int j = 1; j < size; j++) {
				System.out.printf(" %.3f", getNext(rand, minVal, maxVal));
			}
			System.out.println();
		}
	}

	private static double getNext(Random rand, double minVal, double maxVal) {
		Double d = rand.nextDouble();
		if (d == 0.0) {
			d = 0.001;
		}
		d = minVal + ((maxVal - minVal) * d);
		return d;
	}
}
