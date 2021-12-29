#include <Arduino.h>
#include <unity.h>

// void tearDown(void) {
// // clean stuff up here
// }

void test_led_builtin_pin_number(void) {
    // update here if schematic changes
    // TEST_ASSERT_EQUAL(13, );
}

void test_output_state_high(void) {
    // display 
//    digitalWrite(22, HIGH); // SCL
//    digitalWrite(21, HIGH); // SDA
    // Encoders SPI
//    digitalWrite(19, HIGH); // CIPO
//    digitalWrite(18, HIGH); // SCK
//    digitalWrite(23, HIGH); // COPI
//    digitalWrite(5, HIGH);  // PS-1
//    digitalWrite(36, HIGH); // PS-2
    // buttons
    digitalWrite(15, HIGH); // UP
    digitalWrite(16, HIGH); // OK
    digitalWrite(17, HIGH); // DOWN

        // display 
    TEST_ASSERT_EQUAL(HIGH, digitalRead(22)); // SCL
    TEST_ASSERT_EQUAL(HIGH, digitalRead(21)); // SDA
    // Encoders SPI
//    TEST_ASSERT_EQUAL(HIGH, digitalRead(19)); // CIPO
//    TEST_ASSERT_EQUAL(HIGH, digitalRead(18)); // SCK
//    TEST_ASSERT_EQUAL(HIGH, digitalRead(23)); // COPI
//    TEST_ASSERT_EQUAL(HIGH, digitalRead(5));  // PS-1
//    TEST_ASSERT_EQUAL(HIGH, digitalRead(36)); // PS-2
    // buttons
    TEST_ASSERT_EQUAL(HIGH, digitalRead(15)); // UP
    TEST_ASSERT_EQUAL(HIGH, digitalRead(16)); // OK
    TEST_ASSERT_EQUAL(HIGH, digitalRead(17)); // DOWN

}

void test_output_state_low(void) {
    // display 
//    digitalWrite(22, LOW); // SCL
//    digitalWrite(21, LOW); // SDA
    // Encoders SPI
//    digitalWrite(19, LOW); // CIPO
//    digitalWrite(18, LOW); // SCK
//    digitalWrite(23, LOW); // COPI
//    digitalWrite(5, LOW);  // PS-1
//    digitalWrite(36, LOW); // PS-2
    // buttons
    digitalWrite(15, LOW); // UP
    digitalWrite(16, LOW); // OK
    digitalWrite(17, LOW); // DOWN

        // display 
//    TEST_ASSERT_EQUAL(LOW, digitalRead(22)); // SCL
//    TEST_ASSERT_EQUAL(LOW, digitalRead(21)); // SDA
    // Encoders SPI
//    TEST_ASSERT_EQUAL(LOW, digitalRead(19)); // CIPO
//    TEST_ASSERT_EQUAL(LOW, digitalRead(18)); // SCK
//    TEST_ASSERT_EQUAL(LOW, digitalRead(23)); // COPI
//    TEST_ASSERT_EQUAL(LOW, digitalRead(5));  // PS-1
//    TEST_ASSERT_EQUAL(LOW, digitalRead(36)); // PS-2
    // buttons
    TEST_ASSERT_EQUAL(LOW, digitalRead(15)); // UP
    TEST_ASSERT_EQUAL(LOW, digitalRead(16)); // OK
    TEST_ASSERT_EQUAL(LOW, digitalRead(17)); // DOWN
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

//    pinMode(22, OUTPUT); // SCL
//    pinMode(21, OUTPUT); // SDA
    // Encoders SPI
//    pinMode(19, OUTPUT); // CIPO
//    pinMode(18, OUTPUT); // SCK
//    pinMode(23, OUTPUT); // COPI
//    pinMode(5, OUTPUT);  // PS-1
//    pinMode(36, OUTPUT); // PS-2
    // buttons
    pinMode(15, OUTPUT); // UP
    pinMode(16, OUTPUT); // OK
    pinMode(17, OUTPUT); // DOWN

    UNITY_BEGIN();    // IMPORTANT LINE!
    RUN_TEST(test_led_builtin_pin_number);

    pinMode(48, OUTPUT);
}

uint8_t i = 0;
uint8_t max_blinks = 5;

void loop() {
    if (i < max_blinks)
    {
        RUN_TEST(test_output_state_high);
        delay(500);
        RUN_TEST(test_output_state_low);
        delay(500);
        i++;
    }
    else if (i == max_blinks) {
      UNITY_END(); // stop unit testing
    }
}