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
	public static class TileGenerationIsTooEarly extends Exception {
		private static final long serialVersionUID = 1L;

		public TileGenerationIsTooEarly(String message) {
			super(message);
		}
	}

	public static class EndOfFunction extends RuntimeException {
		public EndOfFunction(String message) {
			super(message);
		}

		// eclipse suggested to put this variable here
		private static final long serialVersionUID = 1L;
	}

	public static class SynchronizationFailed extends RuntimeException {
		public SynchronizationFailed(String message) {
			super(message);
		}

		// eclipse suggested to put this variable here
		private static final long serialVersionUID = 1L;
	}

	public class Tile {
		public int generation;
		public boolean curr_gen_data;
		public boolean prev_gen_data;

		public Tile(int gen, boolean prev_gen_data, boolean curr_gen_data) {
			generation = gen;
			this.curr_gen_data = curr_gen_data;
			this.prev_gen_data = prev_gen_data;
		}

		public synchronized boolean getDataAux(int gen)
				throws TileGenerationIsTooEarly {
			if (gen == generation) {
				return curr_gen_data;
			} else if (gen == generation - 1) {
				return prev_gen_data;
			} else {
				throw new TileGenerationIsTooEarly(
						((Integer) generation).toString());

			}
		}
	}

	public int target_gen;
	Tile[][] result; // it's initialized as [cols][rows]
	public int starting_row;
	public int starting_col;
	public int num_of_rows;
	public int num_of_cols;
	public int count;
	public boolean wasNotified;
	public boolean changed;// if current iteration changed generation
	public boolean done;// all tiles are in the target generation.
	public ArrayList<SectionController> neighbours; // includes yourself
	/*
	 * input[0] is the starting input matrix at the start of the run input[1] is
	 * the last generation matrix at the end of the run input[0] is the
	 * generation matrix before the last (generation-1)
	 */
	public static boolean[][][] input = new boolean[2][][];

	public SectionController(int starting_row, int starting_col,
			int num_of_rows, int num_of_cols, int target_gen) {
		this.target_gen = target_gen;
		this.starting_col = starting_col;
		this.starting_row = starting_row;
		this.num_of_cols = num_of_cols;
		this.num_of_rows = num_of_rows;
		this.result = new Tile[num_of_cols][num_of_rows];
		this.neighbours = new ArrayList<SectionController>();
		this.wasNotified = false;
		for (int i = 0; i < num_of_cols; i++) {
			for (int j = 0; j < num_of_rows; j++) {
				// we initialize the tiles so the prev gen is -1 (meaningless),
				// and the current gen is 0.
				result[i][j] = new Tile(0, input[0][i + starting_col][j
						+ starting_row], input[0][i + starting_col][j
						+ starting_row]);
			}
		}
		changed = false;
		done = (target_gen == 0);// finish immediately if the target generation
									// is 0.
	}
	private void paintCell(int col,int row,int adj){
			result[col][row].prev_gen_data = result[col][row].curr_gen_data;
			// calculate the current generation of this tile
			result[col][row].curr_gen_data = false;
			if (adj == 3
					|| (result[col][row].prev_gen_data && adj == 2)) {
				result[col][row].curr_gen_data = true;
			}
			// update generation and changed.
			result[col][row].generation += 1;
	}
	@Override
	public void run() {
		while (!done) {
			done = true; // it will be made false in the next iteration if not
							// done.
			changed = false;// it will be made true if any tile advanced a
							// generation.
			ArrayList<SectionController> notifyedNeighbors = new ArrayList<SectionController>();
			// iterate on the section
			for (int i = 0; i < num_of_cols; i++) {
				for (int j = 0; j < num_of_rows; j++) {
					if (result[i][j].generation < target_gen) {
						done = false;
						int adj = numNeighbors(i, j);
						if (adj >= 0) {// adj will be -1 if can't be
										// calculated
							if (i==0||j==0||i==num_of_cols-1||j==num_of_rows-1){
								synchronized (result[i][j]) {
									paintCell(i, j, adj);
								}
							}
							else paintCell(i, j, adj);
							changed = true;
							if (i == 0 || i == num_of_cols - 1 || j == 0
									|| j == num_of_rows - 1) {
								notifyAllRelatedNeighbors(i, j,
										notifyedNeighbors);// (all
								// neighbors related to this)
							}
						}
					}
				}
			}
			// notify all neighbours
			for (SectionController sectionController : notifyedNeighbors) {
				synchronized (sectionController) {
					sectionController.wasNotified = true;
					sectionController.count++;
					sectionController.notify();
				}

			}
			// if we can't calculate any of our tiles
			if (!changed && !done) {
				synchronized (this) {
					while (!wasNotified) {
						try {
							this.wait();
						} catch (InterruptedException e) {
							throw new SynchronizationFailed(
									"there is no more work to do and the wait has failed");
						}

					}
					wasNotified = false;
				}
			}
		}
		for (int i = 0; i < result.length; i++) {
			for (int j = 0; j < result[0].length; j++) {
				input[0][i + starting_col][j + starting_row] = result[i][j].prev_gen_data;
				input[1][i + starting_col][j + starting_row] = result[i][j].curr_gen_data;
			}

		}

	}

	private SectionController convertPointToThread(int col, int row) {
		// check if out of bounds
		if (col >= input[0].length || row >= input[0][0].length || col < 0
				|| row < 0) {
			return null;
		}
		for (SectionController t : neighbours) {
			if (col >= t.starting_col && col < t.starting_col + t.num_of_cols
					&& row >= t.starting_row
					&& row < t.starting_row + t.num_of_rows) {
				return t;
			}
		}
		System.out.println(col + " " + row);
		throw new EndOfFunction(
				"convertPointToThread didnt find the point a thread owner");
	}

	private int numNeighbors(int col, int row) {
		int counter = (result[col][row].curr_gen_data ? -1 : 0);
		for (int i = col - 1; i <= col + 1; i++) {
			for (int j = row - 1; j <= row + 1; j++) {
				// need to check what section controller owns the square. null
				// if it's outside the board limits.
				SectionController neighborOwner = convertPointToThread(i
						+ starting_col, j + starting_row);// convert by index on
															// the board
				if (neighborOwner == null) {
					continue; // always false.
				}
				try {
					if (neighborOwner.getTileData(i + starting_col, j
							+ starting_row, result[col][row].generation,
							neighborOwner)) {
						counter++;
					}
				} catch (TileGenerationIsTooEarly e) {
					return -1;
				}
			}
		}
		return counter;
	}

	// col and row are location on the "global" matrix
	private boolean getTileData(int col, int row, int generation,
			SectionController sender) throws TileGenerationIsTooEarly {
		// the current sectionController own the Tile
		if (sender.starting_col == this.starting_col
				&& sender.starting_row == this.starting_row) {
			return result[col - starting_col][row - starting_row]
					.getDataAux(generation);
		}
		synchronized (result[col - starting_col][row - starting_row]) {
			return result[col - starting_col][row - starting_row]
					.getDataAux(generation);
		}
	}

	// calculate by the column and row which of the Neighbouring thread should
	// be notified about the new data;
	public void notifyAllRelatedNeighbors(int col, int row,
			ArrayList<SectionController> notifyedNeighbors) {
		int global_col = col + starting_col;
		int global_row = row + starting_row;
		for (SectionController tmp : neighbours) {
			if (tmp == this)
				continue;
			// if tmp is next to the tile (col,row)

			if (((global_row + 1 < tmp.starting_row + num_of_rows && global_row + 1 >= tmp.starting_row)
					|| (global_row - 1 < tmp.starting_row + num_of_rows && global_row - 1 >= tmp.starting_row) || (global_row < tmp.starting_row
					+ num_of_rows && global_row >= tmp.starting_row))
					&& ((global_col + 1 < tmp.starting_col + num_of_cols && global_col + 1 >= tmp.starting_col)
							|| (global_col - 1 < tmp.starting_col + num_of_cols && global_col - 1 >= tmp.starting_col) || (global_col < tmp.starting_col
							+ num_of_cols && global_col >= tmp.starting_col)))

				if (!notifyedNeighbors.contains(tmp)) {
					notifyedNeighbors.add(tmp);
				}

		}
	}
}
