// C library headers
#include <ncurses.h>

int main(int argc, char* argv[])
{
  WINDOW* pWindow = initscr();
  box(pWindow, ACS_VLINE, ACS_HLINE);
  getch();
  endwin();
  return 0;
}
