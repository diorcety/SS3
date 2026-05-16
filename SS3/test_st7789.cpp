#include <gtest/gtest.h>
#include <climits>
#include <cstring>
#include "util.h"

extern "C" TESTABLE int snprint_int(char *buf, size_t size, int value, char pad, int width);
extern "C" TESTABLE int print_temperature(char *s, size_t n, int temperature, bool unit, bool offset);
extern "C" int temperature_unit;


// ============================================================
// snprint_int – edge cases
// ============================================================

class SnprintIntTest : public ::testing::Test {
protected:
    char buf[64];
};

// --- Zero ---
TEST_F(SnprintIntTest, ZeroNoWidth) {
    int ret = snprint_int(buf, sizeof(buf), 0, ' ', 0);
    EXPECT_STREQ("0", buf);
    EXPECT_EQ(1, ret);
}

TEST_F(SnprintIntTest, ZeroWithWidth3) {
    int ret = snprint_int(buf, sizeof(buf), 0, '0', 3);
    EXPECT_STREQ("000", buf);
    EXPECT_EQ(3, ret);
}

// --- Positive values ---
TEST_F(SnprintIntTest, PositiveSingleDigit) {
    int ret = snprint_int(buf, sizeof(buf), 7, ' ', 0);
    EXPECT_STREQ("7", buf);
    EXPECT_EQ(1, ret);
}

TEST_F(SnprintIntTest, PositiveMultiDigitExactWidth) {
    int ret = snprint_int(buf, sizeof(buf), 42, '0', 2);
    EXPECT_STREQ("42", buf);
    EXPECT_EQ(2, ret);
}

TEST_F(SnprintIntTest, PositiveWidthLargerThanValue) {
    int ret = snprint_int(buf, sizeof(buf), 5, '0', 4);
    EXPECT_STREQ("0005", buf);
    EXPECT_EQ(4, ret);
}

TEST_F(SnprintIntTest, PositiveWidthSmallerThanValue) {
    // value "12345" is 5 digits; width=3 → would_write=5
    int ret = snprint_int(buf, sizeof(buf), 12345, '0', 3);
    EXPECT_STREQ("12345", buf);
    EXPECT_EQ(5, ret);
}

// --- Negative values ---
TEST_F(SnprintIntTest, NegativeSingleDigit) {
    int ret = snprint_int(buf, sizeof(buf), -3, ' ', 0);
    EXPECT_STREQ("-3", buf);
    EXPECT_EQ(2, ret);
}

TEST_F(SnprintIntTest, NegativeWithPadding) {
    // "-3" is 2 chars; width=3 → "-03"
    int ret = snprint_int(buf, sizeof(buf), -3, '0', 3);
    EXPECT_STREQ("-03", buf);
    EXPECT_EQ(3, ret);
}

TEST_F(SnprintIntTest, NegativeWidthSmallerThanValue) {
    int ret = snprint_int(buf, sizeof(buf), -999, '0', 3);
    EXPECT_STREQ("-999", buf);
    EXPECT_EQ(4, ret);
}

// --- size == 0 (dry-run / returns length only) ---
TEST_F(SnprintIntTest, SizeZeroReturnsWouldWrite) {
    int ret = snprint_int(nullptr, 0, 42, '0', 3);
    EXPECT_EQ(3, ret);  // MAX(2, 3) = 3
}

TEST_F(SnprintIntTest, SizeZeroLargeValue) {
    int ret = snprint_int(nullptr, 0, 123456, '0', 3);
    EXPECT_EQ(6, ret);
}

// --- Buffer exactly fits (size == would_write + 1) ---
TEST_F(SnprintIntTest, ExactBufferFit) {
    char small[4];  // fits "042\0"
    int ret = snprint_int(small, 4, 42, '0', 3);
    EXPECT_STREQ("042", small);
    EXPECT_EQ(3, ret);
}

// --- Buffer too small: truncation ---
TEST_F(SnprintIntTest, BufferTooSmallTruncates) {
    char small[3];  // only room for 2 chars + NUL
    int ret = snprint_int(small, 3, 12345, '0', 5);
    // would_write = 5, total = 2, output is truncated but NUL-terminated
    EXPECT_EQ('\0', small[2]);
    EXPECT_EQ(5, ret);  // still reports would_write
}

TEST_F(SnprintIntTest, SizeOneWritesOnlyNul) {
    char tiny[1];
    int ret = snprint_int(tiny, 1, 99, '0', 3);
    EXPECT_EQ('\0', tiny[0]);
    EXPECT_EQ(3, ret);
}

// --- INT_MAX / INT_MIN boundary ---
TEST_F(SnprintIntTest, IntMax) {
    int ret = snprint_int(buf, sizeof(buf), INT_MAX, ' ', 0);
    char expected[32];
    snprintf(expected, sizeof(expected), "%d", INT_MAX);
    EXPECT_STREQ(expected, buf);
    EXPECT_EQ((int)strlen(expected), ret);
}

// Note: INT_MIN causes ABS(INT_MIN) to overflow in C; document expected behaviour.
// If your code guards against this, adjust accordingly.
TEST_F(SnprintIntTest, IntMinPlusOne) {
    int val = INT_MIN + 1;
    int ret = snprint_int(buf, sizeof(buf), val, ' ', 0);
    char expected[32];
    snprintf(expected, sizeof(expected), "%d", val);
    EXPECT_STREQ(expected, buf);
    EXPECT_EQ((int)strlen(expected), ret);
}

// --- Padding with space vs '0' ---
TEST_F(SnprintIntTest, SpacePad) {
    int ret = snprint_int(buf, sizeof(buf), 7, ' ', 5);
    EXPECT_STREQ("    7", buf);
    EXPECT_EQ(5, ret);
}

TEST_F(SnprintIntTest, ZeroPad) {
    int ret = snprint_int(buf, sizeof(buf), 7, '0', 5);
    EXPECT_STREQ("00007", buf);
    EXPECT_EQ(5, ret);
}


// ============================================================
// print_temperature – edge cases
// ============================================================

class PrintTemperatureTest : public ::testing::Test {
protected:
    char buf[32];

    void SetUp() override {
        temperature_unit = 0;  // Celsius
        memset(buf, 0xAB, sizeof(buf));  // poison to catch missing NUL
    }
};

// ---- Celsius, no unit suffix ----

TEST_F(PrintTemperatureTest, Celsius_Zero_NoUnit_NoOffset) {
    int ret = print_temperature(buf, sizeof(buf), 0, false, false);
    EXPECT_STREQ("000", buf);
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, Celsius_Positive_NoUnit) {
    int ret = print_temperature(buf, sizeof(buf), 25, false, false);
    EXPECT_STREQ("025", buf);
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, Celsius_Negative_NoUnit) {
    int ret = print_temperature(buf, sizeof(buf), -5, false, false);
    EXPECT_STREQ("-05", buf);   // snprint_int(-5, pad='0', width=3)
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, Celsius_Large_NoUnit) {
    // 1000 → 4 digits, wider than width=3
    int ret = print_temperature(buf, sizeof(buf), 1000, false, false);
    EXPECT_STREQ("1000", buf);
    EXPECT_EQ(4, ret);
}

// ---- Celsius, with unit suffix ----

TEST_F(PrintTemperatureTest, Celsius_WithUnit) {
    int ret = print_temperature(buf, sizeof(buf), 20, true, false);
    // "020" + "°C" = 5 chars + NUL
    EXPECT_STREQ("020""\xB0""C", buf);
    EXPECT_EQ(5, ret);
}

// ---- Fahrenheit, offset = false (conversion only, no +32) ----

TEST_F(PrintTemperatureTest, Fahrenheit_NoOffset_NoUnit) {
    temperature_unit = 1;  // Fahrenheit
    // 100 * 9 / 5 = 180
    int ret = print_temperature(buf, sizeof(buf), 100, false, false);
    EXPECT_STREQ("180", buf);
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, Fahrenheit_NoOffset_Zero) {
    temperature_unit = 1;  // Fahrenheit
    // 0 * 9 / 5 = 0
    int ret = print_temperature(buf, sizeof(buf), 0, false, false);
    EXPECT_STREQ("000", buf);
    EXPECT_EQ(3, ret);
}

// ---- Fahrenheit, offset = true (conversion + 32) ----

TEST_F(PrintTemperatureTest, Fahrenheit_WithOffset_Freezing) {
    temperature_unit = 1;  // Fahrenheit
    // 0°C → 0*9/5 + 32 = 32°F
    int ret = print_temperature(buf, sizeof(buf), 0, false, true);
    EXPECT_STREQ("032", buf);
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, Fahrenheit_WithOffset_Boiling) {
    temperature_unit = 1;  // Fahrenheit
    // 100°C → 100*9/5 + 32 = 212°F
    int ret = print_temperature(buf, sizeof(buf), 100, false, true);
    EXPECT_STREQ("212", buf);
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, Fahrenheit_WithOffset_NegativeBodyTemp) {
    temperature_unit = 1;  // Fahrenheit
    // -40°C → -40*9/5 + 32 = -40°F (the crossover point)
    int ret = print_temperature(buf, sizeof(buf), -40, false, true);
    EXPECT_STREQ("-40", buf);
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, Fahrenheit_WithOffset_WithUnit) {
    temperature_unit = 1;  // Fahrenheit
    // 0 C -> 0*9/5 + 32 = 32 F, with F suffix
    int ret = print_temperature(buf, sizeof(buf), 0, true, true);
    EXPECT_STREQ("032""\xB0""F", buf);
    EXPECT_EQ(5, ret);
}

// ---- Buffer size edge cases ----

TEST_F(PrintTemperatureTest, SizeZero_ReturnsWouldWrite) {
    // size == 0 passed through to snprint_int, unit suffix still counted
    int ret = print_temperature(nullptr, 0, 20, true, false);
    EXPECT_GT(ret, 0);  // should return total would-write length
}

TEST_F(PrintTemperatureTest, SizeOne_OnlyNul) {
    char tiny[1] = { 0x7F };
    print_temperature(tiny, 1, 20, false, false);
    EXPECT_EQ('\0', tiny[0]);
}

TEST_F(PrintTemperatureTest, ExactSize_NoUnit) {
    char exact[4];  // "020\0"
    int ret = print_temperature(exact, 4, 20, false, false);
    EXPECT_STREQ("020", exact);
    EXPECT_EQ(3, ret);
}

TEST_F(PrintTemperatureTest, TooSmall_Truncates_WithUnit) {
    // Buffer only fits the integer part, suffix gets cut
    char small[4];  // room for "020\0" but not the "C" suffix
    int ret = print_temperature(small, 4, 20, true, false);
    EXPECT_STREQ("020", small);
    EXPECT_EQ('\0', small[3]);      // must be NUL-terminated
    EXPECT_GT(ret, 3);              // would_write > what fit (3 digits + "C")
}

// ---- offset flag has no effect in Celsius mode ----

TEST_F(PrintTemperatureTest, Celsius_OffsetIgnored) {
    int r1 = print_temperature(buf, sizeof(buf), 20, false, false);
    char buf2[32];
    int r2 = print_temperature(buf2, sizeof(buf2), 20, false, true);
    EXPECT_STREQ(buf, buf2);
    EXPECT_EQ(r1, r2);
}
