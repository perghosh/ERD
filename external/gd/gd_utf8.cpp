#include <cassert>
#include <stdint.h>
#include <stdexcept>
#include <initializer_list>
#include <vector>

#include "gd_types.h"

#include "gd_utf8.hpp"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-const-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
#elif defined( _MSC_VER )
#endif



namespace gd { 
   namespace utf8 {

      const uint8_t CHARACTER_TAB = 9;
      const uint8_t CHARACTER_LINEFEED = 10;
      const uint8_t CHARACTER_CARRAGERETURN = 13;
      const uint8_t CHARACTER_SPACE = 32;

      static const uint32_t CHARACTER_1_BYTE_MASK = 0x80;
      static const uint32_t CHARACTER_2_BYTE_MASK = 0xE0;
      static const uint32_t CHARACTER_3_BYTE_MASK = 0xF0;
      static const uint32_t CHARACTER_4_BYTE_MASK = 0xF8;


      /**
       * @brief Number of bytes needed for utf-8 character.
       * Each character in utf-8  has a codepoint where information about how
       * many bytes that follows are needed to store character. This table has 
       * information about number of bytes that is needed for character.
       * 
         | First code point | Last code point |	Byte 1   | Byte 2   | Byte 3   | Byte 4   | Code points |
         | :---             | :---            | :---     | :---     | :---     | :---     | :---        |
         | U+0000 	       | U+007F 	       | 0xxxxxxx | 128      |          |          |             |
         | U+0080 	       | U+07FF 	       | 110xxxxx | 10xxxxxx | 1920     |          |             |
         | U+0800 	       | U+FFFF 	       | 1110xxxx | 10xxxxxx | 10xxxxxx |	61440    |             |
         | U+10000 	       | U+10FFFF 	    | 11110xxx | 10xxxxxx | 10xxxxxx |	10xxxxxx | 1048576     |   
      */
      static const uint8_t pNeededByteCount_s[0x100] =
      {
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x00-0x0F */
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x10-0x1F */
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x20-0x2F */
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x30-0x3F */
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x40-0x4F */
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x50-0x5F */
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x60-0x6F */
          1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x70-0x7F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xA0-0xAF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xB0-0xBF */
          0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xC0-0xCF */
          2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xD0-0xDF */
          3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, /* 0xE0-0xEF */
          4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0, /* 0xF0-0xFF */
      };


      /**
       * @brief Lookup table used to convert hexadecimal value in text to
       * value stored in byte.
       * It takes the ascii code for character and use the ascii number to find
       * what hex value it is in this table.
      */
      static const uint8_t pHexValue_s[0x100] =
      {
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x20-0x2F */
          0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, /* 0x30-0x3F (0 - 9) */
          0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F (A - F) */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
          0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F (a - f) */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x70-0x7F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xA0-0xAF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xB0-0xBF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xC0-0xCF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xD0-0xDF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xE0-0xEF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xF0-0xFF */
      };

      /**
       * @brief Lookup table used to validate hexadecimal value in text to
       * value stored in byte.
       * It takes the ascii code for character and use the ascii number to find
       * if it is valid hexadecimal character
      */
      static const uint8_t pHexValidate_s[0x100] =
      {
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x20-0x2F */
          1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0, /* 0x30-0x3F (0 - 9) */
          0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F (A - F) */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
          0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F (a - f) */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x70-0x7F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xA0-0xAF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xB0-0xBF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xC0-0xCF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xD0-0xDF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xE0-0xEF */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xF0-0xFF */
      };

      /**
      * @brief Lookuptable used to validate xml characters. If value i 0 then
      * character isn't encoded. Bit 1 is set for characters that are encoded
      */
      static const uint8_t pEncodeXml_s[0x80] =
      {
       //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
         0,0,6,0,0,0,5,6,0,0,0,0,0,0,0,0, /* 0x20-0x2F "=quot &=amp '=apos */
         0,0,0,0,0,0,0,0,0,0,0,0,4,0,4,0, /* 0x30-0x3F <=lt >=gt */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x70-0x7F */
      };



      /**
       * @brief Lookuptable used to validate uri characters. If value i 0 then
       * character isn't encoded. Bit 1 is set for characters that are encoded
       * with the browser api `encodeURI`. Bit 2 is set with characters that are 
       * encoded with `encodeURIComponent`
      */
      static const uint8_t pEncodeUri_s[0x80] =
      {
        //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
          3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, /* 0x00-0x0F */
          3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, /* 0x10-0x1F */
          3,0,3,2,2,3,2,0,0,0,0,2,2,0,0,2, /* 0x20-0x2F */
          0,0,0,0,0,0,0,0,0,0,2,2,3,2,3,2, /* 0x30-0x3F */
          2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F */
          0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,0, /* 0x50-0x5F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F */
          0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0, /* 0x70-0x7F */
      };


      static const char pszHEX_s[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };


      /**
       * @brief number characters
      */
      static const uint8_t pIsDigit_s[0x80] =
      {
        //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
          0,0,0,0,0,0,0,0,0,0,0,3,0,3,3,0, /* 0x20-0x2F */
          1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0, /* 0x30-0x3F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
          0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F */
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x70-0x7F */
      };

   const char pIsJsonEscape_s[256] =
   {
      /* 0 */ 0,0,0,0, 0,0,0,0, 'b','t','n',0, 'f','r',0,0, /* 0   - 15  */
      /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 16  - 31  */
      /* 2 */ 0,0,'"',0, 0,0,0,0, 0,0,0,0, 0,0,0,0,/* 32  - 47  */
      /* 3 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 48  - 63  */

      /* 4 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 64  - 79  */
      /* 5 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, '\\',0,0,0,/* 80  - 95  */
      /* 6 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 96  - 111 */
      /* 7 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 112 - 127 */

      /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 128 - 143 */
      /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 144 - 159 */
      /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 160 - 175 */
      /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 176 - 191 */

      /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 192 - 207 */
      /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 208 - 223 */
      /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  /* 224 - 239 */
      /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0   /* 240 - 255 */
   };




      /** ---------------------------------------------------------------------
       * @brief get the plain ascii character from utf8 sequence for same character
       * @param uCharacter character in utf8 format that is converted to ascii code
       * @param value reference to char that get the ascii code from utf8 sequence
      */
      void normalize( uint32_t uCharacter, char& value )
      {
         if( uCharacter < 0x80 ) value = static_cast<uint8_t>(uCharacter);
         else if( ((uCharacter >> 8) & 0xc0) == 0xc0 )
         {
            value = static_cast<uint8_t>(uCharacter >> 8) & 0x1f;
            value |= static_cast<uint8_t>(uCharacter) & 0x3f;
            
         }
         assert(false); throw std::runtime_error("invalid convert  (operation = normalize)");
      }


      uint32_t character(const uint8_t* pubszCharacter)
      {
         switch( pNeededByteCount_s[*pubszCharacter] )
         {
         case 0:
            throw std::runtime_error("invalid UTF-8  (operation = character)");
         case 1:
            return static_cast<uint32_t>(*pubszCharacter);
         case 2:
            return static_cast<uint32_t>( ((0x1f & pubszCharacter[0]) << 6) | (0x3f & pubszCharacter[1]) );
         case 3:
            return static_cast<uint32_t>( ((0x0f & pubszCharacter[0]) << 12) | ((0x3f & pubszCharacter[1]) << 6) | (0x3f & pubszCharacter[2]) );
         case 4:
            return static_cast<uint32_t>( ((0x07 & pubszCharacter[0]) << 18) | ((0x3f & pubszCharacter[1]) << 12) | ((0x3f & pubszCharacter[2]) << 6) | (0x3f & pubszCharacter[3]) );
         }
         return 0;
      }

      /** ---------------------------------------------------------------------
       * @brief count utf8 characters in buffer
       * @param pubszText pointer to buffer with text where characters are counted
       * @return number of utf8 characters in buffer
      */
      std::pair<uint32_t, const uint8_t*> count( const uint8_t* pubszText )
      {
         uint32_t uCount = 0; // counted characters in buffer
         const uint8_t* pubszPosition = pubszText;
         while( *pubszPosition != '\0' )
         {                                                                    assert( pNeededByteCount_s[*pubszPosition] != 0 );
            pubszPosition += pNeededByteCount_s[*pubszPosition];
            uCount++;

         }

         return std::pair<uint32_t, const uint8_t*>(uCount, pubszPosition);
      }

      /**
       * @brief count utf8 characters in buffer
       * @param pubszText pointer to buffer with text where characters are counted
       * @param pubszEnd pointer to end of buffer
       * @return number of utf8 characters in buffer
      */
      std::pair<uint32_t, const uint8_t*> count( const uint8_t* pubszText, const uint8_t* pubszEnd )
      {                                                                        assert( pubszText < pubszEnd ); assert( pubszEnd - pubszText < 0x00100000 );
         uint32_t uCount = 0; // counted characters in buffer
         const uint8_t* pubszPosition = pubszText;
         while( pubszPosition < pubszEnd )
         {
            pubszPosition += pNeededByteCount_s[*pubszPosition];
            uCount++;

         }

         return std::pair<uint32_t, const uint8_t*>(uCount, pubszPosition);
      }


      /** ---------------------------------------------------------------------
       * @brief get number of bytes needed in buffer to store utf8 for ascii character
       * @param uCharacter character  size is calculated for
       * @return uint32_t number of bytes needed to store ascii character as utf8 string
      */
      uint32_t size( uint8_t uCharacter ) 
      {
         if( uCharacter < 0x80 ) return 1;  
         return 2;
      }

      uint32_t size(uint16_t uCharacter)
      {
         if(uCharacter < 0x80) return 1;
         else if(uCharacter < 0x800) return 2;
         return 3;
      }

      /** ---------------------------------------------------------------------
       * @brief get number of bytes needed in buffer to store utf8 for utf32 character
       * @param uCharacter character size is calculated for utf32 character
       * @return uint32_t number of bytes needed to store utf32 character as utf8 string
      */
      uint32_t size(uint32_t uCharacter)
      {
         if(uCharacter < 0x80) return 1;
         else if(uCharacter < 0x800) return 2;
         else if(uCharacter < 0x10000) return 3;
         else if(uCharacter < 0x200000) return 4;
         return 5;
      }

      /**
       * @brief count needed size to store unicode character as utf8 text
       * @param uCharacter unicode character
       * @return number of bytes needed for buffer to store unicode character
      */
      uint32_t size(wchar_t uCharacter)
      {
         if( uCharacter < 0x80 ) return 1;
         else if( uCharacter < 0x800 ) return 2;
         return 3;
      }

      /**
       * @brief count needed buffer size to store ascii text
       * @param pbszAsciiText pointer to checked text for how big the buffer needs to be if stored in utf8 format
       * @param uLength length for ascii text that is checked
       * @return number of bytes needed in buffer to store ascii as utf8
      */
      uint32_t size( const char* pbszAsciiText, uint32_t uLength )
      {
         uint32_t uSize = 0;
         const char* pbszEnd = pbszAsciiText + uLength;
         while( pbszAsciiText < pbszEnd )
         {
            uSize += static_cast<uint8_t>(*pbszAsciiText) < 0x80 ? 1 : 2;
            pbszAsciiText++;
         }
         return uSize;
      }

      /**
       * @brief get needed size in bytes to store utf8 character
       * @param puCharacter pointer to utf8 character
       * @return byte count needed to store character
      */
      uint32_t get_character_size( const uint8_t* puCharacter ) {                                  assert( pNeededByteCount_s[ *puCharacter ] != 0 );
         return pNeededByteCount_s[ *puCharacter ];
      }

      /** ---------------------------------------------------------------------
       * @brief Validate utf8 sequence
       * @param pubBegin start of utf8 sequence to validate
       * @param pubEnd end of utf8 sequence
       * @return true if validated, false and position if error
      */
      std::pair<bool, const uint8_t*> validate( const uint8_t* pubBegin, const uint8_t* pubEnd )
      {                                                                                            assert( pubBegin <= pubEnd );
#ifdef _DEBUG
         const char* pbsz_d = (const char*)pubBegin;                           // simplify debugging
         const char8_t* putf8_d = (const char8_t*)pubBegin;                    // simplify debugging
#endif // DEBUG

         auto pubPosition = pubBegin;
         for( auto pubPosition = pubBegin; pubPosition != pubEnd; pubPosition++ )
         {
            if( *pubPosition < UTF8_MIN_ENCODE ) continue;
            else
            {
               bool bOk = true;
               std::size_t uLength = pNeededByteCount_s[*pubPosition];
               if( uLength > 0 )
               {
                  if( static_cast<std::size_t>(pubEnd - pubPosition) > uLength ) // do we have enough space
                  {
                     if( uLength == 2 )
                     {
                        if( (*pubPosition & 0b1110'0000) != 0b1100'0000 ) bOk = false;
                        pubPosition++;
                        if( (*pubPosition & UTF8_VALIDATE_TAIL_MASK) != UTF8_MIN_ENCODE ) bOk = false;
                     }
                     else if( uLength == 3 )
                     {
                        if( (*pubPosition & 0b1111'0000) != 0b1110'0000 ) bOk = false;
                        pubPosition++;
                        if( (*pubPosition & UTF8_VALIDATE_TAIL_MASK) != UTF8_MIN_ENCODE ) bOk = false;
                        pubPosition++;
                        if( (*pubPosition & UTF8_VALIDATE_TAIL_MASK) != UTF8_MIN_ENCODE ) bOk = false;
                     }
                     else if( uLength == 4 )
                     {
                        if( (*pubPosition & 0b1111'1000) != 0b1111'0000 ) bOk = false;
                        pubPosition++;
                        if( (*pubPosition & UTF8_VALIDATE_TAIL_MASK) != UTF8_MIN_ENCODE ) bOk = false;
                        pubPosition++;
                        if( (*pubPosition & UTF8_VALIDATE_TAIL_MASK) != UTF8_MIN_ENCODE ) bOk = false;
                        pubPosition++;
                        if( (*pubPosition & UTF8_VALIDATE_TAIL_MASK) != UTF8_MIN_ENCODE ) bOk = false;
                     }
                  }
                  else { bOk = false; }

                  if( bOk == false ) { return { false, pubPosition }; }
               }
               else { return { false, pubPosition }; }
            }
         }

         return { true, pubEnd };
      }

      /** ---------------------------------------------------------------------
       * @brief Validates string hexadecimal characters
       * @param pubBegin start of string
       * @param pubEnd end of string
       * @return true if ok, false an pointer to invalid character if error
      */
      std::pair<bool, const uint8_t*> validate_hex( const uint8_t* pubBegin, const uint8_t* pubEnd )
      {                                                                                            assert( pubBegin <= pubEnd ); assert( pubEnd - pubBegin < 0x1000'0000 );
         auto pubPosition = pubBegin;
         for( auto pubPosition = pubBegin; pubPosition != pubEnd; pubPosition++ )
         {
            if( pHexValidate_s[*pubPosition] != 0 ) continue;

            return  { false, pubPosition };
         }

         return { true, pubEnd };
      }

      uint32_t convert(uint8_t uCharacter, uint8_t* pbszTo)
      {
         if( uCharacter < 0x80 )
         {
            *pbszTo = static_cast<uint8_t>(uCharacter);
            return 1;
         }
         else
         {
            pbszTo[0] = static_cast<uint8_t>( 0xc0 | ((uCharacter >> 6) & 0x1f) );
            pbszTo[1] = static_cast<uint8_t>( 0x80 | (uCharacter & 0x3f) );
            return 2;
         }
         return 0;
      }



      /**
       * @brief Convert character to utf8
       * @param uCharacter character that is converted
       * @param pbszTo pointer to buffer that gets character information formated as utf8
       * @return buffer size used to store character in utf8
       */
      uint32_t convert( uint16_t uCharacter, uint8_t* pbszTo )
      {
         if( uCharacter < 0x80 )
         {
            *pbszTo = static_cast<uint8_t>(uCharacter);
            return 1;
         }
         else if( uCharacter < 0x800 )
         {
            pbszTo[0] = static_cast<uint8_t>( 0xc0 | ((uCharacter >> 6) & 0x1f) );
            pbszTo[1] = static_cast<uint8_t>( 0x80 | (uCharacter & 0x3f) );
            return 2;
         }
         return 0;
      }


      /** ---------------------------------------------------------------------
       * @brief Convert character to utf8
       * @param uCharacter character that is converted
       * @param pbszTo pointer to buffer that gets character information formated as utf8
       * @return buffer size used to store character in utf8
       */
      uint32_t convert( uint32_t uCharacter, uint8_t* pbszTo )
      {
         if( uCharacter < 0x80 )
         {
            *pbszTo = static_cast<uint8_t>(uCharacter);
            return 1;
         }
         else if( uCharacter < 0x800 )
         {
            pbszTo[0] = static_cast<uint8_t>( 0xc0 | ((uCharacter >> 6) & 0x1f) );
            pbszTo[1] = static_cast<uint8_t>( 0x80 | (uCharacter & 0x3f) );
            return 2;
         }
         else if( uCharacter < 0x10000 )
         {
            pbszTo[0] = static_cast<uint8_t>( 0xe0 | ((uCharacter >> 12) & 0x0f) );
            pbszTo[1] = static_cast<uint8_t>( 0x80 | ((uCharacter >> 6) & 0x3f) );
            pbszTo[2] = static_cast<uint8_t>( 0x80 | (uCharacter & 0x3f) );
            return 3;
         }
         else
         {
            pbszTo[0] = static_cast<uint8_t>( 0xf0 | ((uCharacter >> 18) & 0x07) );
            pbszTo[1] = static_cast<uint8_t>( 0x80 | ((uCharacter >> 12) & 0x3f) );
            pbszTo[2] = static_cast<uint8_t>( 0x80 | ((uCharacter >> 6) & 0x3f) );
            pbszTo[3] = static_cast<uint8_t>( 0x80 | (uCharacter & 0x3f) );
            return 4;
         }
         return 0;
      }

      /** ---------------------------------------------------------------------
       * @brief Convert character to utf8 and append it to string
       * @param uCharacter character that is converted
       * @param stringTo string getting character information formated as utf8
       * @return buffer size used to store character in utf8
       */
      uint32_t convert( uint32_t uCharacter, std::string& stringTo )
      {
         if( uCharacter < 0x80 )
         {
            stringTo += static_cast<char>(uCharacter);
            return 1;
         }
         else if( uCharacter < 0x800 )
         {
            stringTo += static_cast<char>( 0xc0 | ((uCharacter >> 6) & 0x1f) );
            stringTo += static_cast<char>( 0x80 | (uCharacter & 0x3f) );
            return 2;
         }
         else if( uCharacter < 0x10000 )
         {
            stringTo += static_cast<char>( 0xe0 | ((uCharacter >> 12) & 0x0f) );
            stringTo += static_cast<char>( 0x80 | ((uCharacter >> 6) & 0x3f) );
            stringTo += static_cast<char>( 0x80 | (uCharacter & 0x3f) );
            return 3;
         }
         else
         {
            stringTo += static_cast<char>( 0xf0 | ((uCharacter >> 18) & 0x07) );
            stringTo += static_cast<char>( 0x80 | ((uCharacter >> 12) & 0x3f) );
            stringTo += static_cast<char>( 0x80 | ((uCharacter >> 6) & 0x3f) );
            stringTo += static_cast<char>(  0x80 | (uCharacter & 0x3f) );
            return 4;
         }
         return 0;
      }


#if defined(__cpp_char8_t)
      /**
       * @brief convert utf16 to utf8
       * @param pwszUtf16 pointer to utf16 string
       * @param pbszUtf8 pointer to buffer where converted characters are stored
       * @return std::tuple<bool, const char16_t*, char8_t*> true if succeded, false if not. also returns positions where converted pointers ended
      */
      std::tuple<bool, const char16_t*, char8_t*> convert_utf16_to_uft8(const char16_t* pwszUtf16, char8_t* pbszUtf8)
      {
         const char16_t* pwszPosition = pwszUtf16;
         while(*pwszPosition)
         {
            uint32_t uCharacter = gd::utf16::character(pwszPosition);
            uint32_t uSize = convert(uCharacter, pbszUtf8);
            pwszPosition++;
            pbszUtf8 += uSize;
         }

         *pbszUtf8 = '\0';

         return { true, pwszPosition, pbszUtf8 };
      }
#endif

      /**
       * @brief convert utf16 to utf8
       * @param pwszUtf16 pointer to utf16 string
       * @param pbszUtf8 pointer to buffer where converted characters are stored
       * @return std::tuple<bool, const char16_t*, char8_t*> true if succeded, false if not. also returns positions where converted pointers ended
      */
      std::tuple<bool, const wchar_t*, char*> convert_utf16_to_uft8(const wchar_t* pwszUtf16, char* pbszUtf8, tag_utf8)
      {
         const wchar_t* pwszPosition = pwszUtf16;
         while(*pwszPosition)
         {
            uint32_t uCharacter = gd::utf16::character(pwszPosition);
            uint32_t uSize = convert(uCharacter, (uint8_t*)pbszUtf8);
            pwszPosition++;
            pbszUtf8 += uSize;
         }

         *pbszUtf8 = '\0';

         return { true, pwszPosition, pbszUtf8 };
      }


      std::tuple<bool, const uint16_t*> convert_utf16_to_uft8(const uint16_t* pwszUtf16, std::string& stringUtf8)
      {
         uint8_t puBuffer[SIZE32_MAX_UTF_SIZE + 1];
         const uint16_t* pwszPosition = pwszUtf16;
         while( *pwszPosition )
         {
            uint32_t uCharacter = gd::utf16::character(pwszPosition);
            uint32_t uSize = size(static_cast<uint16_t>(*pwszPosition));
            pwszPosition++;
            uSize = convert(uCharacter, puBuffer);
            stringUtf8.append(reinterpret_cast<const char*>(puBuffer), uSize);
         }

         return { true, pwszPosition };
      }

      /**
       * @brief convert utf8 text to wstring
       * @param pbszUtf8 points to test that will be converter to wstring
       * @param stringUtf16 string object getting converted text
       * @return pair with true if convert was ok and last position in buffer, otherwise false
      */
      std::tuple<bool, const uint8_t*> convert_utf8_to_uft16(const uint8_t* pbszUtf8, std::wstring& stringUtf16)
      {
         const uint8_t* pbszPosition = pbszUtf8;
         while( *pbszPosition )
         {
            uint32_t uCharacter = gd::utf8::character(pbszPosition);
            uint32_t uSize = size(static_cast<uint8_t>(*pbszPosition));
            pbszPosition += uSize;
            stringUtf16 += static_cast<wchar_t>(uCharacter);
         }

         return { true, pbszPosition };
      }




      /** ---------------------------------------------------------------------
       * @brief convert ascii text to utf8   
       * @param pbszFrom pointer to ascii text that will be converted
       * @param pbszTo pointer to buffer where utf8 characters are stored
       * @return {std::pair<bool, const uint8_t*>} true or false if all characters have been converted, and pointer to position where conversion ended
      */
      std::pair<bool, const uint8_t*> convert_ascii(const uint8_t* pbszFrom, uint8_t* pbszTo )
      {
         const uint8_t* pbszPosition = pbszFrom;
         uint8_t* pbszInsert = pbszTo;

         while(*pbszPosition)
         {
            if(*pbszPosition < 0x80) { *pbszInsert = *pbszPosition; pbszInsert++; }
            else
            {
               pbszInsert[0] = (0xc0 | ((*pbszPosition >> 6) & 0x1f));
               pbszInsert[1] = (0x80 | (*pbszPosition & 0x3f));
               pbszInsert += 2;
            }

            pbszPosition++;
         }

         *pbszInsert = '\0';

         return std::make_pair(true, pbszPosition);
      }


      /** ---------------------------------------------------------------------
       * @brief convert ascii text to utf8   
       * @param pbszFrom pointer to ascii text that will be converted
       * @param pbszTo pointer to buffer where utf8 characters are stored
       * @param pbszEnd end of buffer
       * @return {std::pair<bool, const uint8_t*>} true or false if all characters have been converted, and pointer to position where conversion ended
      */
      std::pair<bool, const uint8_t*> convert_ascii(const uint8_t* pbszFrom, uint8_t* pbszTo, const uint8_t* pbszEnd)
      {
         const uint8_t* pbszPosition = pbszFrom;
         uint8_t* pbszInsert = pbszTo;

         pbszEnd--; // reserve one character in buffer, some ascii characters will need two bytes
         while(pbszInsert < pbszEnd && *pbszPosition)
         {
            if(*pbszPosition < 0x80) { *pbszInsert = *pbszPosition; pbszInsert++; }
            else
            {
               pbszInsert[0] = (0xc0 | ((*pbszPosition >> 6) & 0x1f));
               pbszInsert[1] = (0x80 | (*pbszPosition & 0x3f));
               pbszInsert += 2;
            }

            pbszPosition++;
         }

         if(pbszInsert < pbszEnd) *pbszInsert = '\0';

         return std::make_pair(true, pbszPosition);
      }

      /** ---------------------------------------------------------------------
       * @brief convert ascii char string to string as utf8
       * @param pbsBegin pointer to ascii text to convert
       * @param pbsEnd end of ascii text to convert
       * @param stringTo string getting the converted text
      */
      void convert_ascii( const char* pbsBegin, const char* pbsEnd, std::string& stringTo )
      {
         uint8_t pbUtf8[2];
         for( const char* pbsPosition = pbsBegin; pbsPosition != pbsEnd; pbsPosition++ )
         {
            if( (uint8_t)*pbsPosition < UTF8_MIN_ENCODE ) stringTo += *pbsPosition;
            else
            {
               convert( (const uint8_t)*pbsPosition, pbUtf8 );
               stringTo.append( (const char*)pbUtf8, 2 );
            }
         }
      }

      /** ---------------------------------------------------------------------
       * @brief convert ascii char string to utf8 buffer
       * @param pbsBegin pointer to ascii text to convert
       * @param pbsEnd end of ascii text to convert
       * @param puTo pointer to buffer where converted ascii characters are placed
       * @param puEnd last position for buffer where converted ascii characters are placed
      */
      void convert_ascii( const char* pbsBegin, const char* pbsEnd, uint8_t* puTo, const uint8_t* puEnd )
      {
         for( const char* pbsPosition = pbsBegin; pbsPosition != pbsEnd && puTo < puEnd; pbsPosition++ )
         {
            if( ( uint8_t )*pbsPosition < UTF8_MIN_ENCODE )
            {
               *puTo = *pbsPosition;
               puTo++;
            }
            else
            {
               convert( (const uint8_t)*pbsPosition, puTo );
               puTo += 2;                                                                          assert( puTo <= puEnd ); // check buffer barrier
            }
         }
      }

      /** ---------------------------------------------------------------------
       * @brief convert ascii char string to utf8 buffer
       * @param puBegin pointer to ascii text to convert
       * @param puEnd end of ascii text to convert
       * @param pbszTo pointer to buffer where converted ascii characters are placed
       * @param pbszEnd last position for buffer where converted ascii characters are placed
      */
      std::pair<bool, const uint8_t*> convert_utf8_to_ascii( const uint8_t* puBegin, const uint8_t* puEnd, char* pbszTo, const char* pbszEnd )
      {
         const uint8_t* puPosition = puBegin;
         for( ; puPosition != puEnd && pbszTo < pbszEnd; puPosition++ )
         {
            auto uCount = get_character_size( puPosition );                                        assert( uCount != 0 );
            if( uCount == 1 )
            {
               *pbszTo = *puPosition;
            }
            else if( uCount == 2 )
            {
               auto uCharacter = character( puPosition );                                          assert( uCharacter < 256 );
               *pbszTo = (char)(uint8_t)uCharacter;
               puPosition++;
            }
            else {  assert( false ); }

            pbszTo++;
         }

         return { true, puPosition };
      }

      /** ---------------------------------------------------------------------
       * @brief Convert string with text in utf8 format to ascii
       * Note: this method do not work if utf8 characters contains ascii character
       * above 255
       * @param stringUtf8 string in utf8 format
       * @return std::string string in ascii format
       */
      std::string convert_utf8_to_ascii(const std::string_view& stringUtf8)
      {
         std::string stringAscii;

         const uint8_t* puBegin = (const uint8_t*)stringUtf8.data();
         const uint8_t* puEnd = (const uint8_t*)stringUtf8.data() + stringUtf8.length();
         for( auto puPosition = puBegin; puPosition != puEnd; puPosition++ )
         {
            auto uCount = get_character_size( puPosition );                                        assert( uCount != 0 );
            if( uCount == 1 )
            {
               stringAscii += (char)*puPosition;
            }
            else if( uCount == 2 )
            {
               auto uCharacter = character( puPosition );                                          assert( uCharacter < 256 );
               stringAscii += (char)(uint8_t)uCharacter;
               puPosition++;
            }
            else {  assert( false ); }
         }


         return stringAscii;
      }


      /** ---------------------------------------------------------------------
       * @brief convert unicode text to utf8   
       * @param pwszFrom pointer to unicode text that will be converted
       * @param pbszTo pointer to buffer where utf8 characters are stored
       * @param pbszEnd end of buffer
       * @return {std::pair<bool, const uint8_t*>} true or false if all characters have been converted, and pointer to position where conversion ended
      */
      std::pair<bool, const uint16_t*> convert_unicode(const uint16_t* pwszFrom, uint8_t* pbszTo, const uint8_t* pbszEnd)
      {
         const uint16_t* pwszPosition = pwszFrom;
         uint8_t* pbszInsert = pbszTo;

         while(pbszInsert < pbszEnd && *pwszPosition)
         {
            if(*pwszPosition < 0x80) { *pbszInsert = static_cast<uint8_t>(*pwszPosition); pbszInsert++; }
            else if( (*pwszPosition & 0xf800) == 0 )
            {
               pbszInsert[0] = (0xc0 | (*pwszPosition >> 6));
               pbszInsert[1] = (0x80 | (*pwszPosition & 0x3f));
               pbszInsert += 2;
            }
            else 
            {
               pbszInsert[0] = (0xE0 | (*pwszPosition >> 12));
               pbszInsert[1] = (0x80 | ((*pwszPosition >> 6) & 0x3F));
               pbszInsert[2] = (0x80 | (*pwszPosition & 0x3F));
               pbszInsert += 3;
            }

            if constexpr( sizeof( wchar_t ) == 2 ) { pwszPosition++; }
            else                                   { pwszPosition += 2; }
         }

         if(pbszInsert < pbszEnd) *pbszInsert = '\0';

         return std::make_pair(true, pwszPosition);
      }

      /**
       * @brief convert unicode to ascii
       * @param pwszFrom pointer to unicode text that is converted to ascii
       * @param pbszTo pointer to ascii buffer getting converted unicode text
       * @param pbszEnd pointer to end of buffer 
       * @return std::pair<const wchar_t*, const char*> pointer to end after last unicode character copied and end after last char getting ascii code
      */
      std::pair<const wchar_t*, const char*> convert_unicode_to_ascii(const wchar_t* pwszFrom, char* pbszTo, const char* pbszEnd)
      {
         const wchar_t* pwszPosition = pwszFrom;
         char* pbszInsert = pbszTo;

         while( pbszInsert < pbszEnd && *pwszPosition )
         {
            if( *pwszPosition <= 0xFF )
            {
               *(uint8_t*)pbszInsert = (uint8_t)*pwszPosition;
               pbszInsert++;
            }

            pwszPosition++;
         }

         if(pbszInsert < pbszEnd) *pbszInsert = '\0';

         return { pwszPosition, pbszInsert };
      }

      /**
       * @brief convert unicode to ascii and return as string
       * @param stringUnicode unicode string to convert
       * @return ascii string with converted unicode characters
      */
      std::string convert_unicode_to_ascii(std::wstring_view stringUnicode)
      {
         std::string stringAscii;
         auto uLength = stringUnicode.length() + 1;
         stringAscii.resize(stringUnicode.length() + 1);
         auto [pwszUnicodeEnd, pbszAsciiEnd] = gd::utf8::convert_unicode_to_ascii(stringUnicode.data(), stringAscii.data(), stringAscii.data() + uLength);
         stringAscii.resize(pbszAsciiEnd - stringAscii.c_str());
         return stringAscii;
      }


      /**
       * @brief convert ascii to unicode
       * @param pszFrom pointer to ascii text that is converted to unicode
       * @param pwszTo pointer to unicode buffer getting converted ascii text
       * @param pwszEnd pointer to end of buffer 
       * @return std::pair<const char*, const wchar_t*> pointer to end after last ascii character copied and end after last char getting unicode code
      */
      std::pair<const char*, const wchar_t*> convert_ascii_to_unicode(const char* pszFrom, wchar_t* pwszTo, const wchar_t* pwszEnd)
      {
         const char* pbszPosition = pszFrom;
         wchar_t* pwszInsert = pwszTo;

         while( pwszInsert < pwszEnd && *pbszPosition )
         {
            *pwszInsert = *pbszPosition;
            pwszInsert++;
            pbszPosition++;
         }

         if( pwszInsert < pwszEnd ) *pwszInsert = '\0';

         return { pbszPosition, pwszInsert };
      }


      /** ---------------------------------------------------------------------
       * @brief convert ascii to unicode and return as string
       * @param stringAscii ascii string to convert
       * @return unicode string with converted ascii characters
       */
      std::wstring convert_ascii_to_unicode(std::string_view stringAscii)
      {
         std::wstring stringUnicode;
         auto uLength = stringAscii.length() + 1;
         stringUnicode.resize(stringAscii.length() + 1);
         auto [pwszUnicodeEnd, pbszAsciiEnd] = gd::utf8::convert_ascii_to_unicode(stringAscii.data(), stringUnicode.data(), stringUnicode.data() + uLength);
         stringUnicode.resize(pbszAsciiEnd - stringUnicode.c_str());
         return stringUnicode;
      }

      /** ---------------------------------------------------------------------
       * @brief convert json escaped string to utf8
       * @param puFrom escaped json string start
       * @param puEnd escaped json sting end
       * @param puTo buffer where converted characters are written to
       * @return true and end position of buffer where characters have been written to, or false if error
       */
      std::pair<bool, const uint8_t*> convert_json(const uint8_t* puFrom, const uint8_t* puEnd, uint8_t* puTo)
      {
         if( puEnd == nullptr ) { puEnd = puFrom +  strlen( puFrom ); }

         const auto* puPosition = puFrom;
         
         while( puPosition < puEnd )
         {
            uint32_t uCharacter = json::character( puPosition );
            puTo += convert( uCharacter, puTo );
            puPosition= json::next( puPosition );
         }
         
         *puTo = '\0';

         return { true, puTo };
      }

      /** ---------------------------------------------------------------------
       * @brief convert json escaped string to utf8 formated string
       * @param stringJson json escaped string
       * @return 
       */
      std::string convert_json(const std::string_view& stringJson)
      {
         std::string stringUtf8;
         const char* pbPosition = stringJson.data();
         const char* pbEnd = pbPosition + stringJson.length();

         while( pbPosition < pbEnd )
         {
            uint32_t uCharacter = json::character( pbPosition );
            convert( uCharacter, stringUtf8 );
            pbPosition = json::next( pbPosition );
         }

         return stringUtf8;
      }


      /**
       * @brief convert char buffer to unsigned interger value
       * @param puNumber pointer to buffer that is converted to number
       * @return uint32_t number for buffer
       */
      uint32_t atou(const uint8_t* puNumber)
      {  
         using namespace gd::types;
         uint32_t uNumber = 0;
         while(ctype_g(*puNumber, tag_main_type{}) == CHAR_TYPE_DIGIT)
         {
            uNumber = uNumber * 10 + (*puNumber - '0');
            puNumber++;
         }
         return uNumber;
      }

      /** ---------------------------------------------------------------------
       * @brief convert char buffer to unsigned interger value
       * @param puBegin start of text to convert
       * @param puEnd end of text to convert
       * @return uint32_t number for buffer
       */
      uint32_t atou(const uint8_t* puBegin, const uint8_t* puEnd)
      {
         using namespace gd::types;
         uint32_t uNumber = 0;
         while( puBegin < puEnd && ctype_g(*puBegin, tag_main_type{}) == CHAR_TYPE_DIGIT)
         {
            uNumber = uNumber * 10 + (*puBegin - '0');
            puBegin++;
         }
         return uNumber;
      }

      /** ---------------------------------------------------------------------
       * @brief find first digit and convert to number and return, no digit will return 0
       * @param puNumber pointer to buffer that is converted to number
       * @return uint32_t number for buffer
       */
      uint32_t atou(const uint8_t* puNumber, tag_find)
      {
         using namespace gd::types;
         while(*puNumber != '\0' && ctype_g(*puNumber, tag_main_type{}) != CHAR_TYPE_DIGIT) puNumber++;
         uint32_t uNumber = 0;
         if( *puNumber != '\0' ) { uNumber = atou( puNumber ); }
         return uNumber;
      }

      /** ---------------------------------------------------------------------
       * @brief find first digit and convert to number and return, no digit will return 0
       * @param puBegin start of text to convert
       * @param puEnd end of text to convert
       * @return uint32_t number for buffer
       */
      uint32_t atou(const uint8_t* puBegin, const uint8_t* puEnd, tag_find)
      {
         using namespace gd::types;
         while(puBegin <  puEnd && ctype_g(*puBegin, tag_main_type{}) != CHAR_TYPE_DIGIT) puBegin++;
         uint32_t uNumber = 0;
         if( puBegin < puEnd ) { uNumber = atou( puBegin, puEnd ); }
         return uNumber;
      }

      /**
       * @brief convert char buffer to unsigned interger value
       * @param puNumber pointer to buffer that is converted to number
       * @return uint64_t number for buffer
       */
      uint64_t atou64(const uint8_t* puNumber)
      {  
         using namespace gd::types;
         uint64_t uNumber = 0;
         while(ctype_g(*puNumber, tag_main_type{}) == CHAR_TYPE_DIGIT)
         {
            uNumber = uNumber * 10 + (*puNumber - '0');
            puNumber++;
         }

         return uNumber;
      }

      /** ---------------------------------------------------------------------
       * @brief convert char buffer to unsigned interger value
       * @param puBegin start of text to convert
       * @param puEnd end of text to convert
       * @return uint32_t number for buffer
       */
      uint64_t atou64(const uint8_t* puBegin, const uint8_t* puEnd)
      {
         using namespace gd::types;
         uint64_t uNumber = 0;
         while( puBegin < puEnd && ctype_g(*puBegin, tag_main_type{}) == CHAR_TYPE_DIGIT)
         {
            uNumber = uNumber * 10 + (*puBegin - '0');
            puBegin++;
         }
         return uNumber;
      }



      /// 
      static const uint8_t puDigits_s[200] = {
         '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',          // 000 - 019
         '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',          // 020 - 039
         '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',          // 040 - 059
         '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',          // 060 - 079
         '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',          // 080 - 099
         '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',          // 100 - 119
         '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',          // 120 - 139
         '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',          // 140 - 159
         '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',          // 160 - 179
         '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'           // 180 - 199
      };

      /**
       * @brief convert integer to text
       * Numbers are placed in buffer as text
       * @param iNumber integer number converted to text
       * @param pbszTo buffer that gets text, make sure it is big enough
       * @return uint8_t* pointer to last position (the `\0`character)
       */
      uint8_t* itoa( int32_t iNumber, uint8_t* pbszTo )
      {
         uint32_t uNumber = static_cast<uint32_t>(iNumber);
         if(iNumber < 0) 
         {
            *pbszTo++ = '-';                                                  // add negative sign for text because value is below 0
            uNumber = ~uNumber + 1;                                           // invert and set to proper value after -
         }
         return utoa( uNumber, pbszTo );                                      // convert number
      }

      /**
       * @brief convert unsigned integer to text
       * Numbers are placed in buffer as text
       * @param uNumber unsigned integer number converted to text
       * @param pbszTo buffer that gets text, make sure it is big enough
       * @return uint8_t* pointer to last position (the `\0`character)
       */
      uint8_t* utoa( uint32_t uNumber, uint8_t* pbszTo )
      {
         uint8_t pbBuffer[10]; // buffer used to set values in reverse order
         uint8_t* pu = pbBuffer;

         while(uNumber >= 100)
         {
            const unsigned u = (uNumber % 100) << 1;
            uNumber /= 100;
            *pu++ = puDigits_s[u + 1];
            *pu++ = puDigits_s[u];
         }

         if(uNumber < 10) *pu++ = uint8_t(uNumber) + '0';
         else 
         {
            const unsigned u = uNumber << 1;
            *pu++ = puDigits_s[u + 1];
            *pu++ = puDigits_s[u];
         }
                                                                         
         do { *pbszTo++ = *--pu; } while(pu != pbBuffer);                     // copy buffer in reverse order

         *pbszTo = '\0';
         return pbszTo;
      }

      /** ---------------------------------------------------------------------
       * @brief convert integer to text
       * Numbers are placed in buffer as text
       * @param uCharacter integer number converted to text
       * @param pbszTo buffer that gets text, make sure it is big enough
       * @return uint8_t* pointer to last position (the `\0`character)
       */
      uint8_t* itoa( int64_t iNumber, uint8_t* pbszTo )
      {
         uint64_t uNumber = static_cast<uint64_t>(iNumber);
         if(iNumber < 0) 
         {
            *pbszTo++ = '-';                                                  // add negative sign for text because value is below 0
            uNumber = ~uNumber + 1;                                           // invert and set to proper value after -
         }
         return utoa( uNumber, pbszTo );                                      // convert number
      }

      /** ---------------------------------------------------------------------
       * @brief convert unsigned integer 64 to text
       * Numbers are placed in buffer as text
       * @param uCharacter 
       * @param pbszTo buffer that gets text, make sure it is big enough
       * @return uint8_t* pointer to last position (the `\0`character)
       */
      uint8_t* utoa( uint64_t uNumber, uint8_t* pbszTo ) 
      {
         uint8_t pbBuffer[20]; // buffer used to set values in reverse order
         uint8_t* pu = pbBuffer;
         while(uNumber >= 100)
         {
            const unsigned u = (uNumber % 100) << 1;
            uNumber /= 100;
            *pu++ = puDigits_s[u + 1];
            *pu++ = puDigits_s[u];
         }

         if(uNumber < 10) *pu++ = uint8_t(uNumber) + '0';
         else 
         {
            const uint32_t u = (uint32_t)uNumber << 1;
            *pu++ = puDigits_s[u + 1];
            *pu++ = puDigits_s[u];
         }
                                                                         
         do { *pbszTo++ = *--pu; } while(pu != pbBuffer);                     // copy buffer in reverse order

         *pbszTo = '\0';
         return pbszTo;
      }

      /** ---------------------------------------------------------------------
       * @brief converts hex string to binary data
       * @param puFrom start of string to convert
       * @param puTo string end
       * @param puBinary buffer getting converted bytes
       * @return true and pointer to last converted byte
      */
      std::pair<bool, uint8_t*> convert_hex_to_binary( const uint8_t* puFrom, const uint8_t* puTo, uint8_t* puBinary )
      {                                                                                            assert( ((puTo - puFrom) % 2) == 0 );// has to be even number (hex byte value is two hexadecimal characters)
         uint8_t* puPosition = puBinary;
         for( auto itChar = puFrom; itChar < puTo; itChar += 2, puPosition++ )
         {
            uint8_t uHexValue = pHexValue_s[*itChar] * 16 + pHexValue_s[*(itChar + 1)];// converts hex sequence to byte
            *puPosition = uHexValue;                                           // set byte
         }
         puPosition++;

         return { true, puPosition };
      }

       /**
       * @brief copy utf8 character into buffoer
       * @param puCopyTo buffer getting character
       * @param puCopyFrom buffer character is read from
       * @return uint8_t* position in buffer character is copied to
       */
      uint8_t* copy_character(uint8_t* puCopyTo, const uint8_t* puCopyFrom)
      {                                                                       assert( pNeededByteCount_s[*puCopyFrom] != 0 );
         auto uCount = pNeededByteCount_s[*puCopyFrom];
         while( uCount-- )
         {
            *puCopyTo = *puCopyFrom;
            puCopyTo++;
            puCopyFrom++;
         }
         return puCopyTo;
      }

      /**
       * @brief calculate distance in utf8 characters between two positions in a stream of utf8 characters
       * @param p1 first position in utf8 stream
       * @param p2 second position in utf8 stream
       * @return distance in number of utf8 characters
       */
      std::intptr_t distance(const uint8_t* p1, const uint8_t* p2)
      {
         std::intptr_t iCount = 0;
         auto pFrom = p1 < p2 ? p1 : p2;
         auto pTo = p1 >= p2 ? p1 : p2;

         while(pFrom != pTo) {
            pFrom = move::next(pFrom);
            iCount++;
         }

         return p1 < p2 ? iCount : -iCount;
      }
   } // utf8 
} // gd


namespace gd {
   namespace utf8 {
      namespace move {

         /**
          * @brief move to next character in utf8 buffer
          * @param pubszPosition pointer to position where movement starts
          * @return {const uint8_t*} new position
          */
         const uint8_t* next(const uint8_t* pubszPosition)
         {
            if(*pubszPosition != '\0')
            {
               if((*pubszPosition & 0x80) == 0)          return pubszPosition + 1;
               else if((*pubszPosition & 0xc0) == 0xc0)  return pubszPosition + 2;
               else if((*pubszPosition & 0xf0) == 0xe0)  return pubszPosition + 3;
               else if((*pubszPosition & 0xf8) == 0xf0)  return pubszPosition + 4;
               else throw std::runtime_error("invalid UTF-8 (operation = next)");
            }

            return pubszPosition;
         }

         /**
          * @brief Move forward in text
          * @param pubszPosition pointer to current position in text
          * @param uCount number of characters to move
          * @return pointer to new position
          */
         const uint8_t* next( const uint8_t* pubszPosition, uint32_t uCount )
         {
            while(uCount-- > 0) pubszPosition = next( pubszPosition );
            return pubszPosition;
         }

         /**
          * @brief move pointer in utf-8 text
          * @param ppubszPosition pointer to pointer that is moved in text
          * @param uCount how much movement
          * @return {bool} true if pointer was moved, false if not
          */
         bool next(const uint8_t** ppubszPosition, uint32_t uCount) {
            auto pubszPosition = *ppubszPosition;
            
            while(uCount-- > 0)
            {
               auto pubszSave = pubszPosition;
               pubszPosition = next(pubszPosition);
               if(pubszSave == pubszPosition) return false;
               pubszSave = pubszPosition;
            }

            *ppubszPosition = pubszPosition;
            return true;

         }

         /**
          * @brief move pointer to next space
          * @param pubszPosition pointer to string where first space is searched for
          * @return {const uint8_t*} position to next position in text where space is found or end of string
          */
         const uint8_t* next_space(const uint8_t* pubszPosition)
         {
            const uint8_t* pubzNext = pubszPosition;
            do
            {
               pubszPosition = pubzNext;
               if( *pubszPosition <= CHARACTER_SPACE )
               {
                  if( *pubszPosition == CHARACTER_SPACE || *pubszPosition == CHARACTER_TAB || *pubszPosition == CHARACTER_LINEFEED || *pubszPosition == CHARACTER_CARRAGERETURN ) return pubszPosition;
               }
               pubzNext = next( pubszPosition );
            } while( pubzNext != pubszPosition );

            return pubzNext;
         }

         /**
          * @brief move pointer to next non space character
          * @param pubszPosition pointer to string where first non space is searched for
          * @return {const uint8_t*} position to next position in text where non space is found or end of string
          */
         const uint8_t* next_non_space(const uint8_t* pubszPosition)
         {
            const uint8_t* pubNext = pubszPosition;
            do
            {
               pubszPosition = pubNext;
               if( *pubszPosition > CHARACTER_SPACE ) return pubszPosition;
               pubNext = next( pubszPosition );
            } while( pubNext != pubszPosition );

            return pubNext;
         }

         /**
          * @brief move pointer to next non space character
          * @param pubPosition pointer to string where first non space is searched for
          * @param pubEnd end pointer, stop search for non space if passing this
          * @return {const uint8_t*} position to next position in text where non space is found or end of string
          */
         const uint8_t* next_non_space(const uint8_t* pubPosition, const uint8_t* pubEnd )
         {
            const uint8_t* pubNext = pubPosition;
            while( pubPosition < pubEnd )
            {
               if( *pubPosition > CHARACTER_SPACE ) return pubPosition;
               pubNext = next( pubPosition );
            }
            return pubPosition;
         }

         /**
          * @brief Move to previous character in utf8 buffer
          * @param pubszPosition pointer to position where movement starts
          * @return {const uint8_t*} new position
          */
         const uint8_t* previous(const uint8_t* pubszPosition)
         {
            auto pubszTest = pubszPosition - 1;
            if((*pubszTest & 0x80) == 0)                return pubszPosition - 1;
            else if((*(pubszTest - 1) & 0xc0) == 0xc0)  return pubszPosition - 2;
            else if((*(pubszTest - 2) & 0xf0) == 0xe0)  return pubszPosition - 3;
            else if((*(pubszTest - 3) & 0xf8) == 0xf0)  return pubszPosition - 4;
            else throw std::runtime_error("invalid UTF-8 (operation = previous)");

            return pubszPosition;
         }

         /**
          * @brief move backwards in text
          * @param pubszPosition pointer to current position in text 
          * @param uCount number of characters to move
          * @return {const uint8_t*} pointer to new position
          */
         const uint8_t* previous(const uint8_t* pubszPosition, uint32_t uCount)
         {
            while(uCount-- > 0) pubszPosition = previous(pubszPosition);
            return pubszPosition;
         }


         /**
          * @brief move to end of utf8 text
          * @param pubszPosition 
          * @return {const uint8_t*} pointer to utf8 text end
          */
         const uint8_t* end(const uint8_t* pubszPosition)
         {
            while(*pubszPosition != '\0') pubszPosition++;
            return pubszPosition;
         }

         const uint8_t* find( const uint8_t* pubszPosition, uint32_t uCharacter )
         {
            uint8_t puBuffer[SIZE32_MAX_UTF_SIZE + 1];
            auto uLength = gd::utf8::convert( uCharacter, puBuffer );
            puBuffer[uLength] = '\0';

            return find_character( pubszPosition, puBuffer, uLength );
         }

         /**
          * @brief converts character to utf8 sequence and tries to find it within range
          * @param pubszPosition start position to start search
          * @param pubszEnd end position end search
          * @param uCharacter character to search for
          * @return position where character was found or nullptr if not found
         */
         const uint8_t* find( const uint8_t* pubszPosition, const uint8_t* pubszEnd, uint32_t uCharacter )
         {
            uint8_t puBuffer[SIZE32_MAX_UTF_SIZE + 1];
            auto uLength = gd::utf8::convert( uCharacter, puBuffer );
            puBuffer[uLength] = '\0';

            return find_character( pubszPosition, pubszEnd, puBuffer, uLength );
         }

         /**
          * @brief finds character where utf8 character length is specified. 
          * This method is optimized to avoid to calculate the utf8 character length befor trying to find character
          * @param pubszPosition start position to start search
          * @param pubszCharacter character to find
          * @param ulength character length
          * @return 
         */
         const uint8_t* find_character( const uint8_t* pubszPosition, const uint8_t* pubszCharacter, uint32_t ulength ) 
         {                                                                                         assert( ulength < 6 );
            auto pubszFind = pubszPosition;
            while( *pubszFind != '\0' )
            {
               if( *pubszFind == *pubszCharacter )
               {
                  switch( ulength )
                  {
                     case 1: return pubszFind;
                     case 2: if( pubszFind[1] == pubszCharacter[1] ) return pubszFind;
                        break;
                     case 3: if( pubszFind[1] == pubszCharacter[1] && pubszFind[2] == pubszCharacter[2] ) return pubszFind;
                        break;
                     case 4: if( pubszFind[1] == pubszCharacter[1] && pubszFind[2] == pubszCharacter[2] && pubszFind[3] == pubszCharacter[3] ) return pubszFind;
                        break;
                     case 5: if( pubszFind[1] == pubszCharacter[1] && pubszFind[2] == pubszCharacter[2] && pubszFind[3] == pubszCharacter[3] && pubszFind[4] == pubszCharacter[4] ) return pubszFind;
                        break;
                     default: throw std::runtime_error("invalid UTF-8 (operation = find_character)");
                  }
               }

               pubszFind++;
            }

            return nullptr;
         }

         /**
          * @brief find string in text
          * @param pubszPosition text that string is searched for
          * @param pubszEnd end of text
          * @param pubszFind string to find
          * @param uSize length of string to find
          * @return 
         */
         const uint8_t* find( const uint8_t* pubszPosition, const uint8_t* pubszEnd, const uint8_t* pubszFind, uint32_t uSize )
         {
            pubszEnd -= uSize;
            uSize--;
            while( pubszPosition < pubszEnd )
            {
               if( *pubszPosition == *pubszFind )
               {
                  if( memcmp( pubszPosition + 1, pubszFind + 1, uSize ) == 0  ) return pubszPosition;
               }

               pubszPosition++;
            }

            return nullptr;
         }


         /**
          * @brief Find utf8 character sequence in buffer
          * @param pubszFind text to look for utf8 character
          * @param pubszEnd where to stop searching
          * @param pubszCharacter character to look for, remember that this need to be a utf8 sequence
          * @param uLength utf8 sequence length
          * @return position to found character or if not found return null
         */
         const uint8_t* find_character( const uint8_t* pubszFind, const uint8_t* pubszEnd, const uint8_t* pubszCharacter, uint32_t uLength ) 
         {                                                                                         assert( uLength < 6 );
            while( pubszFind < pubszEnd )
            {
               if( *pubszFind == *pubszCharacter )
               {
                  switch( uLength )
                  {
                     case 1: return pubszFind;
                     case 2: if( pubszFind[1] == pubszCharacter[1] ) return pubszFind;
                        break;
                     case 3: if( pubszFind[1] == pubszCharacter[1] && pubszFind[2] == pubszCharacter[2] ) return pubszFind;
                        break;
                     case 4: if( pubszFind[1] == pubszCharacter[1] && pubszFind[2] == pubszCharacter[2] && pubszFind[3] == pubszCharacter[3] ) return pubszFind;
                        break;
                     case 5: if( pubszFind[1] == pubszCharacter[1] && pubszFind[2] == pubszCharacter[2] && pubszFind[3] == pubszCharacter[3] && pubszFind[4] == pubszCharacter[4] ) return pubszFind;
                        break;
                     default: throw std::runtime_error("invalid UTF-8 (operation = find_character)");
                  }
               }

               pubszFind++;
            }

            return nullptr;
         }


         
      } // move

      /** ---------------------------------------------------------------------
       * @brief methods used to convert text from browser that is formatted to be 
       * converted into utf8
       */
      namespace json {

         /** ------------------------------------------------------------------
          * @brief count number of characters in buffer formated as json text
          * @param pubszText pointer to buffer with text where characters are counted
          * @param pubszEnd pointer to end of buffer
          * @return number of characters in buffer
         */
         /*
         std::pair<uint32_t, const uint8_t*> count( const uint8_t* pubszText, const uint8_t* pubszEnd )
         {
            uint32_t uCount = 0; // counted characters in buffer
            const uint8_t* pubszPosition = pubszText;
            while( pubszPosition != pubszEnd )
            {                                                                                      assert( pubszPosition < pubszEnd );
               if( *pubszPosition == '\\' )                                    // if \ then skip char
               {
                  pubszPosition++;
                  continue;
               }

               pubszPosition++;
               uCount++;
            }

            return std::pair<uint32_t, const uint8_t*>(uCount, pubszPosition);
         }
         */


         /** ------------------------------------------------------------------
          * @brief Check if character need to be encoded when sent as json formated text
          * @param uChar ascii character
          * @return true if character is encoded, false if not
         */
         inline bool is_encoded( uint8_t uChar )
         {
            return pIsJsonEscape_s[uChar] != 0;
         }

         /** ------------------------------------------------------------------
          * @brief Validate json formated text
          * Checks for non valid characters within json formated text
          * @param pubBegin start of json formated text to validate
          * @param pubEnd  end of json formated text to validate
          * @return 
         */
         std::pair<bool, const uint8_t*> validate( const uint8_t* pubBegin, const uint8_t* pubEnd )
         {
            auto pubPosition = pubBegin;
            for( auto pubPosition = pubBegin; pubPosition != pubEnd; pubPosition++ )
            {
               // check if character should be encoded but is not
               if( is_encoded( *pubPosition ) == true ) return {false, pubPosition};// return error, this character is not valid within uri encoded text
            }

            return { true, pubEnd };
         }


         /** ------------------------------------------------------------------
          * @brief get character value for json character
          * Format for json character are \u0000 if unicode or just character if not unicode.
          * This format is used in json format
          * Sample: `\uC383` = O
          * @param pubszCharacter uri character to convert
          * @return uint32_t number for character
         */
         uint32_t character( const uint8_t* pubszCharacter, bool* pbUnicode )
         {
            uint32_t uCharacter = 0;
            if( *pubszCharacter != '\\' ) uCharacter = *pubszCharacter;
            else
            {
               pubszCharacter++;
               if( *pubszCharacter == 'u' )                                    // check for "\u"
               {                                                               // format should be \u + hex + hex (sample: \u0020 = 32 = space)
                  *pbUnicode = true;
                  pubszCharacter++;
                  uCharacter = (pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)];
                  pubszCharacter += 2;

                  uCharacter <<= 16;

                  uCharacter += (pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)];
                  pubszCharacter += 2;
               }
               else
               {
                  *pbUnicode = false;
                  uCharacter = *pubszCharacter;
               }
            }


            return uCharacter;
         }

         /** ------------------------------------------------------------------
          * @brief get the character in utf32 format, format is escaped json
          * @param pubszCharacter position where character is
          * @return uint32_t value for character
          */
         inline uint32_t character( const uint8_t* pubszCharacter )
         {
            uint32_t uCharacter = 0;
            if( *pubszCharacter != '\\' ) uCharacter = *pubszCharacter;
            else
            {
               pubszCharacter++;
               if( *pubszCharacter == 'u' )                                    // check for "\u"
               {                                                               // format should be \u + hex + hex (sample: \u0020 = 32 = space)
                  pubszCharacter++;
                  uCharacter = (pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)];
                  pubszCharacter += 2;

                  uCharacter <<= 16;

                  uCharacter += (pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)];
                  pubszCharacter += 2;
               }
               else
               {
                  uCharacter = *pubszCharacter;
               }
            }

            return uCharacter;
         }


         /** ------------------------------------------------------------------
          * @brief Count number of characters
          * Counts until zero ending
          * @param pubszText text that is counted
          * @return uint32_t number of characters
          */
         uint32_t count( const uint8_t* pubszText )
         {
            uint32_t uCount = 0;
            while( *pubszText != '\0' )
            {
               pubszText = next( pubszText );
               uCount++;
            }
            return uCount;
         }

         /** ------------------------------------------------------------------
          * @brief Count number of characters
          * Counts characters until end is reached
          * @param pubszBegin start of text that is counted
          * @param pubszEnd count until this position is reached
          * @return uint32_t number of characters
          */
         uint32_t count( const uint8_t* pubszBegin, const uint8_t* pubszEnd )
         {                                                                                         assert( pubszBegin <  pubszEnd ); assert( (pubszEnd - pubszBegin) < 0xffffff );
            uint32_t uCount = 0;
            while( pubszBegin <  pubszEnd )
            {
               pubszBegin = next( pubszBegin );
               uCount++;
            }
            return uCount;
         }

         /** ------------------------------------------------------------------
          * @brief Count number of utf8 characters need to store json escaped text
          * Counts number of utf8 until end is reached
          * @param pubszText start of text that is counted
          * @return uint32_t number of characters
          */
         uint32_t size( const uint8_t* pubszText )
         {
            uint32_t uSize = 0;
            while( *pubszText != '\0' )
            {
               uint32_t uChar = character( pubszText );
               uSize += gd::utf8::size( uChar );
               pubszText = next( pubszText );
            }
            return uSize;
         }


         /** ------------------------------------------------------------------
          * @brief Count number of utf8 characters need to store json escaped text
          * Counts number of utf8 until end is reached
          * @param pubszBegin start of text that is counted
          * @param pubszEnd count until this position is reached
          * @return uint32_t number of characters
          */
         uint32_t size( const uint8_t* pubszBegin, const uint8_t* pubszEnd )
         {                                                                                         assert( pubszBegin <  pubszEnd ); assert( (pubszEnd - pubszBegin) < 0xffffff );
            uint32_t uSize = 0;
            while( pubszBegin <  pubszEnd )
            {
               uint32_t uChar = character( pubszBegin );
               uSize += gd::utf8::size( uChar );
               pubszBegin = next( pubszBegin );
            }
            return uSize;
         }


         /** ------------------------------------------------------------------
          * @brief move to next character within json formated text
          * @param pubszPosition start position that is moved to next character
          * @return const uint8_t* position to next character
         */
         const uint8_t* next( const uint8_t* pubszPosition )
         {                                                                                         assert( *pubszPosition != '\0' );
            if( *pubszPosition == '\\' )
            {
               pubszPosition++;
               if( *pubszPosition == 'u' )
               {
                  pubszPosition += 5;
               }
               else
               {
                  pubszPosition++;
               }
            }
            return pubszPosition;
         }

         /** ------------------------------------------------------------------
          * @brief find character in string that need to be escaped to work in json formated text
          * @param pubszBegin pointer to start
          * @param pubszEnd pointer to one behind end
          * @return position to first found character that need to be escaped or null if no character found
          */
         const uint8_t* find_character_to_escape(const uint8_t* pubszBegin, const uint8_t* pubszEnd )
         {
            const uint8_t* pubszPosition = pubszBegin;
            while(pubszPosition < pubszEnd)
            {
               if( pIsJsonEscape_s[*pubszPosition] != 0 ) return pubszPosition;

               pubszPosition++;
            }

            return nullptr;
         }

         /** ------------------------------------------------------------------
          * @brief Convert utf8 formated text to json formated buffer
          * @param pubszText pointer to start of utf8 text to convert
          * @param pubszEnd end of utf8 text
          * @param pbszTo pointer to buffer storing converted utf8 characters
          * @return true and last position in buffer if ok, false if error
          */
         std::pair<bool, const uint8_t*> convert_utf8_to_json(const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo)
         {
            auto* pubszInsert = pbszTo;
            if( pubszText < pubszEnd )
            {
               auto pubszPosition = pubszText;
               while( pubszPosition < pubszEnd )
               {
                  uint32_t uCharacterSize = gd::utf8::get_character_size( pubszPosition );            assert( uCharacterSize != 0 ); assert( uCharacterSize == 1 ? *pubszPosition < 0x80 : true );
                  if( uCharacterSize == 1 && pEncodeXml_s[*pubszPosition] == 0 )
                  {
                     *pubszInsert = *pubszPosition;
                     pubszInsert++;
                     pubszPosition++;
                  }
                  else
                  {
                     *pubszInsert = '\\';
                     pubszInsert++;
                     switch(*pubszPosition)
                     {
                     case '\"': *pubszInsert = '\"'; break;
                     case '\\': *pubszInsert = '\\'; break;
                     case '/':  *pubszInsert = '/';  break;
                     case '\b': *pubszInsert = 'b';  break;
                     case '\f': *pubszInsert = 'f';  break;
                     case '\n': *pubszInsert = 'n';  break;
                     case '\r': *pubszInsert = 'r';  break;
                     case '\t': *pubszInsert = 't';  break;
                     default: assert(false); break;
                     }
                     pubszInsert++;
                     pubszPosition++;
                  }// if else
               }// for(
            } // if(

            return { true, pubszInsert };
         }

         /** ------------------------------------------------------------------
         * @brief Convert utf8 formated text to json formated and place it in string
         * @param pubszText pointer to start of utf8 text to convert
         * @param pubszEnd end of utf8 text
         * @param stringTo string where escaped text is written to
         * @return true if text is properly converted
         */
         bool convert_utf8_to_json(const uint8_t* pubszText, const uint8_t* pubszEnd, std::string& stringTo )
         {
            if( pubszText < pubszEnd )
            {
               auto pubszPosition = pubszText;
               while( pubszPosition < pubszEnd )
               {
                  uint32_t uCharacterSize = gd::utf8::get_character_size( pubszPosition );            assert( uCharacterSize != 0 ); assert( uCharacterSize == 1 ? *pubszPosition < 0x80 : true );
                  if( uCharacterSize == 1 && pEncodeXml_s[*pubszPosition] == 0 )
                  {
                     stringTo += (char)*pubszPosition;
                     pubszPosition++;
                  }
                  else
                  {
                     stringTo += '\\';
                     switch(*pubszPosition)
                     {
                     case '\"': stringTo += '\"'; break;
                     case '\\': stringTo += '\\'; break;
                     case '/':  stringTo += '/';  break;
                     case '\b': stringTo += 'b';  break;
                     case '\f': stringTo += 'f';  break;
                     case '\n': stringTo += 'n';  break;
                     case '\r': stringTo += 'r';  break;
                     case '\t': stringTo += 't';  break;
                     default: assert(false); break;
                     }
                     pubszPosition++;
                  }// if else
               }// for(
            } // if(

            return true;
         }


      }


      /** ---------------------------------------------------------------------
       * @brief methods used to convert text from browser that is formatted to be 
       * converted into utf8
      */
      namespace uri {

         /** ------------------------------------------------------------------
          * @brief get character value for uri character
          * Format for uri character are % and number.  This format is used in
          * query string (url)
          * Sample: `%C3%83` = O
          * @param pubszCharacter uri character to convert
          * @return uint32_t number for character
         */
         uint32_t character( const uint8_t* pubszCharacter )
         {
            uint32_t uCharacter = 0;
            if( *pubszCharacter == '%' )
            {                                                                  // format should be % + hex + hex (sample: %20 = 32 = space)
               pubszCharacter++;
               uCharacter = (pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)];
               uint32_t uSize = pNeededByteCount_s[uCharacter];                                    assert( uSize != 0 );// should never be 0, then it is something wrong with the uri format

               if( uSize == 1 ) { assert( uCharacter < CHARACTER_1_BYTE_MASK ); return (uCharacter & ~CHARACTER_1_BYTE_MASK); }// if only one character then return value without modification

               if( uSize == 2 )
               {
                  uCharacter &= ~CHARACTER_2_BYTE_MASK;                        // keep valid part
                  uCharacter <<= 6;
                  pubszCharacter += sizeof "%00" - 1;                          // go to next character
                                                                                                   assert( *(pubszCharacter - 1) == '%' );
                  uCharacter += (pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)] & 0x3f;
                                                                                                   // agil utveckling https://www.youtube.com/watch?v=vSnCeJEka_s
               }
               else if( uSize == 3 )
               {
                  uCharacter &= ~CHARACTER_3_BYTE_MASK;                        // keep valid part
                  uCharacter <<= 12;
                  pubszCharacter += sizeof "%00" - 1;                          // go to next character
                                                                                                   assert( *(pubszCharacter - 1) == '%' );
                  uCharacter += (((pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)]) & 0x3f) << 6;

                  pubszCharacter += sizeof "%00" - 1;                          // go to next character
                                                                                                   assert( *(pubszCharacter - 1) == '%' );
                  uCharacter += ((pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)]) & 0x3f;
               }
            }
            else
            {
               uCharacter = *pubszCharacter;
            }

            return uCharacter;
         }

         /** ------------------------------------------------------------------
          * @brief Calculate the needed size to store character as utf8
          * @param pubszCharacter pointer to uri character size is calculated for
          * @return number of bytes needed to store
         */
         uint32_t size( const uint8_t* pubszCharacter ) 
         {
            if( *pubszCharacter == '%' )
            {
               pubszCharacter++;                                                                   assert( *pubszCharacter != '\0' && *(pubszCharacter + 1) != '\0' ); // need at least two hex values to get character size, cant be \0
               uint32_t uCharacter = 0;
               uCharacter = (pHexValue_s[*pubszCharacter] << 4) + pHexValue_s[*(pubszCharacter + 1)];
               uint32_t uSize = pNeededByteCount_s[uCharacter];                                    assert( uSize != 0 );// should never be 0, then it is something wrong with the uri format
               return uSize;
            }

            return 1;
         }

         /** ------------------------------------------------------------------
          * @brief count utf8 characters in buffer
          * @param pubszText pointer to buffer with text where characters are counted
          * @param pubszEnd pointer to end of buffer
          * @return number of utf8 characters in buffer
         */
         std::pair<uint32_t, const uint8_t*> count( const uint8_t* pubszText, const uint8_t* pubszEnd )
         {
            uint32_t uCount = 0; // counted characters in buffer
            const uint8_t* pubszPosition = pubszText;
            while( pubszPosition != pubszEnd )
            {                                                                                      assert( pubszPosition < pubszEnd ); assert( pNeededByteCount_s[*pubszPosition] != 0 );
               if( *pubszPosition == '%' )                                     // if % then skip uri sequence
               {
                  uCount++;
                  auto uSize = uri::size( pubszPosition );                     // get size needed to move to next character
                  pubszPosition += (uSize * 3);                                                    assert( pubszPosition <= pubszEnd );
                  continue;
               }

               pubszPosition += pNeededByteCount_s[*pubszPosition];
               uCount++;
            }

            return std::pair<uint32_t, const uint8_t*>(uCount, pubszPosition);
         }

         /** ------------------------------------------------------------------
          * @brief Check if character need to be encoded when sent as uri formated text
          * @param uChar ascii character
          * @return true if character is encoded, false if not
         */
         inline bool is_encoded( uint8_t uChar )
         {
            if( uChar < 0x80 ) return pEncodeUri_s[uChar] != 0;
            return true;
         }

         /** ------------------------------------------------------------------
          * @brief Validate uri formated text
          * Checks for non valid characters within uri formated text
          * @param pubBegin start of uri formated text to validate
          * @param pubEnd  end of uri formated text to validate
          * @return 
         */
         std::pair<bool, const uint8_t*> validate( const uint8_t* pubBegin, const uint8_t* pubEnd )
         {
            auto pubPosition = pubBegin;
            for( auto pubPosition = pubBegin; pubPosition != pubEnd; pubPosition++ )
            {
               // check if character should be encoded but is not
               if( is_encoded( *pubPosition ) == true && *pubPosition != '%' ) return {false, pubPosition};// return error, this character is not valid within uri encoded text
            }

            return { true, pubEnd };
         }

         /** ------------------------------------------------------------------
          * @brief Calculate needed buffer size to store uri text as utf8
          * @param pubszText uri text buffer size is calculated for
          * @param pubszEnd end of uri text
          * @return number of bytes needed to store uri text in utf8 format not including zero terminator
         */
         uint32_t get_character_size( const uint8_t* pubszText, const uint8_t* pubszEnd )
         {
            uint32_t uSize = 0; // counted characters in buffer
            const uint8_t* pubszPosition = pubszText;
            while( pubszPosition != pubszEnd )
            {                                                                                      assert( pubszPosition < pubszEnd ); assert( pNeededByteCount_s[*pubszPosition] != 0 );
               if( *pubszPosition == '%' )                                     // if % then skip uri sequence
               {
                  auto uCount = uri::size( pubszPosition );                    // get size needed to move to next character
                  pubszPosition += (uCount * 3);                                                   assert( pubszPosition <= pubszEnd );
                  uSize += uCount;
                  continue;
               }

               pubszPosition += pNeededByteCount_s[*pubszPosition];
               uSize++;
            }

            return uSize;
         }

         /** ------------------------------------------------------------------
          * @brief move to next character within uri formated text
          * @param pubszPosition start position that is moved to next character
          * @return const uint8_t* position to next character
         */
         inline const uint8_t* next( const uint8_t* pubszPosition )
         {                                                                                         assert( *pubszPosition != '\0' );

            if( *pubszPosition != '%' )                                        // if normal ascii character
            {
               pubszPosition++;
            }
            else                                                               // found `%` character that marks utf8 hex code
            {
               auto uSize = uri::size( pubszPosition );
               pubszPosition += (uSize * 3);
            }

            return pubszPosition;
         }

         /** ------------------------------------------------------------------
          * @brief finds first utf8 escape sequence in string 
          * @param pubszPosition start position where to begin searching for utf8 escape sequence (`%` character)
          * @param pubszEnd end position, where to stop searching
          * @return const uint8_t* position to first utf8 escape sequence if found, otherwise null
         */
         const uint8_t* next_sequence( const uint8_t* pubszPosition, const uint8_t* pubszEnd )
         {
            for( auto it = pubszPosition; it != pubszEnd; it++ )
            {
               if( *it == '%' ) return it;
            }
            return nullptr;
         }

         /** ------------------------------------------------------------------
          * @brief convert uri formated utf8 text into utf8 buffer
          * @param pubszText start of uri formated text converted into utf8 buffer
          * @param pubszEnd end of uri formated text
          * @param pbszTo buffer getting converted uri formated text
          * @return true and last position in buffer if ok, false if error
         */
         std::pair<bool, const uint8_t*> convert_uri_to_uf8( const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo )
         {
            auto pubszInsert = pbszTo;
            for( auto pubszPosition = pubszText; pubszPosition != pubszEnd; pubszPosition = uri::next( pubszPosition ) )
            {                                                                                      assert( pubszPosition < pubszEnd );
               auto uCharacter = uri::character( pubszPosition );
               pubszInsert += convert( uCharacter, pubszInsert );
            }

            return { true, pubszInsert };
         }

         /** ------------------------------------------------------------------
          * @brief converts uri formated text to utf8
          * @param stringUri uri formated string that is converted to utf8
          * @param stringUtf8 converted string is placed in this string
          * @return true if ok, false and position in buffer if error
         */
         std::pair<bool, const uint8_t*> convert_uri_to_uf8( const std::string_view& stringUri, std::string& stringUtf8 )
         {
            auto uSize = get_character_size( reinterpret_cast<const uint8_t*>(stringUri.data()), reinterpret_cast<const uint8_t*>(stringUri.data()) + stringUri.length() );
            std::vector<char> vectorText;                                      // using vector as temporary buffer because std::string do not have the logic for that

            vectorText.reserve( uSize );
            auto result_ = convert_uri_to_uf8( reinterpret_cast<const uint8_t*>(stringUri.data()), reinterpret_cast<const uint8_t*>(stringUri.data()) + stringUri.length(), (uint8_t*)vectorText.data() );
            if( result_.first == true )
            {
               decltype(vectorText.data()) puEnd = decltype(vectorText.data())(result_.second); // get last position for converted text found in vector
               stringUtf8.append( vectorText.data(), puEnd );
               return { true, nullptr };
            }
   
            return result_;
         }

         /** ------------------------------------------------------------------
          * @brief Convert utf8 formated text to uri formated buffer
          * @param pubszText pointer to start of utf8 text to convert
          * @param pubszEnd end of utf8 text
          * @param pbszTo pointer to buffer storing converted utf8 characters
          * @return true and last position in buffer if ok, false if error
         */
         std::pair<bool, const uint8_t*> convert_utf8_to_uri( const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo )
         {
            auto pubszInsert = pbszTo;
            if( pubszText < pubszEnd )
            {
               for( auto pubszPosition = pubszText; pubszPosition != pubszEnd; pubszPosition = uri::next( pubszPosition ) )
               {
                  uint32_t uCharacterSize = gd::utf8::get_character_size( pubszPosition );            assert( uCharacterSize != 0 ); assert( uCharacterSize == 1 ? *pubszPosition < 0x80 : true );
                  if( uCharacterSize == 1 && pEncodeUri_s[*pubszPosition] == 0 )
                  {
                     *pubszInsert = *pubszPosition;
                     pubszInsert++;
                  }
                  else
                  {
                     // ## add uri hex sequence for character
                     for( auto u = 0u; u < uCharacterSize; u++ )
                     {
                        pubszInsert[0] = '%';
                        pubszInsert[1] = pszHEX_s[*pubszPosition >> 4];
                        pubszInsert[2] = pszHEX_s[*pubszPosition & 0x0f];
                        pubszInsert += 3;
                        pubszPosition++;
                     }
                  }// if else
               }// for(
            } // if(

            return { true, pubszInsert };
         }


         /** ------------------------------------------------------------------
          * @brief Convert utf8 formated text to uri formated buffer
          * @param pubszText pointer to start of utf8 text to convert
          * @param pubszEnd end of utf8 text
          * @param stringTo string storing converted utf8 characters
          * @return true if ok, false if error
         */
         bool convert_utf8_to_uri( const uint8_t* pubszText, const uint8_t* pubszEnd, std::string& stringTo )
         {
            if( pubszText < pubszEnd )
            {
               for( auto pubszPosition = pubszText; pubszPosition != pubszEnd; pubszPosition = gd::utf8::move::next( pubszPosition ) )
               {
                  assert( pubszPosition < pubszEnd );
                  uint32_t uCharacterSize = gd::utf8::get_character_size( pubszPosition );            assert( uCharacterSize != 0 );
                  assert( uCharacterSize == 1 ? *pubszPosition < 0x80 : true );
                  if( uCharacterSize == 1 && pEncodeUri_s[*pubszPosition] == 0 )
                  {
                     stringTo += *pubszPosition;
                  }
                  else
                  {
                     auto p_ = pubszPosition;
                     // ## add uri hex sequence for character
                     for( auto u = 0u; u < uCharacterSize; u++ )
                     {
                        stringTo += '%';
                        stringTo += pszHEX_s[*p_ >> 4];
                        stringTo += pszHEX_s[*p_ & 0x0f];
                        p_++;
                     }
                  }
               } // for(
            }

            return true;
         }

      }

      namespace xml {

         /** ------------------------------------------------------------------
         * @brief Calculate the needed size to store character as xml character
         * @param pubszCharacter pointer to uri character size is calculated for
         * @return number of bytes needed to store
         */
         uint32_t size( const uint8_t* pubszCharacter ) 
         {
            if( *pubszCharacter < 0x80 )
            {
               if( pEncodeXml_s[*pubszCharacter] == 0 ) return 1;
               return pEncodeXml_s[*pubszCharacter];
            }

            return gd::utf8::get_character_size( pubszCharacter );
         }

         /** ------------------------------------------------------------------
          * @brief Check if character need to be encoded when sent as uri formated text
          * @param uChar ascii character
          * @return true if character is encoded, false if not
         */
         inline bool is_encoded( uint8_t uChar )
         {
            if( uChar < 0x80 ) return pEncodeXml_s[uChar] != 0;
            return false;
         }



         /** ------------------------------------------------------------------
          * @brief escape utf8 characters to xml text
          * @param pubszText start of utf8 text
          * @param pubszEnd end of text
          * @param pbszTo buffer written to
          * @return rue and last position in buffer if ok, false if error
          */
         std::pair<bool, const uint8_t*> convert_utf8_to_xml(const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo)
         {
            auto* pubszInsert = pbszTo;
            if( pubszText < pubszEnd )
            {
               auto pubszPosition = pubszText;
               while( pubszPosition < pubszEnd )
               {
                  uint32_t uCharacterSize = gd::utf8::get_character_size( pubszPosition );            assert( uCharacterSize != 0 ); assert( uCharacterSize == 1 ? *pubszPosition < 0x80 : true );
                  if( uCharacterSize == 1 && pEncodeXml_s[*pubszPosition] == 0 )
                  {
                     *pubszInsert = *pubszPosition;
                     pubszInsert++;
                     pubszPosition++;
                  }
                  else
                  {
                     if(uCharacterSize == 1)
                     {                                                                             assert( pEncodeXml_s[*pubszPosition] != 0 );
                        uCharacterSize = pEncodeXml_s[*pubszPosition];
                        switch(*pubszPosition)
                        {
                        case '&' : memcpy( pubszInsert, "&amp;", uCharacterSize ); break;
                        case '<' : memcpy( pubszInsert, "&lt;", uCharacterSize ); break;
                        case '>' : memcpy( pubszInsert, "&gt;", uCharacterSize ); break;
                        case '\"': memcpy( pubszInsert, "&quot;", uCharacterSize ); break;
                        case '\'': memcpy( pubszInsert, "&apos;", uCharacterSize ); break;
                        }

                        pubszInsert += uCharacterSize;
                        pubszPosition++;
                     }
                     else
                     {
                        for(auto u = 0u; u < uCharacterSize; u++)
                        {
                           pubszInsert[u] = pubszPosition[u];
                        }
                        pubszInsert += uCharacterSize;
                        pubszPosition += uCharacterSize;
                     }
                  }// if else
               }// for(
            } // if(

            return { true, pubszInsert };
         }

         /** ------------------------------------------------------------------
          * @brief Convert utf8 formated text xml escaped string
          * @param pubszText pointer to start of utf8 text to escpae
          * @param pubszEnd end of utf8 text
          * @param stringTo string storing converted xml escaped value
          * @return true if ok, false if error
         */
         bool convert_utf8_to_xml( const uint8_t* pubszText, const uint8_t* pubszEnd, std::string& stringTo )
         {
            if( pubszText < pubszEnd )
            {
               auto pubszPosition = pubszText;
               while( pubszPosition < pubszEnd )
               {
                  uint32_t uCharacterSize = gd::utf8::get_character_size( pubszPosition );            assert( uCharacterSize != 0 ); assert( uCharacterSize == 1 ? *pubszPosition < 0x80 : true );
                  if( uCharacterSize == 1 && pEncodeXml_s[*pubszPosition] == 0 )
                  {
                     stringTo += (char)*pubszPosition;
                     pubszPosition++;
                  }
                  else
                  {
                     if(uCharacterSize == 1)
                     {                                                                             assert( pEncodeXml_s[*pubszPosition] != 0 );
                        uCharacterSize = pEncodeXml_s[*pubszPosition];
                        switch(*pubszPosition)
                        {
                        case '&' : stringTo.append( "&amp;" ); break;
                        case '<' : stringTo.append( "&lt;" ); break;
                        case '>' : stringTo.append( "&gt;" ); break;
                        case '\"': stringTo.append( "&quot;" ); break;
                        case '\'': stringTo.append( "&apos;" ); break;
                        }

                        pubszPosition++;
                     }
                     else
                     {
                        stringTo.append( (const char*)pubszPosition, uCharacterSize );
                        pubszPosition += uCharacterSize;
                     }
                  }// if else
               }// while(
            }

            return true;
         }

      } // xml

   } // utf8



   namespace utf16 {
      uint32_t character(const uint16_t* pubszCharacter)
      {
         uint32_t uCharacter;
         if(*pubszCharacter < 0x80) uCharacter = static_cast<uint32_t>(*pubszCharacter);
         else if(*pubszCharacter < 0x800) uCharacter = static_cast<uint32_t>( ((0x1f & pubszCharacter[0]) << 6) | (0x3f & pubszCharacter[1]) );
         else uCharacter = static_cast<uint32_t>( ((0x0f & pubszCharacter[0]) << 12) | ((0x3f & pubszCharacter[1]) << 6) | (0x3f & pubszCharacter[2]) );

         return uCharacter;
      }

      uint32_t size(uint16_t uCharacter)
      {
         if(uCharacter < 0x80) return 1;
         else if(uCharacter < 0x800) return 2;
         return 3;
      }

   }
} // gd





/*

    template <typename octet_iterator>
    inline typename std::iterator_traits<octet_iterator>::difference_type
    sequence_length(octet_iterator lead_it)
    {
        uint8_t lead = utf8::internal::mask8(*lead_it);
        if (lead < 0x80)
            return 1;
        else if ((lead >> 5) == 0x6)
            return 2;
        else if ((lead >> 4) == 0xe)
            return 3;
        else if ((lead >> 3) == 0x1e)
            return 4;
        else
            return 0;
    }


size_t utf8codepointsize(utf8_int32_t chr) {
  if (0 == ((utf8_int32_t)0xffffff80 & chr)) {
    return 1;
  } else if (0 == ((utf8_int32_t)0xfffff800 & chr)) {
    return 2;
  } else if (0 == ((utf8_int32_t)0xffff0000 & chr)) {
    return 3;
  } else { // if (0 == ((int)0xffe00000 & chr)) {
    return 4;
  }
}



*/

namespace gd {
   namespace utf8 {

      std::string quoted( const std::string_view& stringToQuote )
      {
         std::string stringQuoted;
         stringQuoted += '\"';
         stringQuoted += stringToQuote;
         stringQuoted += '\"';
         return stringQuoted;
      }

      std::string quoted_if_text( const std::string_view& stringToQuote )
      {
         bool bIsText = false;
         for( auto it = std::begin( stringToQuote ), itEnd = std::end( stringToQuote ); bIsText == false && it != itEnd; it++ )
         {
            if( *it > 0x65 || pIsDigit_s[(uint8_t)*it] == 0 ) bIsText = true;  // 'e' = 0x65 = exponent
         }

         if( bIsText == true )
         {
            std::string stringQuoted;
            stringQuoted += '\"';
            stringQuoted += stringToQuote;
            stringQuoted += '\"';
            return stringQuoted;
         }

         return std::string( stringToQuote );
      }


      /** -------------------------------------------------------------------
       * @brief Split string into multiple string_view and and store them in vector, string is split where char is found
       * @param stringText text to split into multiple parts (remember to not destroy string as long as work with string_view items are used)
       * @param chSplitWith character used to mark where to split text
       * @param vectorPart vector where strings are stored
      */
      void split( const std::string_view& stringText, char chSplitWith, std::vector<std::string_view>& vectorPart )
      {
         std::string stringPart;                // Store string parts added to vector

         const char* pbszStart = stringText.data();
         for( const char* pbszPosition = stringText.data(), *pbszEnd = stringText.data() + stringText.length(); pbszPosition != pbszEnd; pbszPosition++ )
         {
            if( *pbszPosition == chSplitWith )
            {
               vectorPart.emplace_back( std::string_view( pbszStart, pbszPosition - pbszStart ) );
               pbszStart = pbszPosition + 1;
            }
         }

         // if part contains text or last position in string is same as split character, then add one more to vector
         if( pbszStart <= &stringText.back() || stringText.back() == chSplitWith ) vectorPart.emplace_back( std::string_view( pbszStart, (stringText.data() + stringText.length()) - pbszStart));
      }

      /** -------------------------------------------------------------------
       * @brief Split string into multiple strings and store them in vector
       * @param pbBegin start of string that is splitted
       * @param pbEnd end of string
       * @param chSplit split character
       * @param vectorPart vector where strings are stored 
      */
      void split( const char* pbBegin, const char* pbEnd, char chSplitWith, std::vector<std::string_view>& vectorPart )
      {                                                                                                  assert( pbBegin <= pbEnd );
         const char* pbTo;
         const char* pbFrom = pbBegin;
         const char* pbPosition = pbBegin;
         for( ; pbPosition != pbEnd; pbPosition++ )
         {
            if( *pbPosition == chSplitWith )
            {
               pbTo = pbPosition;
               std::size_t uLength = pbTo - pbFrom;
               vectorPart.push_back(std::string_view{ pbFrom, uLength });
               pbFrom = pbTo + 1;
            }
         }

         if(pbPosition != pbFrom)
         {
            pbTo = pbPosition;
            std::size_t uLength = pbTo - pbFrom;
            vectorPart.push_back(std::string_view{ pbFrom, std::size_t(pbTo - pbFrom) });
         }
      }



      /** -------------------------------------------------------------------
       * @brief Split string into multiple strings and store them in vector, string is split where char is found
       * @param stringText text to split into multiple parts
       * @param chSplitWith character used to mark where to split text
       * @param vectorPart vector where strings are stored
      */
      void split(const std::string_view& stringText, char chSplitWith, std::vector<std::string>& vectorPart)
      {
         std::string stringPart;                // Store string parts added to vector

         for(auto it : stringText)
         {
            if(it == chSplitWith)
            {
               vectorPart.emplace_back(stringPart);
               stringPart.clear();
            }
            else
            {
               stringPart += it;
            }
         }

         // if part contains text or last position in string is same as split character, then add one more to vector
         if(stringPart.empty() == false || stringText.back() == chSplitWith) vectorPart.emplace_back(stringPart);
      }

      /** -------------------------------------------------------------------
       * @brief Split string into multiple strings and store them in vector, string is split where char is found
       * @param stringText text to split into multiple parts
       * @param stringSplit character sequence that marks where to split text
       * @param vectorPart vector where strings are stored
      */
      void split(const std::string_view& stringText, const std::string_view& stringSplit, std::vector<std::string>& vectorPart)
      {                                                                                            assert(stringSplit.length() > 0);
         std::string stringPart;                // Store string parts added to vector
         auto uLength = stringSplit.length();   // Split string lenght
         const uint8_t* pubszSplitWith = reinterpret_cast<const uint8_t*>(stringSplit.data()); // help compiler to optimize ?

         const uint8_t* pubszPosition = reinterpret_cast<const uint8_t*>(stringText.data()); // start of text
         const uint8_t* pubszTextEnd = reinterpret_cast<const uint8_t*>(stringText.data() + stringText.length()); // end of text
         while(pubszPosition != pubszTextEnd)
         {
            assert(pubszPosition < pubszTextEnd); assert(*pubszPosition != 0);
            // Compare if split text sequence is found
            if(*pubszPosition == *pubszSplitWith && memcmp((void*)pubszPosition, (void*)pubszSplitWith, uLength) == 0)
            {
               vectorPart.emplace_back(stringPart);
               stringPart.clear();
               pubszPosition += uLength;                                       // move past split sequence
               continue;
            }

            stringPart += (char*)pubszPosition;
            pubszPosition++;                                                   // next character
         }
      }

      /** ---------------------------------------------------------------------
       * @brief Split string into multiple strings and store them in vector, string is split where utf8 character `uSplitWith` is found
       * @param stringText text to split into multiple parts
       * @param uSplitWith utf8 character to split string with
       * @param vectorPart vector where strings are stored
      */
      void split(const std::string_view& stringText, uint32_t uSplitWith, std::vector<std::string>& vectorPart)
      {
         uint8_t pubszBuffer[] = { 0, 0, 0, 0, 0 };
         auto uSize = convert(uSplitWith, pubszBuffer);
         return split(stringText, std::string_view((const char*)pubszBuffer, uSize), vectorPart);
      }

      /** ---------------------------------------------------------------------
       * @brief Split string into string parts or unsigned number parts
       * @param pbBegin start of string
       * @param pbEnd end of string
       * @param chSplitWith character used to split string into parts
       * @param vectorPart vector where parts are added
       */
      void split(const char* pbBegin, const char* pbEnd, char chSplitWith, std::vector< std::variant< unsigned, std::string > >& vectorPart)
      {
         using namespace gd::types;
         const char* pbTo;
         const char* pbFrom = pbBegin;
         const char* pbPosition = pbBegin;

         // ## lambda method used to add value to vector
         auto add_value_ = [](decltype(vectorPart) vectorPart, const char* pbFrom, const char* pbTo) {
            std::size_t uLength = pbTo - pbFrom;
            if( is_ctype_g( *pbFrom, "digit"_ctype ) == true )                 // found digit?
            {
               unsigned uColumn = *pbFrom - '0';
               pbFrom++;
               while(pbFrom < pbTo && is_ctype_g(*pbFrom, "digit"_ctype) == true) 
               {
                  uColumn *= 10;
                  uColumn += *pbFrom - '0';
                  pbFrom++;
               }
               vectorPart.push_back( uColumn );
            }
            else { vectorPart.push_back( std::string{ pbFrom, uLength } ); }
         };

         for(; pbPosition != pbEnd; pbPosition++)
         {
            if(*pbPosition == chSplitWith)
            {
               pbTo = pbPosition;
               add_value_( vectorPart, pbFrom, pbTo );

               pbFrom = pbTo + 1;
            }
         }

         if( pbPosition != pbFrom )
         {
            pbTo = pbPosition;
            add_value_( vectorPart, pbFrom, pbTo );
         }
      }

      /** ---------------------------------------------------------------------
       * @brief Split string into vector of string_view parts for each split offset positions
       * @param pbBegin start of string
       * @param pbEnd end of string
       * @param vectorSplit positions where to split string
       * @param vectorPart string parts from splitting
       */
      void split(const char* pbBegin, const char* pbEnd, const std::vector<std::size_t>& vectorSplit, std::vector<std::string_view>& vectorPart)
      {
         std::size_t uTextSize = pbEnd - pbBegin; 
         std::size_t uPosition = 0;
         for(auto it : vectorSplit)
         {
            if(uPosition < uTextSize)
            {  
               auto uLength = it - uPosition;
               vectorPart.push_back( std::string_view( pbBegin + uPosition, uLength ) );
               uPosition = it + 1;
            }
            else
            {
               vectorPart.push_back( std::string_view() );
            }
         }

         if( uPosition < uTextSize ) { vectorPart.push_back( std::string_view( pbBegin + uPosition, uTextSize - uPosition ) ); }
      }

      /** ---------------------------------------------------------------------
       * @brief Split string into vector of string parts for each split offset positions
       * @param pbBegin start of string
       * @param pbEnd end of string
       * @param vectorSplit positions where to split string
       * @param vectorPart string parts from splitting
       */
      void split(const char* pbBegin, const char* pbEnd, const std::vector<std::size_t>& vectorSplit, std::vector<std::string>& vectorPart)
      {
         std::size_t uTextSize = pbEnd - pbBegin; 
         std::size_t uPosition = 0;
         for(auto it : vectorSplit)
         {
            if(uPosition < uTextSize)
            {  
               auto uLength = it - uPosition;
               vectorPart.push_back( std::string( pbBegin + uPosition, uLength ) );
               uPosition = it + 1;
            }
            else
            {
               vectorPart.push_back( std::string() );
            }
         }

         if( uPosition < uTextSize ) { vectorPart.push_back( std::string( pbBegin + uPosition, uTextSize - uPosition ) ); }
      }


      /** ---------------------------------------------------------------------
       * @brief Extract text betweend selected characters
       * @param puBegin start of text
       * @param puEnd end of text
       * @param uStop character used to mark where to select text
       * @param ppairPosition pointer to pair with pointers used to mark positions where text are splitted
       * @return std::string selected text
       */
      std::string mid( const uint8_t* puBegin, const uint8_t* puEnd, uint8_t uStop, std::pair< const uint8_t*, const uint8_t* >* ppairPosition )
      {
         const uint8_t* puFirst = puBegin;
         const uint8_t* puLast = puEnd;
         std::string stringMid;

         if( *puLast == '\0' ) puLast--;
         else move::previous( puLast );

         while( puFirst < puEnd && *puFirst != uStop ) { puFirst = move::next( puFirst ); }

         if( puFirst < puEnd )
         {
            while( puLast > puBegin && *puLast != uStop ) { puLast = move::previous( puLast ); }            assert( puFirst <= puLast );

            puFirst++;                                                         // strip marked character
            if( puFirst >= puLast ) puLast = puEnd;

            stringMid = std::string_view( (const char*)puFirst, puLast - puFirst );
         }
         else
         {
            puFirst = puBegin;
         }

         if( ppairPosition != nullptr ) *ppairPosition = std::pair< const uint8_t*, const uint8_t* >{ puFirst, puLast };
         
         return stringMid;
      }

      /** -------------------------------------------------------------------
       * @brief Find offset positions based on start of string
       * @param pbBegin from offset relates to
       * @param pbEnd last position to search for offsets for character
       * @param chMarkOffset character offsets are marked for
       * @param vectorOffset vector storing offset positions
       */
      void offset(const char* pbBegin, const char* pbEnd, char chMarkOffset, std::vector<std::size_t>& vectorOffset)
      {                                                                                                  assert( pbBegin <= pbEnd );
         for(const char* pbPosition = pbBegin; pbPosition != pbEnd; pbPosition++)
         {
            if( *pbPosition == chMarkOffset ) vectorOffset.push_back( pbPosition - pbBegin );
         }
      }


      /** -------------------------------------------------------------------
       * @brief Print utf8 text buffer data to buffer as hexadecimal string
       * @param puText utf8 text buffer
       * @param puEnd end of utf8 text buffer
       * @param puHex buffer written to
       * @return last position in buffer written to
      */
      uint8_t* print_hex( const uint8_t* puText, const uint8_t* puEnd, uint8_t* puHex )
      {
         for( auto it = puText; it != puEnd; it++ )
         {
            uint8_t uCharacter = *it;

            *puHex = pszHEX_s[ uCharacter >> 4 ];
            puHex++;
            *puHex = pszHEX_s[ uCharacter & 0x0f ];
            puHex++;
         }

         return puHex;
      }

      /** -------------------------------------------------------------------
       * @brief Print utf8 text buffer data to std string as hexadecimal
       * @param puText utf8 text buffer
       * @param puEnd end of utf8 text buffer
       * @param stringHex buffer written to
      */
      void print_hex( const uint8_t* puText, const uint8_t* puEnd, std::string& stringHex )
      {
         char psBuffer[2];
         for( auto it = puText; it != puEnd; it++ )
         {
            uint8_t uCharacter = *it;

            psBuffer[0] = pszHEX_s[uCharacter >> 4];
            psBuffer[1] = pszHEX_s[ uCharacter & 0x0f ];

            stringHex.append( psBuffer, 2 );
         }
      }

   }
}


namespace gd {
   namespace utf8 {
      namespace format {
         /// pad string with char value to the right
         void pad_right( std::string& stringText, std::size_t uLength, char chSpace )
         {                                                                                         assert( uLength < 0x0010'0000 ); // realistic ??
            auto uTextLength = stringText.length();
            if( uTextLength < uLength )
            {
               auto uCount = uLength - uTextLength;
               stringText.insert( stringText.end(), uCount, chSpace );
            }
         }

         /// pad string with char value to the left
         void pad_left( std::string& stringText, std::size_t uLength, char chSpace )
         {                                                                                         assert( uLength < 0x0010'0000 ); // realistic ??
            auto uTextLength = stringText.length();
            if( uTextLength < uLength )
            {
               auto uCount = uLength - uTextLength;
               stringText.insert( 0, uCount, chSpace );
            }
         }

         /// format buffer with 16 bytes (uuid value) to string
         void to_uuid( const uint8_t* puData, std::string& stringUuid )
         {
            const unsigned puDash[] = {3, 5, 7, 9, 0};
            const unsigned* puIndex = &puDash[0];
            char pbBuffer[36];

            unsigned uIndex = 0;
            char* pbsz = &pbBuffer[0];
            for( auto it = puData, itEnd = puData + 16; it != itEnd; it++, uIndex++ )
            {
               unsigned uByte = ((*it) >> 4) & 0x0F;
               *pbsz++ = uByte <= 9 ? '0' + uByte : 'a' + (uByte - 10);

               uByte = (*it) & 0x0F;
               *pbsz++ = uByte <= 9 ? '0' + uByte : 'a' + (uByte - 10);

               if( uIndex == *puIndex )
               {
                  *pbsz++ = '-';
                  puIndex++;
               }
            }

            stringUuid.append( pbBuffer, 36 );
         }


         /** ------------------------------------------------------------------
          * @brief replace all occurrences of string to find with passed replace text
          * @code
          * std::string string_("11 22 33 22 11 22");
          * string_ = gd::utf8::format::replace( string_, "22", "44");
          * std::cout << string_ ; // 11 44 33 44 11 442
          * @endcode
          * @param stringText string with all text where find string is replaced
          * @param stringFind string to find
          * @param stringReplace string to replace with
          * @return std::string new string with replaced string parts
          */
         std::string replace( const std::string_view& stringText, const std::string_view& stringFind, const std::string_view& stringReplace )
         {
            std::string stringNew;
            std::size_t uStart = 0;
            std::size_t uFind = stringText.find( stringFind );
            while( uFind != std::string_view::npos )
            {
               stringNew.append( stringText.substr( uStart, uFind - uStart ) );// append text before found text
               stringNew.append( stringReplace );                              // insert replace text

               uFind += stringFind.length();                                   // move past the found text
               uStart = uFind;                                                 // set start text to new position

               uFind = stringText.find( stringFind, uFind );                   // find next
            }

            stringNew.append( stringText.substr( uStart, stringText.length() - uStart ) );
            return stringNew;
         }
      }
   }
}



namespace gd {
   namespace utf8 {
      namespace debug {
         std::string print( const uint8_t* puText, const uint8_t* puEnd, const char* pbszSplit, unsigned uColumnCount )
         {
            const auto CHAR_SIZE = 32;
            unsigned uCount = CHAR_SIZE;
            unsigned uCharSize = 2 + strlen( pbszSplit );                                          assert( uCharSize < CHAR_SIZE );
            char pbszByte[CHAR_SIZE];
            std::memcpy( pbszByte, pbszSplit, uCharSize - 2 );
            std::string stringHex;
            for( auto it = puText; it != puEnd; it++ )
            {
               uint8_t uCharacter = *it;

               pbszByte[uCharSize - 2] = pszHEX_s[ uCharacter >> 4 ];
               pbszByte[uCharSize - 1] = pszHEX_s[ uCharacter & 0x0f ];

               stringHex.append( pbszByte, uCharSize );

               uCount++;

               if( uColumnCount > 0 && (uCount % uColumnCount) == 0 ) stringHex += ( '\n' );
            }

            return stringHex;
         }

         std::string print( const void* pText, std::size_t uLength )
         {
            return print( (const uint8_t*)pText, (const uint8_t*)pText + uLength, " ", 8 );
         }
      }

   }
}

