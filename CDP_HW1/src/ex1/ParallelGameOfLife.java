package ex1;

import java.util.ArrayList;

public class ParallelGameOfLife implements GameOfLife {
	ArrayList<SectionController> threads; // the array of section controllers
											// working on the input matrix.

	public boolean[][][] invoke(boolean[][] initalField, int hSplit,
			int vSplit, int generations) {
		boolean[][][] result = new boolean[2][][];
		result[0] = invoke_aux(initalField, hSplit, vSplit, generations - 1);
		result[1] = invoke_aux(result[0], hSplit, vSplit, 1);
		return result;
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
		for (int i = 0; i <= vSplit; i++) {
			start_col = (int) (initalField.length - remaining_col);
			num_col = (int) (Math.ceil(remaining_col) / (vSplit + 1 - i));
			for (int j = 0; j <= hSplit; j++) {
				start_row = (int) (initalField[0].length - remaining_row);
				num_row = (int) (Math.ceil(remaining_row) / (hSplit + 1 - j));
				SectionController tmp = new SectionController(start_row,
						start_col, num_row, num_col, generations,initalField);
				section_Controllers.add(tmp);
				threads.add(new Thread(tmp));
				remaining_row -= num_row;
			}
			remaining_col -= num_col;
			remaining_row = initalField[0].length;
		}
	}

	private boolean[][] invoke_aux(boolean[][] initalField, int hSplit,
			int vSplit, int generations) {

		ArrayList<SectionController> section_Controllers = new ArrayList<SectionController>();
		ArrayList<Thread> threads = new ArrayList<Thread>();
		SectionController.input = initalField.clone();
		assign_threads(initalField, hSplit, vSplit, generations,
				section_Controllers, threads);
		SectionController.neighbours = section_Controllers;
		for (Thread thread : threads) {
			thread.start();
		}
		for (Thread thread : threads) {
			try {
				thread.join();
			} catch (InterruptedException e) {
				// TODO handle this exception
			}
		}
		return SectionController.input.clone();
	}

}
