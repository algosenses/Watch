#ifndef DEBUG_H
#define DEBUG_H

/*! Initialize the uart peripheral and create mutex */
void InitDebugUart(void);

/*! Print a string */
void PrintString(signed char * const pString);

/*! Print two strings */
void PrintString2(signed char * const pString1,
                  signed char * const pString2);

/*! Print three strings */
void PrintString3(signed char * const pString1,
                  signed char * const pString2,
                  signed char * const pString3);

/*! Print a 16 bit value in decimal */
void PrintDecimal(unsigned int Value);

/*! Print a 16 bit Value in hexadecimal */
void PrintHex(unsigned int Value);

/*! Print a 16 bit value and a newline*/
void PrintDecimalAndNewline(unsigned int Value);

/*! Print a string and a 16 bit value */
void PrintStringAndDecimal(signed char * const pString,unsigned int Value);

/*! Print a string, space and 16 bit value */
void PrintStringAndSpaceAndDecimal(signed char * const pString,unsigned int Value);

/*! Print a string and an 16 bit value represented in hexadecimal */
void PrintStringAndHex(signed char * const pString,unsigned int Value);

/*! Print a string 16 bit value, and another string and 16 bit value */
void PrintStringAndTwoDecimals(signed char * const pString1,
                               unsigned int Value1,
                               signed char * const pString2,
                               unsigned int Value2);

/*! Print a string and two 16 bit values with spaces inbetween */
void PrintStringSpaceAndTwoDecimals(signed char * const pString1,
                                    unsigned int Value1,
                                    unsigned int Value2);

/*! Print a string and three 16 bit values with spaces inbetween */
void PrintStringSpaceAndThreeDecimals(signed char * const pString1,
                                      unsigned int Value1,
                                      unsigned int Value2,
                                      unsigned int Value3);

/*! Print a signed number and a newline */
void PrintSignedDecimalAndNewline(signed int Value);

/*! Convert a 16 bit value into a string */
void ToDecimalString(unsigned int Value, signed char * pString);

/*! Convert a 16 bit value into a hexadecimal string */
void ToHexString(unsigned int Value, signed char * pString);

#endif
