int main(void) {
  if (ss.cursor_pos) {

    if (ss.command_line[ss.cursor_pos]) {

      // Mid-line, need to pull everything over

      uint8_t bs;

      ss.cursor_pos--;

      console_write("\b\0337"); // Backspace and save

      console_write("\033[K"); // Erase line

      // Shift remainder left

      for (bs = ss.cursor_pos; ss.command_line[bs]; bs++)

      {

        ss.command_line[bs] = ss.command_line[bs + 1];
      }

      console_write(&ss.command_line[ss.cursor_pos]);

      console_write("\0338");

    }

    else

    {

      // At end of line, just delete

      ss.cursor_pos--;

      ss.command_line[ss.cursor_pos] = 0;

      console_write("\b \b");
    }
  }
}
