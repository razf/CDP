package ex1;

import java.util.ArrayList;

public class ParallelGameOfLife implements GameOfLife {
	ArrayList<SectionController> threads; // the array of section controllers working on the input matrix. 

	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit,
			int generations) {
		boolean [][][] result = new boolean [2][][];
		result[0]=invoke_aux(initalField, hSplit, vSplit,
			generations-1);
		result[1]=invoke_aux(result[0], hSplit, vSplit,1);
		return result;
	}
	private boolean [][]invoke_aux(boolean[][] initalField, int hSplit, int vSplit,
			int generations){
		int start_col=0;
		int start_row=0;
		int num_col=0;
		int num_row=0;
		ArrayList<SectionController> section_Controllers=new ArrayList<SectionController>();
		ArrayList<Thread> threads=new ArrayList<Thread>();
		SectionController.input=initalField;
		for (int i = 0; i <= initalField.length/vSplit; i++) {
			start_col=i*vSplit;
			// num_col is min(vsplit,largest number that fit in the remaining array)
			num_col = i==initalField.length/vSplit ? initalField.length-start_col:vSplit;
			for (int j = 0; j < initalField[0].length/hSplit; j++) {
				start_row=j*hSplit;
				//num_row is min(hsplit,largest number that fit in the remaining array)
				num_row= j==initalField[0].length/hSplit ? initalField[0].length-start_row : hSplit;
				SectionController tmp=new SectionController( start_row, start_col,
						num_row, num_col, generations);
				section_Controllers.add(tmp);
				threads.add(new Thread(tmp));
			}
		}
		SectionController.neighbours=section_Controllers;
		for (Thread thread : threads) {
			thread.start();
		}
		for (Thread thread : threads) {
			try {
				thread.join();
			}catch (InterruptedException e){
				// TODO handle this exception
			}
		}
		return SectionController.input.clone();
	}
	
	

}
