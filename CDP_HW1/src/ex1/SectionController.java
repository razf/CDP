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
	public class EndOfFunction extends RuntimeException {
		public EndOfFunction(String message) {
			super(message);
		}

		// eclipse suggested to put this variable here
		private static final long serialVersionUID = 1L;
	}

	public class SynchronizationFailed extends RuntimeException {
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

		public synchronized boolean GetData(int gen) {
			if (gen == generation) {
				return curr_gen_data;
			} else if (gen == generation - 1) {
				return prev_gen_data;
			} else {
				System.out.println("panic!!!");// TODO throw an exception
				return false;// shouldn't reach this
			}
		}
	}

	private Integer input_gen;
	private Integer result_gen;
	public int target_gen;
	Tile[][] result; // it's initialized as [cols][rows]
	public int starting_row;
	public int starting_col;
	public int num_of_rows;
	public int num_of_cols;
	public boolean changed;// if current iteration changed generation
	public boolean done;// all tiles are in the target generation.
	public ArrayList<SectionController> neighbours; // includes yourself
	//FIXME add a static board
	
	
	public SectionController(int starting_row, int starting_col,
			int num_of_rows, int num_of_cols, int target_gen, boolean[][] input) {
		input_gen = 0;
		result_gen = 0;
		this.target_gen = target_gen;
		this.starting_col = starting_col;
		this.starting_row = starting_row;
		this.num_of_cols = num_of_cols;
		this.num_of_rows = num_of_rows;
		this.result = new Tile[num_of_cols][num_of_rows];
		for (int i = 0; i < num_of_cols; i++) {
			for (int j = 0; j < num_of_rows; j++) {
				// we initialize the tiles so the prev gen is -1 (meaningless),
				// and the current gen is 0.
				result[i][j] = new Tile(0, false, input[i + starting_col][j
						+ starting_row]);
			}
		}
		changed = false;
		done = (target_gen == 0);// finish immediately if the target generation
									// is 0.
	}

	@Override
	public void run() {
		while (!done) {
			done = true; // it will be made false in the next iteration if not
							// done.
			changed = false;// it will be made true if any tile advanced a
							// generation.
			// iterate on the section
			for (int i = 0; i < num_of_cols; i++) {
				for (int j = 0; j < num_of_rows; j++) {
					if (result[i][j].generation < target_gen) {
						done = false;
						int adj = numNeighbors(i, j);
						if (adj >= 0) {// adj will be negative if can't be
										// calculated
							synchronized (result[i][j]) {
								result[i][j].prev_gen_data = result[i][j].curr_gen_data;
								// calculate the current generation of this tile
								result[i][j].curr_gen_data = false;
								if (adj == 3
										|| (result[i][j].prev_gen_data && adj == 2)) {
									result[i][j].curr_gen_data = true;
								}
								// update generation and changed.
								result[i][j].generation += 1;
							}
							changed = true;
							if (i == 0 || i == num_of_cols - 1 || j == 0
									|| j == num_of_rows - 1) {
								// TODO notifyAllRelatedNeighbors(i,j); (all
								// neighbors related to this)
							}
						}
					}
				}
				// if we can't calculate any of our tiles
				if (!changed) {
					synchronized (this) {
						try{
							this.wait();
						} catch (InterruptedException e) {
							//FIXME
						}
					}
				}
			}
		}
		//TODO - write last generation and the one before it to the global input matrix.
	}
	
	//returns null if index out of bounds.
	//TODO
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
		throw new EndOfFunction(
				"convertPointToThread didnt find the point a thread owner");
	}

	private int numNeighbors(int col, int row) {
		int counter = (result[col][row].prev_gen_data ? -1 : 0);
		
		for (int i = col - 1; i <= col + 1; i++) {
			for (int j = row - 1; j <= row + 1; j++) {
				// need to check what section controller owns the square. null if it's outside the board limits.
				SectionController neighborOwner = convertPointToThread(i+starting_col, j+starting_row);//convert by index on the board
				if(neighborOwner == null) {
					continue; // always false.
				}
				try {
					if(neighborOwner.getTileData(i,j,result[i][j].generation-1)) {
						counter++;
					}
				} catch (Exception e) {
					//FIXME, also create an exception
				}
			}
		}
		return counter;
	}

	private boolean getTileData(int col, int row, int generation) {
		// TODO Auto-generated method stub
		return false;
	}

	public void notifyAllRelatedNeighbors(int col, int row) {
		// TODO 
	}
}
