package ex1;

import java.util.ArrayList;

public class ParallelGameOfLife implements GameOfLife {
	ArrayList<SectionController> threads; // the array of section controllers
											// working on the input matrix.

	public boolean[][][] invoke(boolean[][] initalField, int hSplit,
			int vSplit, int generations) {
		
		boolean [][][]tmp=invoke_aux(initalField, hSplit, vSplit, generations);		
		return tmp;
	}

	/*
	 * assign the threads there sector_controllers and assign the
	 * sector_controllers their borders
	 */
	private void assign_threads(boolean[][] initalField, int hSplit,
			int vSplit, int generations,
			ArrayList<SectionController> section_Controllers,
			ArrayList<Thread> threads) {
		int start_col = 0;
		int start_row = 0;
		int num_col = 0;
		int num_row = 0;
		double remaining_col = initalField.length;
		double remaining_row = initalField[0].length;
		for (int i = 0; i < vSplit; i++) {
			start_col = (int) (initalField.length - remaining_col);
			num_col = (int) (Math.ceil(remaining_col) / (vSplit - i));
			for (int j = 0; j < hSplit; j++) {
				start_row = (int) (initalField[0].length - remaining_row);
				num_row = (int) (Math.ceil(remaining_row) / (hSplit - j));
				SectionController tmp = new SectionController(start_row,
						start_col, num_row, num_col, generations);
				section_Controllers.add(tmp);
				threads.add(new Thread(tmp));
				remaining_row -= num_row;
			}
			remaining_col -= num_col;
			remaining_row = initalField[0].length;
		}
	}
	
	private boolean[][][] invoke_aux(boolean[][] initalField, int hSplit,
			int vSplit, int generations) {

		ArrayList<SectionController> section_Controllers = new ArrayList<SectionController>();
		ArrayList<Thread> threads = new ArrayList<Thread>();
		SectionController.input[0] = initalField.clone();
		SectionController.input[1] = new boolean[initalField.length][initalField[0].length];
		assign_threads(initalField, hSplit, vSplit, generations,
				section_Controllers, threads);
		for (SectionController i : section_Controllers) {
			for (SectionController j : section_Controllers) {
				// if i and j are neighbours
				if ((i.starting_row+i.num_of_rows==j.starting_row || 
						j.starting_row+j.num_of_rows==i.starting_row || 
						j.starting_row==i.starting_row) &&
						(i.starting_col+i.num_of_cols==j.starting_col || 
						j.starting_col+j.num_of_cols==i.starting_col || 
						j.starting_col==i.starting_col)){
					i.neighbours.add(j);
				}
				
			}
		}
		for (Thread thread : threads) {
			thread.start();
		}
		for (Thread thread : threads) {
			try {
				thread.join();
			} catch (InterruptedException e) {
				throw (new SectionController.SynchronizationFailed("master thread can't join"));
			}
		}
		/*try {
			Ex1.printArray(SectionController.input[0], "P_output.txt");
			Ex1.printArray(SectionController.input[1], "P_output1.txt");
		} catch (Exception e) {
			System.err.println("Failed to write the output to file");
			e.printStackTrace();
			System.exit(-1);
		}*/
		return SectionController.input.clone();
	}
}
