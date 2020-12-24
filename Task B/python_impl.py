# Task B

rows, columns = 6, 8

board = [[0 for x in range(columns)] for y in range(rows)]

board = [[ 0,   0,   0,   0,   0,   0,   0,   0],
         [ 0,   0,   0,   0,   0,   0,   0,   0],
         [ 0,   1 ,  0,   0,   0,   0,   0,   0],
         [ 1,  -1,   0,   0,   0,   0,   0,   0],
         [-1,   1,   0,   0,   0,   0,  -1,   1],
         [-1,   1,   0,   0,   0,   0,   1,  -1]]

from pprint import pprint

players = { "x": 1, "o": -1, ".": 0 }
rev_players = { 1: "x", -1: "o", 0: "." }

def get_col(board, col):
    col_data = []

    for row in board:
        col_data.append(row[col])

    return col_data

def set_col(board, col, col_data):
    for i, element in enumerate(col_data):
        board[i][col] = element

    return board

def shift_to_end(array):
    filledSpaces = 0
    moved = True

    while moved:
        moved = False

        for i in range(len(array) - 1 - filledSpaces, -1 , -1):
            if array[i] != 0:
                shift_index = len(array) - 1 - filledSpaces
                array[shift_index] = array[i]

                if shift_index != i:
                    array[i] = 0

                moved = True
                filledSpaces += 1

    return array

def apply_gravity(board):
    for col in range(columns):
        current_col = get_col(board, col)
        shifted_col = shift_to_end(current_col)
        board = set_col(board, col, shifted_col)

    return board

def rotate_row(row):
    copy = row[0]

    for i in range(len(row)):
        temp = row[(i + 1) % len(row)]
        row[(i + 1) % len(row)] = copy
        copy = temp

    return row

def make_move(board, player, drop_column, row_to_rotate):
    # full board
    column = get_col(board, drop_column)

    if column[0] != 0:
        return board

    column[0] = players[player]
    board = set_col(board, drop_column, column)
    board = apply_gravity(board)
    print_board(board)

    if row_to_rotate != -1:
        row = board[row_to_rotate]
        row = rotate_row(row)
        board[row_to_rotate] = row
        board = apply_gravity(board)

    return board

def print_board(board):
    mapped_board = [[rev_players[pos] for pos in row] for row in board]
    pprint(mapped_board)

# 1 = LEFT DIAGONAL, -1 = RIGHT DIAGONAL
def get_right_diagonal(board, row, column):
    diagonal = []
    cr, cc = row, column

    while cr != -1:
        #print(cr, cc)
        diagonal.append(board[cr][cc])
        cr -= 1
        cc += 1
        cc %= columns;

    return diagonal

def get_left_diagonal(board, row, column):
    diagonal = []
    cr, cc = row, column

    while cr != -1:
        #print(cr, cc)
        diagonal.append(board[cr][cc])
        cr -= 1
        cc -= 1
        cc %= columns;

    return diagonal

print_board(board)

for i in range(columns):
    print((5, i), "left", get_left_diagonal(board, 5, i))

for i in range(columns):
    print((5, i), "right", get_right_diagonal(board, 5, i))

#pprint(get_left_diagonal(board, 5, 1))
# # columns are left to right, rows are top to bottm
# board = make_move(board, "o", 3 - 1, rows - 4)
# print_board(board)
