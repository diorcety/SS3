#ifndef PRINT_H__
#define PRINT_H__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>   // for sprintf
#include <string.h>  // for strlen

class Print {
public:
    // --- pure virtual: subclasses implement this ---
    virtual size_t write(uint8_t c) = 0;

    // write buffer (default: loop over write(uint8_t))
    virtual size_t write(const uint8_t *buf, size_t size) {
        size_t n = 0;
        while (size--) n += write(*buf++);
        return n;
    }

    size_t write(const char *str) {
        if (!str) return 0;
        return write((const uint8_t *)str, strlen(str));
    }

    // --- print() overloads ---
    size_t print(const char *str)         { return write(str); }
    size_t print(char c)                  { return write((uint8_t)c); }
    size_t print(unsigned char v, int base = 10) { return printNumber(v, base); }
    size_t print(int v,           int base = 10) { return printInt(v, base); }
    size_t print(unsigned int v,  int base = 10) { return printNumber(v, base); }
    size_t print(long v,          int base = 10) { return printInt(v, base); }
    size_t print(unsigned long v, int base = 10) { return printNumber(v, base); }
    size_t print(double v,        int digits = 2){ return printFloat(v, digits); }

    // --- println() overloads ---
    size_t println()                             { return write("\r\n"); }
    size_t println(const char *str)              { return print(str)   + println(); }
    size_t println(char c)                       { return print(c)     + println(); }
    size_t println(unsigned char v, int base=10) { return print(v, base) + println(); }
    size_t println(int v,           int base=10) { return print(v, base) + println(); }
    size_t println(unsigned int v,  int base=10) { return print(v, base) + println(); }
    size_t println(long v,          int base=10) { return print(v, base) + println(); }
    size_t println(unsigned long v, int base=10) { return print(v, base) + println(); }
    size_t println(double v,        int digits=2){ return print(v, digits) + println(); }

private:
    size_t printInt(long v, int base) {
        if (v < 0 && base == 10) {
            write('-');
            return 1 + printNumber((unsigned long)(-v), base);
        }
        return printNumber((unsigned long)v, base);
    }

    size_t printNumber(unsigned long v, int base) {
        char buf[8 * sizeof(long) + 1];
        char *p = &buf[sizeof(buf) - 1];
        *p = '\0';
        if (base < 2) base = 10;
        do {
            char d = v % base;
            *--p = d < 10 ? '0' + d : 'A' + d - 10;
            v /= base;
        } while (v);
        return write(p);
    }

    size_t printFloat(double v, int digits) {
        char buf[32];
        // simple snprintf-based approach; replace if no libc
        snprintf(buf, sizeof(buf), "%.*f", digits, v);
        return write(buf);
    }
};
#endif //PRINT_H__