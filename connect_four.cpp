#include <iostream>
using namespace std;

#include "mcts.cpp"
#include "connect_four.h"

typedef int Move;

void ConnectFourState::do_move(Move move)
{
	assert(0 <= move && move < num_cols);
	assert(board[0][move] == player_markers[0]);
	check_invariant();

	int row = num_rows - 1;
	while (board[row][move] != player_markers[0]) row--;
	board[row][move] = player_markers[player_to_move];
	last_col = move;
	last_row = row;

	player_to_move = 3 - player_to_move;
}

template<typename RandomEngine>
void ConnectFourState::do_random_move(RandomEngine* engine)
{
	assert(has_moves());
	check_invariant();
	uniform_int_distribution<Move> moves(0, num_cols - 1);

	while (true) {
		auto move = moves(*engine);
		if (board[0][move] == player_markers[0]) {
			do_move(move);
			return;
		}
	}
}

bool ConnectFourState::has_moves() const
{
	check_invariant();

	char winner = get_winner();
	if (winner != player_markers[0]) {
		return false;
	}

	for (int col = 0; col < num_cols; ++col) {
		if (board[0][col] == player_markers[0]) {
			return true;
		}
	}
	return false;
}

vector<Move> ConnectFourState::get_moves() const
{
	check_invariant();

	vector<Move> moves;
	if (get_winner() != player_markers[0]) {
		return moves;
	}

	moves.reserve(num_cols);

	for (int col = 0; col < num_cols; ++col) {
		if (board[0][col] == player_markers[0]) {
			moves.push_back(col);
		}
	}
	return moves;
}

char ConnectFourState::get_winner() const
{
	if (last_col < 0) {
		return player_markers[0];
	}

	// We only need to check around the last piece played.
	auto piece = board[last_row][last_col];

	// X X X X
	int left = 0, right = 0;
	for (int col = last_col - 1; col >= 0 && board[last_row][col] == piece; --col) left++;
	for (int col = last_col + 1; col < num_cols && board[last_row][col] == piece; ++col) right++;
	if (left + 1 + right >= 4) {
		return piece;
	}

	// X
	// X
	// X
	// X
	int up = 0, down = 0;
	for (int row = last_row - 1; row >= 0 && board[row][last_col] == piece; --row) up++;
	for (int row = last_row + 1; row < num_rows && board[row][last_col] == piece; ++row) down++;
	if (up + 1 + down >= 4) {
		return piece;
	}

	// X
	//  X
	//   X
	//    X
	up = 0;
	down = 0;
	for (int row = last_row - 1, col = last_col - 1; row >= 0 && col >= 0 && board[row][col] == piece; --row, --col) up++;
	for (int row = last_row + 1, col = last_col + 1; row < num_rows && col < num_cols && board[row][col] == piece; ++row, ++col) down++;
	if (up + 1 + down >= 4) {
		return piece;
	}

	//    X
	//   X
	//  X
	// X
	up = 0;
	down = 0;
	for (int row = last_row + 1, col = last_col - 1; row < num_rows && col >= 0 && board[row][col] == piece; ++row, --col) up++;
	for (int row = last_row - 1, col = last_col + 1; row >= 0 && col < num_cols && board[row][col] == piece; --row, ++col) down++;
	if (up + 1 + down >= 4) {
		return piece;
	}

	return player_markers[0];
}

double ConnectFourState::get_result(int current_player_to_move) const
{
	assert( ! has_moves());
	check_invariant();

	auto winner = get_winner();
	if (winner == player_markers[0]) {
		return 0.5;
	}

	if (winner == player_markers[current_player_to_move]) {
		return 0.0;
	}
	else {
		return 1.0;
	}
}

void ConnectFourState::print(ostream& out) const
{
	out << endl;
	out << " ";
	for (int col = 0; col < num_cols - 1; ++col) {
		out << col << ' ';
	}
	out << num_cols - 1 << endl;
	for (int row = 0; row < num_rows; ++row) {
		out << "|";
		for (int col = 0; col < num_cols - 1; ++col) {
			out << board[row][col] << ' ';
		}
		out << board[row][num_cols - 1] << "|" << endl;
	}
	out << "+";
	for (int col = 0; col < num_cols - 1; ++col) {
		out << "--";
	}
	out << "-+" << endl;
	out << player_markers[player_to_move] << " to move " << endl << endl;
}







void main_program()
{
	using namespace std;

	bool human_player = true;

	ComputeOptions player1_options, player2_options;
	player1_options.max_iterations = 100000;
	player1_options.verbose = true;
	player2_options.max_iterations =  10000;
	player2_options.verbose = true;

	ConnectFourState state;
	while (state.has_moves()) {
		cout << endl << "State: " << state << endl;

		ConnectFourState::Move move = ConnectFourState::no_move;
		if (state.player_to_move == 1) {
			move = compute_move(state, player1_options);
			state.do_move(move);
		}
		else {
			if (human_player) {
				while (true) {
					cout << "Input your move: ";
					move = ConnectFourState::no_move;
					cin >> move;
					try {
						state.do_move(move);
						break;
					}
					catch (std::exception& ) {
						cout << "Invalid move." << endl;
					}
				}
			}
			else {
				move = compute_move(state, player2_options);
				state.do_move(move);
			}
		}
	}

	cout << endl << "Final state: " << state << endl;

	if (state.get_result(2) == 1.0) {
		cout << "Player 1 wins!" << endl;
	}
	else if (state.get_result(1) == 1.0) {
		cout << "Player 2 wins!" << endl;
	}
	else {
		cout << "Nobody wins!" << endl;
	}
}

int main()
{
	try {
		main_program();
	}
	catch (std::runtime_error& error) {
		std::cerr << "ERROR: " << error.what() << std::endl;
		return 1;
	}
}