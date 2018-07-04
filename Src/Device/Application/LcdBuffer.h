#ifndef METAWATCH_LCD_BUFFER_H
#define METAWATCH_LCD_BUFFER_H

#define STARTING_ROW                  ( 0 )
#define WATCH_DRAWN_IDLE_BUFFER_ROWS  ( 30 )
#define PHONE_IDLE_BUFFER_ROWS        ( 66 )

#define ADD_SPACE_AT_END      ( 1 )
#define DONT_ADD_SPACE_AT_END ( 0 )

#define LEFT_JUSTIFIED         ( 0 )
#define RIGHT_JUSTIFIED        ( 10 )

void InitMyBuffer(void);

unsigned char WriteString(unsigned char *pString,
                          unsigned char RowOffset,
                          unsigned char ColumnOffset,
                          unsigned char AddSpace);
void WriteSpriteChar(unsigned char Char,
                            unsigned char RowOffset,
                            unsigned char ColumnOffset);
void WriteSpriteDigit(unsigned char Digit,
                             unsigned char RowOffset,
                             unsigned char ColumnOffset,
                             signed char ShiftAmount);
void FillMyBuffer(unsigned char StartingRow,
                         unsigned char NumberOfRows,
                         unsigned char FillValue);

void PrepareMyBufferForLcd(unsigned char StartingRow,
                                  unsigned char NumberOfRows);

void CopyRowsIntoMyBuffer(unsigned char const* pImage,
                                 unsigned char StartingRow,
                                 unsigned char NumberOfRows);

void CopyColumnsIntoMyBuffer(unsigned char const* pImage,
                                    unsigned char StartingRow,
                                    unsigned char NumberOfRows,
                                    unsigned char StartingColumn,
                                    unsigned char NumberOfColumns);
void WriteTimeDigit(unsigned char Digit,
                           unsigned char RowOffset,
                           unsigned char ColumnOffset,
                           unsigned char JustificationOffset);
void WriteTimeColon(unsigned char RowOffset,
                           unsigned char ColumnOffset,
                           unsigned char Justification);
void DisplayDataSeparator(unsigned char RowOffset,
                                 unsigned char ColumnOffset);
void WriteFoo(unsigned char const * pFoo,
                     unsigned char RowOffset,
                     unsigned char ColumnOffset);
void AddDecimalPoint8w10h(unsigned char RowOffset,
                                 unsigned char ColumnOffset);

unsigned char BitRev8(unsigned char byte);

#endif /*METAWATCH_LCD_BUFFER_H*/
