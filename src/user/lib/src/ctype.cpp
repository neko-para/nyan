#include <ctype.h>

extern "C" {

int isalnum(int ch) {
    return isalpha(ch) || isdigit(ch);
}

int isalpha(int ch) {
    return islower(ch) || isupper(ch);
}

int isascii(int ch) {
    return (unsigned)ch <= 0x7F;
}

int isblank(int ch) {
    return ch == ' ' || ch == '\t';
}

int iscntrl(int ch) {
    return (unsigned)ch < 0x20 || ch == 0x7F;
}

int isdigit(int ch) {
    return ch >= '0' && ch <= '9';
}

int isgraph(int ch) {
    return ch > ' ' && ch <= '~';
}

int islower(int ch) {
    return ch >= 'a' && ch <= 'z';
}

int isprint(int ch) {
    return ch >= ' ' && ch <= '~';
}

int ispunct(int ch) {
    return isgraph(ch) && !isalnum(ch);
}

int isspace(int ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
}

int isupper(int ch) {
    return ch >= 'A' && ch <= 'Z';
}

int isxdigit(int ch) {
    return isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

int toascii(int ch) {
    return ch & 0x7F;
}

int tolower(int ch) {
    return isupper(ch) ? ch + ('a' - 'A') : ch;
}

int toupper(int ch) {
    return islower(ch) ? ch - ('a' - 'A') : ch;
}
}
