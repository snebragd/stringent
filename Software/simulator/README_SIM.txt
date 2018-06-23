
command line:

#build simulator with
./build.sh

#run simulator with
echo 1 > irdata.txt ; ./sim 1000 | java PlotVisualizer

what is it? "1" is for plotting 1.svg from current dir. "sim" will simulate the plot and output timestamped stepper signals on stdout. PlotVisualizer reads from stdin and illustrates.

Useful output is the total plot time expected.
