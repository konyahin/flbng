#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct screen_size {
    unsigned short width;
    unsigned short height;
};

struct data {
    char **data;
    size_t lines;
    size_t max_width;
};

// global variable for terminal state restoring
struct termios orig_term;

void
restoreTerminal() {
    // show cursor
    puts("\e[?25h");
    if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &orig_term) < 0)
        err(1, "can't restore terminal state");
}

void
prepareTerminal() {
    if (tcgetattr(STDOUT_FILENO, &orig_term) < 0)
        err(1, "can't get terminal attributes");

    atexit(restoreTerminal);

    // havely inspired by https://github.com/antirez/kilo/blob/master/kilo.c
    struct termios raw = orig_term;
    raw.c_iflag &= 
        (unsigned) ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
    raw.c_oflag &= (unsigned) ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= 
        (unsigned) ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw) < 0)
        err(1, "can't set terminal attributes");

    // hide cursor
    puts("\e[?25l");
    // clear screen
    puts("\x1b[2J");
}

void
processInput() {
    // read from /dev/tty, because standart input
    // will be redirected
    int input = open("/dev/tty", O_RDONLY);
    char c = '\0';
    while (1) {
        ssize_t read_s = read(input, &c, 1);
        if (read_s < 0)
            err(1, "can't read from tty");
        if (read_s == 0)
            continue;
        break;
    }
}

struct screen_size
getScreenSize() {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return (struct screen_size) {
        .width = size.ws_col,
        .height = size.ws_row,
    };
}

struct data
getInput(struct screen_size s) {
    struct data text;
    text.data = calloc(sizeof(char*), s.height);
    text.lines = 0;
    text.max_width = 0;

    while (1) {
        char *line = NULL;
        ssize_t len = 0;
        size_t buf_len = 0;

        if ((len = getline(&line, &buf_len, stdin)) < 0)
            break;

        // we can reach EOF here, if there is no delimiter on the last line
        // if there is delimiter - we should remove it
        if (!feof(stdin)) {
            len -= 1;
            line[len] = '\0';
        }

        if ((size_t) len > text.max_width) {
            text.max_width = len;
            if (len > s.width)
                errx(1, "too long line for this terminal, should be shorter than %d", s.width);
        }
        text.data[text.lines] = line;
        text.lines += 1;
        if (text.lines > s.height)
            errx(1, "too much lines for this terminal, should be less than %d", s.height);
    }

    if (ferror(stdin))
        err(1, "can't read line from stdin");

    if (text.lines == 0)
        errx(1, "empty input");

    return text;
}

int
main(void) {
    // TODO: -h for help

    struct screen_size s = getScreenSize();
    struct data text = getInput(s);

    size_t start_position = (s.height - text.lines) / 2;
    int margin = (int) ((s.width - text.max_width) / 2);

    prepareTerminal();

    for (size_t i = 0; i < start_position; i++)
        printf("\r\n");

    for (size_t i = 0; i < text.lines; i++) {
        if (margin)
            printf("%*s", margin, " ");
        printf("%s", text.data[i]);
        if (i != text.lines - 1)
            printf("\r\n");
    }

    for (size_t i = start_position + text.lines; i < s.height; i++)
        printf("\r\n");

    fflush(stdout);

    processInput();

    for (size_t i = 0; i < text.lines; i++)
        free(text.data[i]);
    free(text.data);

    // clear screen
    puts("\x1b[2J");

    return 0;
}
