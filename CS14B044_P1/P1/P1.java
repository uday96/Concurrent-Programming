import java.io.*;
import java.util.HashMap;
import java.util.Stack;

public class P1 {
	
	static int NUM_THREADS;
	static String IP_FNAME;
	static int NUM_NODES;
	static HashMap<Integer, Integer> ParentIDs = new HashMap<Integer, Integer>();
	static Stack<Integer> LeafIDs = new Stack<Integer>();
	static Node[] Nodes;
	
	static class leafThread extends Thread {
		int leafID = -1;
		
		public leafThread() {
			synchronized (LeafIDs) {
				if(!LeafIDs.isEmpty()) {
					leafID = LeafIDs.pop();
				}
			}
//			System.out.println("Creating Thread " +  leafID);
		}
		
		public void run() {
//			System.out.println("Running Thread " +  leafID);
			while(leafID != -1) {
				int currID = leafID;
				while(ParentIDs.containsKey(currID)) {
					int parentID = ParentIDs.get(currID);
					synchronized (Nodes[parentID-1]) {
						if(!Nodes[parentID-1].visited) {
							Nodes[parentID-1].weight += Nodes[currID-1].weight;
							Nodes[parentID-1].visited = true;
							break;
						}
						else {
							Nodes[parentID-1].weight += Nodes[currID-1].weight;
							
							int leftWeight = Nodes[parentID-1].leftChildID == 0 ? 0 : Nodes[Nodes[parentID-1].leftChildID - 1].weight;
							int rightWeight = Nodes[parentID-1].rightChildID == 0 ? 0 : Nodes[Nodes[parentID-1].rightChildID - 1].weight;
							
							if(leftWeight == rightWeight) {
								System.out.println(parentID+" 0");
							}
							else if(leftWeight > rightWeight) {
								System.out.println(parentID+" 1");
							}
							else {
								System.out.println(parentID+" -1");
							}
							
							currID = parentID;
						}
					}
				}
				synchronized (LeafIDs) {
					if(!LeafIDs.isEmpty()) {
						leafID = LeafIDs.pop();
					}
					else {
						break;
					}
				}
				
			}
//			System.out.println("Thread " +  leafID + " exiting.");
		}
	}
	
    public static void main(String[] args) {
        if(args.length != 2) {
        	System.out.println("Usage: java P1 <input_file> <num_threads>");
        	System.exit(0);
        }
        IP_FNAME = args[0];
        NUM_THREADS = Integer.parseInt(args[1]);
// 		System.out.println("fname : "+IP_FNAME);
// 		System.out.println("#threads : "+NUM_THREADS);
 		
 		String line = null;
 		try {
 			FileReader fileReader = new FileReader(IP_FNAME);
 			BufferedReader bufferedReader = new BufferedReader(fileReader);
 			
 			boolean init = true;
            while((line = bufferedReader.readLine()) != null) {
            	if(init) {
            		NUM_NODES = Integer.parseInt(line);
            		init = false;
            		Nodes = new Node[NUM_NODES];
            		continue;
            	}
//                System.out.println(line);
                String parts[]= line.split(" ");
//                for (String part: parts){
//                   System.out.println(part);
//                }
                int myID = Integer.parseInt(parts[0]);
                int weight = Integer.parseInt(parts[1]);
                int leftChildID = Integer.parseInt(parts[2]);
                int rightChildID = Integer.parseInt(parts[3]);
                
                if(leftChildID == 0 && rightChildID == 0) {
                	LeafIDs.push(myID);
                	System.out.println(myID+" "+0);
                	Nodes[myID-1] = new Node(weight, leftChildID, rightChildID,true);
                }
                else if (leftChildID == 0 && rightChildID != 0) {
                	ParentIDs.put(rightChildID, myID);
                	Nodes[myID-1] = new Node(weight, leftChildID, rightChildID,true);
				}
                else if (leftChildID != 0 && rightChildID == 0) {
                	ParentIDs.put(leftChildID, myID);
                	Nodes[myID-1] = new Node(weight, leftChildID, rightChildID,true);
				}
                else {
                	ParentIDs.put(leftChildID, myID);
                	ParentIDs.put(rightChildID, myID);
                	Nodes[myID-1] = new Node(weight, leftChildID, rightChildID,false);
                }
            }   

            bufferedReader.close();         
        }
        catch(FileNotFoundException ex) {
            System.out.println("Unable to open file '" +IP_FNAME + "'");                
        }
        catch(IOException ex) {
            System.out.println("Error reading file '"+ IP_FNAME + "'");
        }
 		
 		for (int tid=0; tid<NUM_THREADS; tid++){
 			leafThread T = new leafThread();
 	 		T.start();
       }
 		
    }
 
}

class Node{
	int weight;
	int leftChildID;
	int rightChildID;
	boolean visited;
	public Node(int weight,int leftChildID, int rightChildID, boolean visited) {
		this.weight = weight;
		this.leftChildID = leftChildID;
		this.rightChildID = rightChildID;
		this.visited = visited;
	}
}