* Compile and use the program "Generator" to create sample input test-cases for P3.

* Compilation: javac Generator.java

* An example usage:

	java Generator -s 100 -min-val 20 -max-val 30 -seed 338 > input.txt
  
  This command generates a 100 x 100 2D array input with random double values in the range [20.0, 30.0[.
  The output is prepended with "100", the size of the input array. (338 is a random seed.)
  This output is printed to STDOUT (which we have redirected to input.txt in the above example).
  Note that the "seed" value is optional; you can provide any integer as seed.

* The general syntax is:
	
	java Generator -s <size> -min-val <min> -max-val <max> [-seed <seed>]
  
  <size> and <seed> should be valid Java integer constants;
  <min-val> and <max-val> should be valid doubles.

EOF.
