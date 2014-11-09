/**
 * 
 */
package ex1;

import java.util.ArrayList;

/**
 * @author Raz
 * 
 */
public class SectionController implements Runnable {
	private Integer input_gen;
	private Integer result_gen;
	public int target_gen;
	boolean[][] input;
	boolean[][] result; // it's initialized as [cols][rows]
	public int starting_row;
	public int starting_col;
	public int num_of_rows;
	public int num_of_cols;
	public static ArrayList<SectionController> neighbours; //includes yourself

	public SectionController(boolean[][] input, int starting_row,
			int starting_col, int num_of_rows, int num_of_cols, int target_gen) {
		input_gen = 0;
		result_gen = 0;
		this.target_gen = target_gen;
		this.input = input;
		this.starting_col = starting_col;
		this.starting_row = starting_row;
		this.num_of_cols = num_of_cols;
		this.num_of_rows = num_of_rows;
		boolean[][] result = new boolean[num_of_cols][num_of_rows];
		for (int i = 0; i < num_of_cols; i++) {
			for (int j = 0; j < num_of_rows; j++) {
				result[i][j] = false;
			}
		}
	}

	@Override
	public void run() {
		while(input_gen < target_gen) {
			nextGen();
			increaseResGen();
			for (SectionController t : neighbours) {
				synchronized(t) {
					while(t.getResGen() != result_gen) {
						try {
							t.wait();
						} catch (InterruptedException e) {
							System.out.println("InterruptedException while waiting for res_gen to update"); // TODO - throw an
							// exception!
						}
					}
				}
			}
			for (int i = 0; i < num_of_cols; i++) {
				for (int j = 0; j < num_of_rows; j++) {
					input[starting_col+i][starting_row+j] = result[i][j];
				}
			}
			increaseInputGen();
		}
		// TODO Auto-generated method stub

	}

	private SectionController convertPointToThread(int col, int row) {
		// check if out of bounds
		if (col > input.length || row > input[0].length) {
			return null;
		}
		for (SectionController t : neighbours) {
			if (col >= t.starting_col && col < t.starting_col + t.num_of_cols
					&& row >= t.starting_row
					&& row < t.starting_row + t.num_of_rows) {
				return t;
			}
		}
		return null; // should not reach here. should throw an exception, but
						// we're lazy for now.
		// TODO throw an exception?
	}

	public int getInputGen() {
		synchronized (input_gen) {
			return input_gen;
		}
	}

	public int getResGen() {
		synchronized (result_gen) {
			return result_gen;
		}
	}

	private void increaseInputGen() {
		synchronized (input_gen) {
			input_gen++;
			notifyAll();
		}
	}

	private void increaseResGen() {
		synchronized (result_gen) {
			result_gen++;
			notifyAll();
		}
	}

	private int numNeighbors(int col, int row) {
		int counter = (input[col][row] ? -1 : 0);
		for (int i = col - 1; i <= col + 1; i++) {
			if (i < 0 || i >= input.length) {
				continue;
			}
			for (int j = row - 1; j <= row + 1; j++) {
				if (j < 0 || j >= input[0].length) {
					continue;
				}
				// need to check what section controller owns the square
				SectionController neighborOwner = convertPointToThread(i, j);
				synchronized (neighborOwner) {
					// if the square is not in the current gen, we can't work on
					// it. wait until the other thread is done.
					while (neighborOwner.getInputGen() != input_gen) {
						try {
							neighborOwner.wait();
						} catch (InterruptedException e) {
							System.out.println("panic!"); // TODO - throw an
															// exception!!!
						}
					}
					counter += (input[i][j] ? 1 : 0);
				}
			}
		}
		return counter;
	}

	private void nextGen() {
		for (int i = starting_col; i < starting_col + num_of_cols; i++) {
			for (int j = starting_row; j < starting_row + num_of_rows; j++) {
				int numNeighbors = numNeighbors(i, j);
				result[i - starting_col][j - starting_row] = false;
				if (numNeighbors == 3 || (input[i][j] && numNeighbors == 2)) {
					result[i - starting_col][j - starting_row] = true;
				}
			}
		}
	}

}
