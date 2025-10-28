#include "dpt.h"
#include <cmath>
#include <cassert>
#include <cstdio>
#include <limits>

using namespace esphome::knx_tp;

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
  do { \
    if (condition) { \
      printf("✅ PASS: %s\n", message); \
      tests_passed++; \
    } else { \
      printf("❌ FAIL: %s\n", message); \
      tests_failed++; \
    } \
  } while(0)

#define TEST_FLOAT_EQUAL(a, b, epsilon, message) \
  TEST_ASSERT(std::fabs((a) - (b)) < (epsilon), message)

void test_dpt1_bool() {
  printf("\n=== Testing DPT 1 (Boolean) ===\n");

  // Test encoding
  auto encoded_true = DPT::encode_dpt1(true);
  TEST_ASSERT(encoded_true.size() == 1, "DPT1 encode true size");
  TEST_ASSERT(encoded_true[0] == 0x01, "DPT1 encode true value");

  auto encoded_false = DPT::encode_dpt1(false);
  TEST_ASSERT(encoded_false.size() == 1, "DPT1 encode false size");
  TEST_ASSERT(encoded_false[0] == 0x00, "DPT1 encode false value");

  // Test decoding
  TEST_ASSERT(DPT::decode_dpt1({0x01}) == true, "DPT1 decode true");
  TEST_ASSERT(DPT::decode_dpt1({0x00}) == false, "DPT1 decode false");
  TEST_ASSERT(DPT::decode_dpt1({}) == false, "DPT1 decode empty returns false");
}

void test_dpt5_uint8() {
  printf("\n=== Testing DPT 5 (8-bit unsigned) ===\n");

  // Test encoding
  auto encoded = DPT::encode_dpt5(123);
  TEST_ASSERT(encoded.size() == 1, "DPT5 encode size");
  TEST_ASSERT(encoded[0] == 123, "DPT5 encode value");

  // Test edge cases
  auto encoded_min = DPT::encode_dpt5(0);
  TEST_ASSERT(encoded_min[0] == 0, "DPT5 encode min (0)");

  auto encoded_max = DPT::encode_dpt5(255);
  TEST_ASSERT(encoded_max[0] == 255, "DPT5 encode max (255)");

  // Test decoding
  TEST_ASSERT(DPT::decode_dpt5({123}) == 123, "DPT5 decode value");
  TEST_ASSERT(DPT::decode_dpt5({}) == 0, "DPT5 decode empty returns 0");
}

void test_dpt5_percentage() {
  printf("\n=== Testing DPT 5.001 (Percentage) ===\n");

  // Test encoding
  auto encoded_50 = DPT::encode_dpt5_percentage(50.0f);
  TEST_ASSERT(encoded_50.size() == 1, "DPT5.001 encode size");
  // 50% = 127.5 ≈ 128
  TEST_ASSERT(std::abs(encoded_50[0] - 128) <= 1, "DPT5.001 encode 50%");

  auto encoded_0 = DPT::encode_dpt5_percentage(0.0f);
  TEST_ASSERT(encoded_0[0] == 0, "DPT5.001 encode 0%");

  auto encoded_100 = DPT::encode_dpt5_percentage(100.0f);
  TEST_ASSERT(encoded_100[0] == 255, "DPT5.001 encode 100%");

  // Test clamping
  auto encoded_over = DPT::encode_dpt5_percentage(150.0f);
  TEST_ASSERT(encoded_over[0] == 255, "DPT5.001 clamps > 100%");

  auto encoded_under = DPT::encode_dpt5_percentage(-10.0f);
  TEST_ASSERT(encoded_under[0] == 0, "DPT5.001 clamps < 0%");

  // Test decoding
  TEST_FLOAT_EQUAL(DPT::decode_dpt5_percentage({0}), 0.0f, 0.1f, "DPT5.001 decode 0%");
  TEST_FLOAT_EQUAL(DPT::decode_dpt5_percentage({255}), 100.0f, 0.1f, "DPT5.001 decode 100%");
  TEST_FLOAT_EQUAL(DPT::decode_dpt5_percentage({128}), 50.2f, 1.0f, "DPT5.001 decode ~50%");
}

void test_dpt5_angle() {
  printf("\n=== Testing DPT 5.003 (Angle) ===\n");

  // Test encoding
  auto encoded_0 = DPT::encode_dpt5_angle(0.0f);
  TEST_ASSERT(encoded_0[0] == 0, "DPT5.003 encode 0°");

  auto encoded_360 = DPT::encode_dpt5_angle(360.0f);
  TEST_ASSERT(encoded_360[0] == 0, "DPT5.003 encode 360° wraps to 0°");

  auto encoded_180 = DPT::encode_dpt5_angle(180.0f);
  TEST_ASSERT(std::abs(encoded_180[0] - 128) <= 1, "DPT5.003 encode 180°");

  // Test negative angle normalization
  auto encoded_neg = DPT::encode_dpt5_angle(-90.0f);
  TEST_ASSERT(encoded_neg.size() == 1, "DPT5.003 negative angle normalized");

  // Test NaN/Infinity (FIX #5)
  auto encoded_nan = DPT::encode_dpt5_angle(NAN);
  TEST_ASSERT(encoded_nan[0] == 0, "DPT5.003 NaN returns 0°");

  auto encoded_inf = DPT::encode_dpt5_angle(INFINITY);
  TEST_ASSERT(encoded_inf[0] == 0, "DPT5.003 Infinity returns 0°");

  auto encoded_neg_inf = DPT::encode_dpt5_angle(-INFINITY);
  TEST_ASSERT(encoded_neg_inf[0] == 0, "DPT5.003 -Infinity returns 0°");
}

void test_dpt9_float() {
  printf("\n=== Testing DPT 9 (2-byte float) ===\n");

  // Test encoding normal values
  auto encoded_20 = DPT::encode_dpt9(20.5f);
  TEST_ASSERT(encoded_20.size() == 2, "DPT9 encode size");

  // Test decoding
  float decoded_20 = DPT::decode_dpt9(encoded_20);
  TEST_FLOAT_EQUAL(decoded_20, 20.5f, 0.1f, "DPT9 round-trip 20.5");

  // Test edge cases
  auto encoded_0 = DPT::encode_dpt9(0.0f);
  float decoded_0 = DPT::decode_dpt9(encoded_0);
  TEST_FLOAT_EQUAL(decoded_0, 0.0f, 0.01f, "DPT9 round-trip 0.0");

  auto encoded_neg = DPT::encode_dpt9(-15.5f);
  float decoded_neg = DPT::decode_dpt9(encoded_neg);
  TEST_FLOAT_EQUAL(decoded_neg, -15.5f, 0.1f, "DPT9 round-trip -15.5");

  // Test clamping (FIX #2)
  auto encoded_max = DPT::encode_dpt9(700000.0f); // Over max
  TEST_ASSERT(encoded_max.size() == 2, "DPT9 clamps overflow");
  float decoded_max = DPT::decode_dpt9(encoded_max);
  TEST_ASSERT(decoded_max <= 670760.96f, "DPT9 max clamped to valid range");

  auto encoded_min = DPT::encode_dpt9(-700000.0f); // Under min
  float decoded_min = DPT::decode_dpt9(encoded_min);
  TEST_ASSERT(decoded_min >= -671088.64f, "DPT9 min clamped to valid range");

  // Test NaN/Infinity (FIX #2)
  auto encoded_nan = DPT::encode_dpt9(NAN);
  TEST_ASSERT(encoded_nan[0] == 0x00 && encoded_nan[1] == 0x00, "DPT9 NaN returns 0x0000");

  auto encoded_inf = DPT::encode_dpt9(INFINITY);
  TEST_ASSERT(encoded_inf.size() == 2, "DPT9 Infinity handled");

  // Test decode with invalid data (FIX #1)
  TEST_ASSERT(DPT::decode_dpt9({}) == 0.0f, "DPT9 decode empty returns 0");
  TEST_ASSERT(DPT::decode_dpt9({0x12}) == 0.0f, "DPT9 decode 1-byte returns 0");
}

void test_dpt14_float() {
  printf("\n=== Testing DPT 14 (4-byte float) ===\n");

  // Test encoding
  auto encoded = DPT::encode_dpt14(123.456f);
  TEST_ASSERT(encoded.size() == 4, "DPT14 encode size");

  // Test decoding
  float decoded = DPT::decode_dpt14(encoded);
  TEST_FLOAT_EQUAL(decoded, 123.456f, 0.001f, "DPT14 round-trip 123.456");

  // Test negative
  auto encoded_neg = DPT::encode_dpt14(-789.012f);
  float decoded_neg = DPT::decode_dpt14(encoded_neg);
  TEST_FLOAT_EQUAL(decoded_neg, -789.012f, 0.001f, "DPT14 round-trip negative");

  // Test invalid data
  TEST_ASSERT(DPT::decode_dpt14({}) == 0.0f, "DPT14 decode empty");
  TEST_ASSERT(DPT::decode_dpt14({0x12, 0x34}) == 0.0f, "DPT14 decode < 4 bytes");
}

void test_dpt16_string() {
  printf("\n=== Testing DPT 16.001 (String) ===\n");

  // Test encoding (FIX #10)
  auto encoded = DPT::encode_dpt16("Hello KNX");
  TEST_ASSERT(encoded.size() == 10, "DPT16 encode normal string"); // 9 chars + null
  TEST_ASSERT(encoded[9] == 0, "DPT16 null terminator");

  // Test max length (14 chars max)
  auto encoded_long = DPT::encode_dpt16("12345678901234567890"); // 20 chars
  TEST_ASSERT(encoded_long.size() <= 15, "DPT16 truncates to max 14+1"); // 14 + null

  // Test decoding
  std::string decoded = DPT::decode_dpt16(encoded);
  TEST_ASSERT(decoded == "Hello KNX", "DPT16 decode matches");

  // Test ASCII validation
  std::string test_with_newline = "Test\nBad";
  auto encoded_bad = DPT::encode_dpt16(test_with_newline);
  std::string decoded_bad = DPT::decode_dpt16(encoded_bad);
  TEST_ASSERT(decoded_bad.find('\n') == std::string::npos, "DPT16 filters non-printable");

  // Test empty
  TEST_ASSERT(DPT::decode_dpt16({}) == "", "DPT16 decode empty");
}

void test_dpt19_datetime() {
  printf("\n=== Testing DPT 19.001 (DateTime) ===\n");

  // Test encoding
  DPT::DateTime dt = {2024, 10, 27, 1, 14, 30, 45, 0, false, false, false, false, false, false, false, false};
  auto encoded = DPT::encode_dpt19(dt);
  TEST_ASSERT(encoded.size() == 8, "DPT19 encode size");

  // Test decoding (FIX #3)
  DPT::DateTime decoded = DPT::decode_dpt19(encoded);
  TEST_ASSERT(decoded.year == 2024, "DPT19 year");
  TEST_ASSERT(decoded.month == 10, "DPT19 month");
  TEST_ASSERT(decoded.day == 27, "DPT19 day");
  TEST_ASSERT(decoded.hour == 14, "DPT19 hour");
  TEST_ASSERT(decoded.minute == 30, "DPT19 minute");
  TEST_ASSERT(decoded.second == 45, "DPT19 second");

  // Test invalid data (FIX #3)
  DPT::DateTime invalid = DPT::decode_dpt19({});
  TEST_ASSERT(invalid.year == 2000, "DPT19 decode empty returns default");

  DPT::DateTime invalid2 = DPT::decode_dpt19({0x01, 0x02, 0x03}); // Only 3 bytes
  TEST_ASSERT(invalid2.year == 2000, "DPT19 decode < 8 bytes returns default");

  // Test out of range values get clamped
  DPT::DateTime bad_dt = {2100, 15, 35, 0, 25, 70, 70, 0, false, false, false, false, false, false, false, false};
  auto encoded_bad = DPT::encode_dpt19(bad_dt);
  DPT::DateTime decoded_bad = DPT::decode_dpt19(encoded_bad);
  TEST_ASSERT(decoded_bad.year <= 2089, "DPT19 year clamped to max 2089");
  TEST_ASSERT(decoded_bad.month <= 12, "DPT19 month clamped to max 12");
  TEST_ASSERT(decoded_bad.day <= 31, "DPT19 day clamped to max 31");
  TEST_ASSERT(decoded_bad.hour <= 23, "DPT19 hour clamped to max 23");
  TEST_ASSERT(decoded_bad.minute <= 59, "DPT19 minute clamped to max 59");
  TEST_ASSERT(decoded_bad.second <= 59, "DPT19 second clamped to max 59");
}

void test_dpt20_hvac() {
  printf("\n=== Testing DPT 20.102 (HVAC Mode) ===\n");

  // Test all valid modes
  auto encoded_auto = DPT::encode_dpt20_102(DPT::HVACMode::AUTO);
  TEST_ASSERT(encoded_auto[0] == 0, "DPT20.102 AUTO");

  auto encoded_comfort = DPT::encode_dpt20_102(DPT::HVACMode::COMFORT);
  TEST_ASSERT(encoded_comfort[0] == 1, "DPT20.102 COMFORT");

  auto encoded_standby = DPT::encode_dpt20_102(DPT::HVACMode::STANDBY);
  TEST_ASSERT(encoded_standby[0] == 2, "DPT20.102 STANDBY");

  auto encoded_night = DPT::encode_dpt20_102(DPT::HVACMode::NIGHT);
  TEST_ASSERT(encoded_night[0] == 3, "DPT20.102 NIGHT");

  auto encoded_frost = DPT::encode_dpt20_102(DPT::HVACMode::FROST_PROTECTION);
  TEST_ASSERT(encoded_frost[0] == 4, "DPT20.102 FROST_PROTECTION");

  // Test decoding
  TEST_ASSERT(DPT::decode_dpt20_102({0}) == DPT::HVACMode::AUTO, "DPT20.102 decode AUTO");
  TEST_ASSERT(DPT::decode_dpt20_102({1}) == DPT::HVACMode::COMFORT, "DPT20.102 decode COMFORT");
  TEST_ASSERT(DPT::decode_dpt20_102({3}) == DPT::HVACMode::NIGHT, "DPT20.102 decode NIGHT");

  // Test invalid
  TEST_ASSERT(DPT::decode_dpt20_102({}) == DPT::HVACMode::AUTO, "DPT20.102 decode empty defaults to AUTO");
  TEST_ASSERT(DPT::decode_dpt20_102({99}) == DPT::HVACMode::AUTO, "DPT20.102 decode invalid defaults to AUTO");
}

int main() {
  printf("\n");
  printf("╔════════════════════════════════════════════════════════════╗\n");
  printf("║         KNX TP Component - DPT Unit Tests                  ║\n");
  printf("║  Testing Security Fixes & Edge Cases                       ║\n");
  printf("╚════════════════════════════════════════════════════════════╝\n");

  // Run all tests
  test_dpt1_bool();
  test_dpt5_uint8();
  test_dpt5_percentage();
  test_dpt5_angle();
  test_dpt9_float();
  test_dpt14_float();
  test_dpt16_string();
  test_dpt19_datetime();
  test_dpt20_hvac();

  // Print summary
  printf("\n");
  printf("╔════════════════════════════════════════════════════════════╗\n");
  printf("║                    TEST SUMMARY                            ║\n");
  printf("╠════════════════════════════════════════════════════════════╣\n");
  printf("║  ✅ Tests Passed: %-3d                                      ║\n", tests_passed);
  printf("║  ❌ Tests Failed: %-3d                                      ║\n", tests_failed);
  printf("║  📊 Total Tests:  %-3d                                      ║\n", tests_passed + tests_failed);
  printf("╚════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  if (tests_failed == 0) {
    printf("🎉 ALL TESTS PASSED! 🎉\n\n");
    return 0;
  } else {
    printf("⚠️  SOME TESTS FAILED ⚠️\n\n");
    return 1;
  }
}
