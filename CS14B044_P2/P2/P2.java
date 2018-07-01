import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.locks.ReentrantLock;

public class P2 {
	
	static int NUM_THREADS;
	static String IP_FNAME;
	static Util utilWrapper = new Util();
	static ArrayList<Long> inputList;
	static HashMap<Long,Boolean> processedPrimes = new HashMap<Long,Boolean>();
	static ReentrantLock storeLock = new ReentrantLock();
	
	static boolean isPrime(long n){
        if(n<=1){
            return false;
        }
        if(n%2==0){
            return false;
        }
        for(int i=3;i<=Math.ceil(Math.sqrt(n));i+=2){
            if(n%i==0){
                return false;
            }
        }
        return true;
    }
	
	static void storePrime(long n){
		try {
			storeLock.lock();
			utilWrapper.storePrime(n);
        } finally {
        	storeLock.unlock();
        }
		return;
	}
	
	static class primeThread extends Thread {
		int tID = -1;
		
		public primeThread(int tID) {
			this.tID = tID;
//			System.out.println("Creating Thread " +  tID);
		}
		
		public void run() {
//			System.out.println("Running Thread " +  tID);
			for(int i=tID;i<inputList.size();i+=NUM_THREADS){
				long n = inputList.get(i);
//				if(processedPrimes.containsKey(n)) {
//					if(processedPrimes.get(n)) {
//						storePrime(n);
//					}
//				}
//				else {
//					if(isPrime(n)) {
//						processedPrimes.put(n, true);
//						storePrime(n);
//					}
//					else {
//						processedPrimes.put(n, false);
//					}
//				}
				if(isPrime(n)) {
					storePrime(n);
				}
			}
//			System.out.println("Exiting Thread " +  tID);
		}
	}
	
    public static void main(String[] args) {
    	/* Read the number of threads to use */
        if(args.length != 1) {
        	System.out.println("Usage: java P2 <num_threads>");
        	System.exit(0);
        }
        IP_FNAME = "input.txt";
        NUM_THREADS = Integer.parseInt(args[0]);
        
		inputList = utilWrapper.readInput(); // read input and start timer.
		
		primeThread[] threads = new primeThread[NUM_THREADS];
		for (int tid=0; tid<NUM_THREADS; tid++){
 			threads[tid] = new primeThread(tid);
 			threads[tid].start();
       }
		
		for (int tid=0; tid<NUM_THREADS; tid++){
 			try {
				threads[tid].join();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
       }

		/* Finish */
		utilWrapper.printPrimes(); // end-timer, print output, and print time taken.
    }
 
}