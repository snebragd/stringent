import java.io.*;
import java.util.*;

public class DXFToPlotter 
{


    public static void main(String args[]) throws IOException{
	
	FileReader fr = new FileReader(args[0]);
	BufferedReader br = new BufferedReader(fr); 
	String s;
	boolean inPoly=false;
	boolean first = false;

	int targetWidth = Integer.parseInt(args[1]);

	int skip = 0;
	int nPoints = -1;

	List<Float> xArray = new ArrayList<Float>();
	List<Float> yArray = new ArrayList<Float>();
	List<Float> penArray = new ArrayList<Float>();
	float minX = 10000;
	float maxX = -10000;
	float minY = 10000;
	float maxY = -10000;

	while((s = br.readLine()) != null) { 
	    if(skip>0) {
		skip--;
		continue;
	    }
	    
	    if(inPoly) {
		if (nPoints < 0) {
		    nPoints = new Integer(s).intValue();
		    skip = 3;
		    continue;
		}
		else if(nPoints > 0) {
		    nPoints--;
		    float x = new Float(s).floatValue();
		    xArray.add(new Float(s));
		    s = br.readLine(); //skip
		    s = br.readLine(); //y
 		    float y = new Float(s).floatValue();
		    yArray.add(new Float(s));
		    //		    System.out.println(x+" , "+y);
		    skip = 3;
		    penArray.add(new Float(first ? 0.0 : 1.0));

		    if(x < minX) {minX = x;}
		    if(x > maxX) {maxX = x;}
		    if(y < minY) {minY = y;}
		    if(y > maxY) {maxY = y;}

		    first = false;
		    continue;
		}
		else {
		    nPoints = -1;
		    inPoly = false;
		    continue;
		}
	    }

	    if(s.equals("AcDbPolyline")) {
		inPoly = true;
		first = true;
		skip = 1;
		//		System.out.println("polyline");
		continue;
	    }
	} 
	fr.close(); 
	
	//	System.out.println("X = {"+minX+","+maxX+"}");
	//System.out.println("Y = {"+minY+","+maxY+"}");

	float scaleFactor = targetWidth / (maxX-minX);

	System.out.println("\n\n#define nStates "+(penArray.size()+1));

	System.out.print("\nPROGMEM prog_int16_t xArray[nStates] = {");
	for(Float x : xArray) {
	    System.out.print((int)((x-minX)*scaleFactor-targetWidth/2) + ",");
	}
	System.out.print("0}");

	System.out.print("\nPROGMEM prog_int16_t yArray[nStates] = {");
	for(Float y : yArray) {
	    System.out.print((int)((y-minY-(maxY-minY)/2)*scaleFactor) + ",");
	}
	System.out.print("0}");

	System.out.print("\nPROGMEM prog_int16_t penArray[nStates] = {");
	for(Float p : penArray) {
	    System.out.print((int)p.floatValue() + ",");
	}
	System.out.print("0}");

	

    }

}