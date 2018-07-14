import javax.swing.*;
import javax.imageio.*;
import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.lang.reflect.*;
import java.util.*;


public class PlotVisualizer implements Runnable
{
    private static JLabel leftLabel = new JLabel(" Left=123456789"); 
    private static JLabel rightLabel = new JLabel(" Right=123456789");
    private static JLabel servoLabel = new JLabel(" Servo=0");
    private static TestPane gfxPanel;
    
    private static Thread uiUpdater;


    private static double spoolCirc=94.2f;
    private static double stepsPerRotation=4075.7728395f;
    private static double stepsPerMM = stepsPerRotation/spoolCirc;

    public static double[] getPos() {
	int currentLeftSteps;
	int currentRightSteps;

	//	System.out.println(centerX + ","+centerY);
	//	System.out.print(".");
	

	synchronized(stepMonitor) {
	    if(leftStepper == null) {
		return new double[2];
	    }
	    currentLeftSteps = leftStepper.getStep();
	    currentRightSteps = rightStepper.getStep();
	}
	
	double currentLeft  = currentLeftSteps / stepsPerMM;
	double currentRight = currentRightSteps / stepsPerMM;
	double tmp1 = (currentRight*currentRight-disparity*disparity-currentLeft*currentLeft);
	double tmp2 = (-2*currentLeft*disparity);
	double a = Math.acos(tmp1/tmp2);    
	double centerX = currentLeft*Math.cos(a);
	double centerY = currentLeft*Math.sin(a);
	
	double ret[] = new double[2];
	ret[0] = centerX;
	ret[1] = centerY;

	return ret;
    }

    private static void showUI() {
	JFrame frame = new JFrame("PlotVisualizer") {};
	frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	frame.setLayout(new BorderLayout());
	
	gfxPanel = new TestPane();
	frame.add(gfxPanel, BorderLayout.CENTER);

	JPanel pan = new JPanel();
	pan.setLayout(new FlowLayout());
	pan.add(leftLabel);	
	pan.add(rightLabel);   
	pan.add(servoLabel);	
	frame.add(pan, BorderLayout.NORTH);
	frame.pack();
	frame.setLocationRelativeTo(null);
	frame.setVisible(true);

	uiUpdater = new Thread(new PlotVisualizer());
	uiUpdater.start();
    }


    public static void main(String args[]) throws IOException, InterruptedException {
	SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    showUI();
		}
	    });

	readLoop();
    }


    private void updateUI() {
	synchronized(stepMonitor) {
	    if (leftStepper != null) { 
		leftLabel.setText(" Left="+leftStepper.getStep());
		rightLabel.setText(" Right="+rightStepper.getStep());
		servoLabel.setText(" Servo="+getServoPos());
	    }
	}

	gfxPanel.repaint();
    }

    public void run() {
	while(true) {
	    try {
		Thread.sleep(25);	
		SwingUtilities.invokeAndWait(new Runnable() {
			public void run() {
			    updateUI();
			}
		    });
	    }
	    catch(InterruptedException e) {
		break;
	    }
	    catch(InvocationTargetException e) {
		e.printStackTrace();
		break;
	    }
	}	
    }

    private static Object stepMonitor = new Object();
    private static Stepper leftStepper;
    private static Stepper rightStepper;
    private static int disparity;
    private static int servoPos;
    private static BufferedImage img;

    public static BufferedImage getImage() {		
	return img;
    }

    public static int getDisparity() {
	return disparity;
    }

    public static int getServoPos() {
	return servoPos;
    }

    private static void readLoop() throws IOException, InterruptedException {
	BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
	br.readLine(); //skip first header [disparity, currentLeftSteps, currentRightSteps, centerX, centerY]
	String state = br.readLine(); //next line is the data
	StringTokenizer tok = new StringTokenizer(state, ", ");
	disparity = Integer.parseInt(tok.nextToken());
	int currLeft = Integer.parseInt(tok.nextToken());
	int currRight = Integer.parseInt(tok.nextToken());
	double cX = Double.parseDouble(tok.nextToken());
	double cY = Double.parseDouble(tok.nextToken());

	img = new BufferedImage(disparity, 2*disparity, BufferedImage.TYPE_INT_RGB);

	//byte leftPins[4] = {9,8,7,6};
	//byte rightPins[4] = {5,4,3,2};

	synchronized(stepMonitor) {
	    //	    leftStepper = new Stepper(currLeft, 9,8,7,6);
	    //rightStepper = new Stepper(currRight, 5,4,3,2);
	    leftStepper = new Stepper(currLeft, 6,7,8,9);
	    rightStepper = new Stepper(currRight, 2,3,4,5);
	}

	while(true) {
	    String str = br.readLine();
	    if(str == null) {
		try {
		    // retrieve image
		    File outputfile = new File("/tmp/plot.png");
		    ImageIO.write(getImage(), "png", outputfile);
		} catch (IOException e) { }
		Thread.sleep(2000);
		System.exit(0); //dirty way out
	    }
	    tok = new StringTokenizer(str, " =");
	    long timestamp = Long.parseLong(tok.nextToken());
	    int pin = Integer.parseInt(tok.nextToken());
	    int val = Integer.parseInt(tok.nextToken());

	    if(pin == 0) {
		//servo
		servoPos = val; 
	    }
	    else {
		leftStepper.setPin(pin,val > 0);
		rightStepper.setPin(pin,val > 0);
	    }
	    if(servoPos == 20) {
		double pos[] = PlotVisualizer.getPos();	 		
		pos[0] = Math.min(Math.max(0.0,pos[0]), disparity-2);
		pos[1] = Math.min(Math.max(0.0,pos[1]), disparity*2-2);
		for(int y=-1;y<2;y++) for(int x=-1;x<2;x++) 
					  img.setRGB((int)pos[0]+x,(int)pos[1]+y,0xffffffff);
	    }
	}
    }
}

class Stepper {
    private int step;
    private int bitState;
    private int pins[] = new int[4];
    //byte stepSequence[8] = {B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001};    

    Stepper(int initialStep, int p1, int p2, int p3, int p4) {
	step = initialStep;
	pins[0] = p1;
	pins[1] = p2;
	pins[2] = p3;
	pins[3] = p4;
    }

    boolean setPin(int pin, boolean set) {
	int prevPos = step & 0x7;

	for(int i=0 ; i<4 ; i++) {
	    if(pins[i] == pin) {
		int orBit = 0b1000 >> i;
		int orMask = 0b1111 ^ orBit;
		bitState = (bitState & orMask);
		if(set) {
		    bitState |= orBit;
		}
		int pos=0;
		switch(bitState) {
		case 0b1000:
		    pos = 0;
		    break;
		case 0b1100:
		    pos = 1;
		    break;
		case 0b0100:
		    pos = 2;
		    break;
		case 0b0110:
		    pos = 3;
		    break;
		case 0b0010:
		    pos = 4;
		    break;
		case 0b0011:
		    pos = 5;
		    break;
		case 0b0001:
		    pos = 6;
		    break;
		case 0b1001:
		    pos = 7;
		    break;
		case 0b0000:
		    //zero, undefined position, guess last
		    pos = prevPos;		    
		    break;
		default:
		    System.out.println("Stepper state messed up: "+bitState);
		    System.exit(1);
		}
		
		int diff = pos-prevPos;
		if(diff == 1) {
		    step++;
		}
		else if(diff == -1) {
		    step--;
		}
		else if(pos == 0 && prevPos == 7) {
		    step++;
		}
		else if(pos == 7 && prevPos == 0) {
		    step--;
		}
		else if(diff == 0) {
		    //no step
		}
		else {
		    System.out.print("*"); //just output some crap for messed up stepping
		}		

		return true;
	    }
	}
	return true;
    } 

    int getStep() {
	return step;
    }
}

 class TestPane extends JPanel {
     public Dimension getPreferredSize() {
	 return new Dimension(500, 1000);
     }
     
     protected void paintComponent(Graphics g) {
	 super.paintComponent(g);
	 //	 g.setColor(Color.GREEN);
	 //g.fillRect(0, 0, WIDTH, HEIGHT);

	 Dimension dim = getSize();
	 BufferedImage img = PlotVisualizer.getImage();
	 if(img != null) {
	     g.drawImage(img,0,0,dim.width,dim.height,Color.BLACK,null);
	 }

	 g.setColor(Color.RED);
	 double pos[] = PlotVisualizer.getPos();	 
	 Dimension size = getSize();
	 double mmPerPixelX = PlotVisualizer.getDisparity()/(double)size.width;
	 double mmPerPixelY = 2*PlotVisualizer.getDisparity()/(double)size.height;
	 
	 g.drawLine(0,0,         (int)(pos[0]/mmPerPixelX),(int)(pos[1]/mmPerPixelY));
	 g.drawLine(size.width,0,(int)(pos[0]/mmPerPixelX),(int)(pos[1]/mmPerPixelY));

	 g.setColor(Color.GREEN);	 
	 int pSize = (int)((1.0-(PlotVisualizer.getServoPos()-20)/(110-20.0))*10+2);
	 g.fillOval((int)(pos[0]/mmPerPixelX)-pSize/2,(int)(pos[1]/mmPerPixelY)-pSize/2, pSize, pSize);	 
     }
 }



