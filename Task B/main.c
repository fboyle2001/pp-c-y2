#include"connect4.h"
int main(){
  FILE *infile,*outfile;

  board my_board=setup_board();
  infile=fopen("./boards/o_first.txt","r");
  read_in_file(infile,my_board);
  fclose(infile);

  write_out_file(stdout,my_board);

  while(current_winner(my_board)=='.') {
    struct move next_move = read_in_move(my_board);

    if (is_valid_move(next_move,my_board)) {
      char winner = is_winning_move(next_move, my_board);
      if(winner != '.') {
        printf("This move would win for %c\n", winner);
        //continue;
      }
      //printf("it's valid c: %d r: %d\n", next_move.column, next_move.row);
    play_move(next_move,my_board);
    write_out_file(stdout,my_board);
    }
  }

  //printf("%c\n", current_winner(my_board));
  outfile=fopen("final_board.txt","w");
  write_out_file(outfile,my_board);
  fclose(outfile);

  cleanup_board(my_board);
  return 0;
}
