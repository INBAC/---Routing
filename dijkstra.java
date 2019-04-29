import java.util.*;
import java.io.*;
import java.net.InetAddress;
import java.net.DatagramSocket;

public class dijkstra
{
	static int[][] graph;
    private static final int NO_PARENT = -1;
    static String outputFile;
    static int pathCheck;
    
	private dijkstra(String fileName, String fileName2)
	{
		outputFile = fileName2;
		pathCheck = 0;
		System.out.println(outputFile);
		int numberOfVertices = 0;
		int source, destination, cost;
		boolean[] duplicateCheck = new boolean[1024];	
		ArrayList<String> inputs = new ArrayList<>();
		try
		{
			String temp = "";
			BufferedReader read = new BufferedReader(new FileReader(fileName));
			while((temp = read.readLine()) != null)
			{
				source = Integer.parseInt(temp.substring(12,  15));
				destination = Integer.parseInt(temp.substring(28, 31));
				if(duplicateCheck[source] == false)
					numberOfVertices++;
				 if(duplicateCheck[destination] == false)
					 numberOfVertices++;
				duplicateCheck[source] = true;
				duplicateCheck[destination] = true;
				inputs.add(temp);
			}
		}
		catch(IOException e)
		{
			System.out.println("Error reading file");
			System.exit(1);
		}
		graph = new int[numberOfVertices][numberOfVertices];
		for(int i = 0; i < graph.length; i++)
			for(int j = 0; j < graph[i].length; j++)
				if(i != j)
					graph[i][j] = -1;
		for(int i = 0; i < inputs.size(); i++)
		{
			source = Integer.parseInt(inputs.get(i).substring(12, 15));
			destination = Integer.parseInt(inputs.get(i).substring(28, 31));
			cost = Integer.parseInt(inputs.get(i).substring(32));
			source = (source % 10) - 1;
			destination = (destination % 10) - 1;
			graph[source][destination] = cost;
		}
	}
	
    private static void dijkstra(int[][] adjacencyMatrix, int startVertex) 
    { 
        int nVertices = adjacencyMatrix[0].length; 
        int[] shortestDistances = new int[nVertices]; 
        boolean[] added = new boolean[nVertices]; 
        for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++) 
        { 
            shortestDistances[vertexIndex] = Integer.MAX_VALUE; 
            added[vertexIndex] = false; 
        } 
        shortestDistances[startVertex] = 0; 
        int[] parents = new int[nVertices]; 
        parents[startVertex] = NO_PARENT; 
        for (int i = 1; i < nVertices; i++) 
        { 
            int nearestVertex = -1; 
            int shortestDistance = Integer.MAX_VALUE; 
            for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++) 
            { 
                if (!added[vertexIndex] && shortestDistances[vertexIndex] < shortestDistance)  
                { 
                    nearestVertex = vertexIndex; 
                    shortestDistance = shortestDistances[vertexIndex]; 
                } 
            } 
            added[nearestVertex] = true; 
            for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++)  
            { 
                int edgeDistance = adjacencyMatrix[nearestVertex][vertexIndex]; 
                  
                if (edgeDistance > 0  && ((shortestDistance + edgeDistance) <  shortestDistances[vertexIndex]))  
                { 
                    parents[vertexIndex] = nearestVertex; 
                    shortestDistances[vertexIndex] = shortestDistance +  
                                                       edgeDistance; 
                } 
            } 
        } 
        printSolution(startVertex, shortestDistances, parents); 
    } 
 
    private static void printSolution(int startVertex, int[] distances, int[] parents) 
    { 
        int nVertices = distances.length; 
        for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++)  
        { 
            if (vertexIndex != startVertex)  
            { 
                System.out.print((startVertex + 1) + " -> "); 
                System.out.print((vertexIndex + 1) + "\t\t"); 
                writeToFile(vertexIndex, 0);
                System.out.print(distances[vertexIndex] + "\t\t"); 
                pathCheck = 0;
                printPath(vertexIndex, parents); 
                System.out.println();
            } 
        } 
    } 
  
    private static void printPath(int currentVertex, int[] parents) 
    { 
        if (currentVertex == NO_PARENT) 
        { 
            return; 
        } 
        printPath(parents[currentVertex], parents); 
        System.out.print((currentVertex + 1) + " "); 
        pathCheck++;
        if(pathCheck == 2)
        	writeToFile(currentVertex, 1);
    }
    
    private static void writeToFile(int input, int type)
    {
    	String text = "220.149.244.21" + Integer.toString(++input);
    	if(type == 0)
    		text += " ";
    	else
    		text += "\n";
    	File file = new File(outputFile);
	    try
	    {
	        FileWriter fw = new FileWriter(file, true);
	        fw.write(text);
	        fw.close();
	    } 
	    catch (IOException e)
	    {
	        e.printStackTrace();
	    }

    }
    
    private static String getIpAddress()
    {
    	String ip = "";
    	try(final DatagramSocket socket = new DatagramSocket())
    	{
    	  socket.connect(InetAddress.getByName("8.8.8.8"), 10002);
    	  ip = socket.getLocalAddress().getHostAddress();
    	}
    	catch(Exception e)
    	{
    		System.exit(1);
    	}
    	return ip;
    }
	
	public static void main(String[] args)
	{
		if(args.length != 2)
		{
			System.out.println("Please Enter 'InputFile' 'OutputFile'");
			System.exit(1);
		}
		String ip = getIpAddress();
		int sourceVertex = Integer.parseInt(ip.substring(14));
		new dijkstra(args[0], args[1]);
		dijkstra(graph, sourceVertex - 1); 
	}
}
