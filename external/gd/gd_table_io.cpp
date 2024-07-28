#include <numeric>

#include "gd_parse.h"
#include "gd_sql_value.h"

#include "gd_table_io.h"

_GD_TABLE_BEGIN

namespace {
   bool format_copy( std::string_view stringValue, std::string& stringNew )
   {
      stringNew += stringValue;
      return true;
   }
}

/// format string as uri and return true if string is modified, if not (no characters found that needs to be formated) false is returned
bool format_if( const std::string_view& stringText, std::string& stringNew, tag_io_uri )
{
   // TODO: Check if string needs to be formatted
   gd::utf8::uri::convert_utf8_to_uri( (const uint8_t*)stringText.data(), ( const uint8_t* )stringText.data() + stringText.length(), stringNew);
   return true;
}

/** ---------------------------------------------------------------------------
 * @brief 
 * @param stringTableData 
 * @param table 
 * @return 
*/
/*
std::pair<bool, std::string> to_table( const std::string_view& stringTableData, dto::table& table )
{
   using namespace gd::parse;

   std::string_view stringHeader;
   std::vector<std::string_view> vectorHeader;
   std::vector<std::string_view> vectorBody;
   std::vector< std::vector<std::string_view> > vectorBodyCell;

   const char* pbszLineEnd;
   const char* pbszText = stringTableData.data();
   const char* pbszEnd = pbszText + stringTableData.length();

   const char* pbszPosition = pbszText;

   csv csvFormat;

   if( table.empty() == true )
   {
      pbszLineEnd = strchr( pbszPosition, pbszEnd, '\n', csvFormat );
      if( pbszLineEnd != nullptr )
      {
         stringHeader = std::string_view( pbszPosition, pbszLineEnd - pbszPosition );
         pbszLineEnd = skip_space_g( pbszLineEnd, pbszEnd );
         pbszPosition = pbszLineEnd;

         //read_line_g( stringHeader,  )

         //vectorHeader
      }
   }

   return std::pair<bool, std::string>();
}
*/

/// convert table to csv
void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_csv )
{
   std::function<bool(std::string_view, std::string& stringNew)> functionFormat( format_copy );
   std::string stringResult;                    // result string with table data
   std::vector<gd::variant_view> vectorValue;   // used to fetch values from table

   if( format_text_ ) { functionFormat = format_text_; }

   auto uEnd = uBegin + uCount;
   for( auto uRow = uBegin; uRow < uEnd; uRow++ )
   {
      if( uRow != uBegin ) stringResult += ",\n";                              // add comma separator between rows

      vectorValue.clear();
      table.row_get_variant_view( uRow, vectorValue );

      unsigned uColumn = 0;
      auto it = std::begin( vectorValue );
      auto itEnd = std::end( vectorValue );
      for( ; it < itEnd; it++ )
      {
         if( uColumn > 0 ) stringResult += ",";                                // add `,` to separate columns
         auto value_ = *it;

         if( value_.is_null() == true )
         {
            // pass, no value
         }
         else if( value_.is_string() == true )
         {
            stringResult += "\"";
#ifdef _DEBUG
            auto uLength_d = strlen((const char*)value_);                                          assert( uLength_d < 0x0010'0000 );
            auto uChar_d = ((const char*)value_)[uLength_d];                                       assert( uChar_d == 0 );
#endif // _DEBUG

            std::string string_ = value_.as_string();
            gd::parse::escape_g( string_, gd::parse::tag_csv{});
            bool bOk = functionFormat( string_, stringResult );

            //auto pbszValue = ( const char* )value_;
            //auto [bOk, pbuszPosition] = gd::utf8::validate( pbszValue );
            //if( bOk == true ) { gd::utf8::uri::convert_utf8_to_uri( (const uint8_t*)pbszValue, (const uint8_t*)pbszValue + value_.length(), stringResult ); }
            //else
            //{                                                               // Not valid utf8 text
            //   // ## text is not valid utf8 format, convert to utf8
            //   std::string stringUtf8;
            //   gd::utf8::convert_ascii( pbszValue, stringUtf8 );
            //   gd::utf8::uri::convert_utf8_to_uri( stringUtf8, stringResult );
            //}
            stringResult += "\"";
         }
         else
         {
            stringResult += value_.as_string( gd::variant_type::tag_scientific{} );
         }

         uColumn++;
      }
   }
   if( stringResult.empty() == false ) stringResult += "\n";                   // add new line

   if( stringOut.empty() == true ) stringOut = std::move( stringResult );
   else stringOut += stringResult;
}


/// convert table (both header and body) to json array
void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_csv)
{
   std::string stringResult;   // result string with table data

   auto it = table.column_begin(), itEnd = table.column_end();

   if(it != itEnd)
   {
      stringResult += '\"';
      if(it->alias() != 0) { stringResult += table.column_get_alias( *it ); }
      else { stringResult += table.column_get_name( *it ); }
      stringResult += '\"';
      it++;
   }

   std::string stringName;
   for(; it < itEnd; it++)
   {
      stringResult += std::string_view( ",\"" );
      if(it->alias() != 0)
      {
         stringName = table.column_get_alias( *it );
      }
      else 
      {
         stringName = table.column_get_name( *it );
      }

      gd::parse::escape_g( stringName, gd::parse::tag_csv{});

      stringResult += stringName;

      stringResult += '\"';
   }

   // ## add body if count is set
   if( uCount > 0 && table.get_row_count() > 0 )
   {
      stringResult += "\n";
      to_string( table, uBegin, uCount, format_text_, stringResult, tag_io_csv{});
   }

   if( stringOut.empty() == true ) stringOut = std::move( stringResult );
   else stringOut += stringResult;
}


void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, bool (*format_text_)(unsigned uColumn, unsigned uType, const gd::variant_view&, std::string& stringNew), std::string& stringOut, tag_io_csv )
{
   std::string stringResult;                    // result string with table data
   std::vector<gd::variant_view> vectorValue;   // used to fetch values from table

   auto uEnd = uBegin + uCount;
   for( auto uRow = uBegin; uRow < uEnd; uRow++ )
   {
      if( uRow != uBegin ) stringResult += ",\n";                              // add comma separator between rows

      vectorValue.clear();
      table.row_get_variant_view( uRow, vectorValue );

      unsigned uColumn = 0;
      auto it = std::begin( vectorValue );
      auto itEnd = std::end( vectorValue );
      for( ; it < itEnd; it++ )
      {
         if( uColumn > 0 ) stringResult += ",";                                // add `,` to separate columns
         const auto& value_ = *it;

         unsigned uType = table.column_get_type( uColumn );
         bool bContinue = format_text_( uColumn, uType, value_, stringResult );
         if( bContinue == true ) { uColumn++; continue; }

         if( value_.is_null() == true )
         {
            // pass, no value
         }
         else if( value_.is_string() == true )
         {
            stringResult += "\"";
            std::string string_ = value_.as_string();
            gd::parse::escape_g( string_, gd::parse::tag_csv{});
            stringResult += string_;
            stringResult += "\"";
         }
         else
         {
            stringResult += value_.as_string( gd::variant_type::tag_scientific{} );
         }

         uColumn++;
      }
   }
   if( stringResult.empty() == false ) stringResult += "\n";                   // add new line

   if( stringOut.empty() == true ) stringOut = std::move( stringResult );
   else stringOut += stringResult;
}


std::pair<bool, const char*> read_g(dto::table& table, const std::string_view& stringCsv, char chSeparator, char chNewLine, tag_io_csv)
{
   gd::parse::csv csvRule( chSeparator, chNewLine );                           // csv rules used to parse values
   const char* pbszPosition = &*stringCsv.begin();                             // get start pointer to string
   const char* pbszEnd = &stringCsv.back() + 1;                                // get end pointer for string
   auto vectorType = table.column_get_type();

   std::vector<gd::variant_view> vectorValue;

   while(pbszPosition < pbszEnd)
   {
      auto result_ = gd::parse::read_line_g( pbszPosition, pbszEnd, vectorValue, vectorType, csvRule );
      if( result_.first == true )
      {
         table.row_add( vectorValue );
         vectorValue.clear();
         pbszPosition = result_.second;
      }
      else
      {
         return result_;
      }
   }

   return { true, pbszEnd };
}

/** ---------------------------------------------------------------------------
 * @brief Read csv formated text into table
 * @param table table receiving read csv formated text
 * @param stringCsv text formated as csv
 * @param vectorColumn index for column where csv formated value is placed
 * @param chSeparator character used to separate csv values
 * @param chNewLine separate each row
 * @return true if ok, false and error information on error
 */
std::pair<bool, const char*> read_g(dto::table& table, const std::string_view& stringCsv, const std::vector<unsigned>& vectorColumn, char chSeparator, char chNewLine, tag_io_csv)
{
   gd::parse::csv csvRule( chSeparator, chNewLine );                           // csv rules used to parse values
   const char* pbszPosition = &*stringCsv.begin();                             // get start pointer to string
   const char* pbszEnd = &stringCsv.back() + 1;                                // get end pointer for string
   auto vectorType = table.column_get_type( vectorColumn );

   std::vector<gd::variant_view> vectorValue; //values read from csv text

   while(pbszPosition < pbszEnd)
   {
      auto result_ = gd::parse::read_line_g( pbszPosition, pbszEnd, vectorValue, vectorType, csvRule );
      if( result_.first == true )
      {
         table.row_add( vectorValue, vectorColumn );
         vectorValue.clear();
         pbszPosition = result_.second;
      }
      else
      {
         return result_;
      }
   }

   return { true, pbszEnd };
}


std::pair<bool, const char*> read_g(std::vector<std::string>& vectorHeader, const std::string_view& stringCsv, char chSeparator, char chNewLine, tag_io_csv)
{
   gd::parse::csv csvRule( chSeparator, chNewLine );                           // csv rules used to parse values
   
   return std::pair<bool, const char*>();
}

// std::pair< bool, std::string >

/// convert table to json array
void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_json )
{
   std::function<bool(std::string_view, std::string& stringNew)> functionFormat( format_copy );
   std::string stringResult;                    // result string with table data
   std::vector<gd::variant_view> vectorValue;   // used to fetch values from table

   if( format_text_ ) { functionFormat = format_text_; }

   auto uEnd = uBegin + uCount;
   for( auto uRow = uBegin; uRow < uEnd; uRow++ )
   {
      if( uRow != uBegin ) stringResult += ",\n";                              // add comma separator between rows
      stringResult += "[";

      vectorValue.clear();
      table.row_get_variant_view( uRow, vectorValue );

      unsigned uColumn = 0;
      auto it = std::begin( vectorValue );
      auto itEnd = std::end( vectorValue );
      for( ; it < itEnd; it++ )
      {
         if( uColumn > 0 ) stringResult += ",";                                // add `,` to separate columns
         auto value_ = *it;                                                    // active column value

         if( value_.is_null() == true )
         {
            stringResult += "null";                                            // for javascript null
         }
         else if( value_.is_string() == true )
         {
            stringResult += "\"";
#ifdef _DEBUG
            auto uLength_d = strlen((const char*)value_);                                          assert( uLength_d < 0x0010'0000 );
            auto uChar_d = ((const char*)value_)[uLength_d];                                       assert( uChar_d == 0 );
#endif // _DEBUG

            const std::string_view text_ = value_.as_string_view();
            bool bOk = functionFormat( text_, stringResult );
            stringResult += "\"";
         }
         else
         {
            stringResult += value_.as_string( gd::variant_type::tag_scientific{} );
         }

         uColumn++;
      }

      stringResult += ']' ;
   }
   if( stringResult.empty() == false ) stringResult += "\n";                   // add new line

   if( stringOut.empty() == true ) stringOut = std::move( stringResult );
   else stringOut += stringResult;
}

/// convert table (both header and body) to json array
void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_json)
{
   std::string stringResult("[");   // result string with table data

   auto it = table.column_begin(), itEnd = table.column_end();

   if(it != itEnd)
   {
      stringResult += '\"';
      if(it->alias() != 0) { stringResult += table.column_get_alias( *it ); }
      else { stringResult += table.column_get_name( *it ); }
      stringResult += '\"';
      it++;
   }

   for(; it < itEnd; it++)
   {
      stringResult += std::string_view( ",\"" );
      if(it->alias() != 0)
      {
         stringResult += table.column_get_alias( *it );
      }
      else 
      {
         stringResult += table.column_get_name( *it );
      }

      stringResult += '\"';
   }

   stringResult += ']' ;

   // ## add body if count is set
   if( uCount > 0 && table.get_row_count() > 0 )
   {
      stringResult += ",\n";
      to_string( table, uBegin, uCount, format_text_, stringResult, tag_io_json{});
   }

   if( stringOut.empty() == true ) stringOut = std::move( stringResult );
   else stringOut += stringResult;
}

/// Convert table data to json arrays where each row is placed in array as an object. key for value is the name for column
void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_json, tag_io_name)
{
   std::function<bool(std::string_view, std::string& stringNew)> functionFormat( format_copy );
   std::string stringName;                      // used for names that need to be escaped
   std::string stringResult;                    // result string with table data
   std::vector<std::string_view> vectorName;    // column names for table
   std::vector<gd::variant_view> vectorValue;   // used to fetch values from table

   if( format_text_ ) { functionFormat = format_text_; }

   vectorName = table.column_get_name();

   auto uEnd = uBegin + uCount;
   for( auto uRow = uBegin; uRow < uEnd; uRow++ )
   {
      if( uRow != uBegin ) stringResult += ",\n";                              // add comma separator between rows
      stringResult += std::string_view("{");

      vectorValue.clear();
      table.row_get_variant_view( uRow, vectorValue );                         assert( vectorName.size() == vectorValue.size() );

      unsigned uColumn = 0;

      for( unsigned uColumn = 0, uColumnMax = (unsigned)vectorValue.size(); uColumn < uColumnMax; uColumn++ )
      {
         if( uColumn > 0 ) stringResult += ",";                                // add `,` to separate columns

         auto name_ = vectorName.at( uColumn );
         auto value_ = vectorValue.at( uColumn );

         stringResult += std::string_view("\"");
         if( gd::utf8::json::find_character_to_escape( name_ ) == nullptr ) { stringResult += name_; }
         else
         {
            stringName.clear();
            gd::utf8::json::convert_utf8_to_json( name_, stringName );
            stringResult += stringName;
         }
         
         stringResult += std::string_view("\": ");

         if( value_.is_null() == true )
         {
            stringResult += "null";                                            // for javascript null
         }
         else if( value_.is_string() == true )
         {
            stringResult += "\"";
#ifdef _DEBUG
            auto uLength_d = strlen((const char*)value_);                                          assert( uLength_d < 0x0010'0000 );
            auto uChar_d = ((const char*)value_)[uLength_d];                                       assert( uChar_d == 0 );
#endif // _DEBUG

            const std::string_view text_ = value_.as_string_view();
            bool bOk = functionFormat( text_, stringResult );
            stringResult += "\"";
         }
         else
         {
            stringResult += value_.as_string( gd::variant_type::tag_scientific{} );
         }

         uColumn++;
      }

      stringResult += std::string_view("}");
   }
   if( stringResult.empty() == false ) stringResult += "\n";                   // add new line

   if( stringOut.empty() == true ) stringOut = std::move( stringResult );
   else stringOut += stringResult;
}

namespace {

/** ---------------------------------------------------------------------------
 * @brief Print table header information
 * @param table table object 
 * @param stringOut 
 * @param  
 */
template <typename TABLE>
void to_string_s(const TABLE& table, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_header)
{
   stringOut.append( std::string_view{"| "});
   for(unsigned uColumn = 0; uColumn < table.get_column_count(); uColumn++)
   {
      if( uColumn != 0 ) { stringOut += std::string_view{ " | "}; }
      auto name_ = table.column_get_name( uColumn );
      stringOut += name_;
   }
   stringOut.append( std::string_view{" |\n"});
}

/** ---------------------------------------------------------------------------
 * @brief print table with nicer format, sutable for console printing
 * @param table table to print
 * @param uBegin from what row in table to print
 * @param uCount last row to print
 * @param vectorWidth max number of characters for each column in table, this needs to be prepared befor sending table to string
 * @param argumentOption options on how to pring
 * @param argumentOption.count max row count converted to string
 * @param argumentOption.nr if true value then print row numbers
 * @param argumentOption.max_column_width if set value then prite elipsis if size is above
 * @param stringOut string getting generated string with database data
*/
template <typename TABLE>
void to_string_s( const TABLE& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const gd::argument::arguments& argumentOption, std::string& stringOut )
{
   if( argumentOption.exists("count") == true ) { uCount = argumentOption["count"].as_uint64(); }  // set max count if argument for count is found

   unsigned uMax = (unsigned)-1;                // max column width
   auto uEnd = uBegin + uCount;                 // last row to print to string
   std::string stringHeader;                    // Column headers
   std::string stringLineDivide;                // Line to divied rows, default is to add it as before first and after last row
   std::string stringResult;                    // result string with table data
   std::string stringValue;                     // value as string added to result
   std::vector<gd::variant_view> vectorValue;   // used to fetch values from table

   bool bNr = argumentOption["nr"].is_true();   // should rows be numbered?
   bool bRaw = true; // argumentOption["raw"].is_true(); // dont add quotes around text?

   if( argumentOption.exists("max_column_width") == true )                     // check for max column width and shrink if above
   { 
      uMax = argumentOption["max_column_width"].as_uint(); 
      std::for_each( std::begin( vectorWidth ), std::end( vectorWidth ), [uMax]( auto& uWidth ) { if( uWidth > uMax ) uWidth = uMax; });
   }

   if( bNr == true )                                                           // are rows numbered ?
   {
      auto uRowCount = table.get_row_count();
      unsigned uLength = gd::variant::compute_digit_count_s( uRowCount );
      vectorWidth.insert( vectorWidth.begin(), uLength);
   }

   // ## calculate total length needed for each row in characters
   unsigned uTotalLength = std::accumulate( std::begin(vectorWidth), std::end(vectorWidth), 0u );
   uTotalLength += 2 * (sizeof(" |") - 1);
   uTotalLength += ((unsigned)vectorWidth.size() - 1) * (sizeof(" | ") - 1);


   // ## Generate line divider
   // ### Create a line with total length
   stringLineDivide += '+';
   stringLineDivide.insert( stringLineDivide.end(), uTotalLength - 2, '-' );
   stringLineDivide.append( std::string_view{"+\n"});

   // ### Place markers for each column
   unsigned uOffset = 2;                                                       // start at position two beqause first characters on row is "+-"
   for( auto it : vectorWidth )
   {
      uOffset += (it + 1);                                                     // add width of the column
      if( uOffset < stringLineDivide.length() ) stringLineDivide[uOffset] = '+';// set column marker `+` if not the last ending marker
      uOffset += 2;                                                            // move offset to next column
   }

   // ## Print column names
   {
      unsigned uColumn = 0;
      auto it = std::begin(vectorWidth);
      if( bNr == true ) 
      { 
         it++; 
         stringHeader += '#';
         stringHeader.insert( stringHeader.begin() + 1, 2 + vectorWidth.at( 0 ), ' '); 
      };
      for( auto itEnd = std::end(vectorWidth); it != itEnd; it++, uColumn++ )
      {
         std::string stringName( table.column_get_name(uColumn) );
         if( (*it + 3) < stringName.length() ) stringName = stringName.substr( 0, *it + 2 );

         unsigned uTotal = *it + 3;
         if( uTotal >= stringName.length() ) stringName.insert( stringName.end(), uTotal - stringName.length(), ' ' );

         stringHeader += stringName;
      }

      stringHeader += '\n';
   }

   stringResult += stringHeader;
   stringResult += stringLineDivide;                                           // add divide line
   
   for( auto uRow = uBegin; uRow < uEnd; uRow++ )
   {
      stringResult += "| ";

      vectorValue.clear();
      table.row_get_variant_view( uRow, vectorValue );

      if( bNr == true ) vectorValue.insert( vectorValue.begin(), uRow );

      unsigned uColumn = 0;
                                                                                                   assert( vectorValue.size() == vectorWidth.size());
      auto itWidth = std::begin( vectorWidth );
      for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it < itEnd; it++, itWidth++ )
      {
         if( uColumn > 0 ) stringResult.append(std::string_view{" | "});                           // add ` | ` to separate columns
         auto value_ = *it;

         if( value_.is_null() == true )
         {
            stringValue = "";
         }
         else if( value_.is_string() == true )
         {
            const std::string_view text_ = value_.as_string_view();
            stringValue += text_;
         }
         else
         {
            stringValue = value_.as_string();
         }

         if(stringValue.length() > uMax)
         {
            stringValue = stringValue.substr( 0, uMax - 3 );
            stringValue += std::string_view{"..."};
         }

         auto uValueLength = stringValue.length();
         if( *itWidth > uValueLength )
         {
            unsigned uAlign = gd::types::align_g( value_.type_number() );
            if( uAlign == 0 ) gd::utf8::format::pad_right( stringValue, *itWidth, ' ' );
            else              gd::utf8::format::pad_left( stringValue, *itWidth, ' ' );
         }

         stringResult += stringValue;
         stringValue.clear();

         uColumn++;
      }

      stringResult.append( std::string_view{" |\n"});
   }

   stringResult += stringLineDivide;                                           // add divide line

   stringOut += stringResult;
}

/** ---------------------------------------------------------------------------
* @brief print table with nicer format, sutable for console printing
* @param table table to print
* @param uBegin from what row in table to print
* @param uCount last row to print
* @param vectorWidth max number of characters for each column in table, this needs to be prepared befor sending table to string
* @param vectorColumn selected columns to print
* @param argumentOption options on how to pring
* @param argumentOption.count max row count converted to string
* @param argumentOption.nr if true value then print row numbers
* @param argumentOption.max_column_width if set value then prite elipsis if size is above
* @param stringOut string getting generated string with database data
*/
template <typename TABLE>
void to_string_s( const TABLE& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, std::vector<unsigned> vectorColumn, const gd::argument::arguments& argumentOption, std::string& stringOut )
{
   if( argumentOption.exists("count") == true ) { uCount = argumentOption["count"].as_uint64(); }  // set max count if argument for count is found

   unsigned uMax = (unsigned)-1;                // max column width
   auto uEnd = uBegin + uCount;                 // last row to print to string
   std::string stringHeader;                    // Column headers
   std::string stringLineDivide;                // Line to divied rows, default is to add it as before first and after last row
   std::string stringResult;                    // result string with table data
   std::string stringValue;                     // value as string added to result
   std::vector<gd::variant_view> vectorValue;   // used to fetch values from table

   bool bNr = argumentOption["nr"].is_true();   // should rows be numbered?
   bool bRaw = true; // argumentOption["raw"].is_true(); // dont add quotes around text?

   if( argumentOption.exists("max_column_width") == true )                     // check for max column width and shrink if above
   { 
      uMax = argumentOption["max_column_width"].as_uint(); 
      std::for_each( std::begin( vectorWidth ), std::end( vectorWidth ), [uMax]( auto& uWidth ) { if( uWidth > uMax ) uWidth = uMax; });
   }

   if( bNr == true )                                                           // are rows numbered ?
   {
      auto uRowCount = table.get_row_count();
      unsigned uLength = gd::variant::compute_digit_count_s( uRowCount );
      vectorWidth.insert( vectorWidth.begin(), uLength);
   }

   // ## calculate total length needed for each row in characters
   unsigned uTotalLength = std::accumulate( std::begin(vectorWidth), std::end(vectorWidth), 0u );
   uTotalLength += 2 * (sizeof(" |") - 1);
   uTotalLength += ((unsigned)vectorWidth.size() - 1) * (sizeof(" | ") - 1);


   // ## Generate line divider
   // ### Create a line with total length
   stringLineDivide += '+';
   stringLineDivide.insert( stringLineDivide.end(), uTotalLength - 2, '-' );
   stringLineDivide.append( std::string_view{"+\n"});

   // ### Place markers for each column
   unsigned uOffset = 2;                                                       // start at position two beqause first characters on row is "+-"
   for( auto it : vectorWidth )
   {
      uOffset += (it + 1);                                                     // add width of the column
      if( uOffset < stringLineDivide.length() ) stringLineDivide[uOffset] = '+';// set column marker `+` if not the last ending marker
      uOffset += 2;                                                            // move offset to next column
   }

   // ## Print column names
   {
      auto it = std::begin(vectorWidth);
      if( bNr == true ) 
      { 
         it++; 
         stringHeader += '#';
         stringHeader.insert( stringHeader.begin() + 1, 2 + vectorWidth.at( 0 ), ' '); 
      };

      unsigned uColumn = 0;
      for( auto itEnd = std::end(vectorWidth); it != itEnd; it++, uColumn++ )
      {
         std::string stringName( table.column_get_name( vectorColumn.at(uColumn) ) );
         if( (*it + 3) < stringName.length() ) stringName = stringName.substr( 0, *it + 2 );

         unsigned uTotal = *it + 3;
         if( uTotal >= stringName.length() ) stringName.insert( stringName.end(), uTotal - stringName.length(), ' ' );

         stringHeader += stringName;
      }

      stringHeader += '\n';
   }

   stringResult += stringHeader;
   stringResult += stringLineDivide;                                           // add divide line

   for( auto uRow = uBegin; uRow < uEnd; uRow++ )
   {
      stringResult += "| ";

      vectorValue.clear();
      table.row_get_variant_view( uRow, vectorColumn, vectorValue );

      if( bNr == true ) vectorValue.insert( vectorValue.begin(), uRow );

      assert( vectorValue.size() == vectorWidth.size());
      auto itWidth = std::begin( vectorWidth );
      for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it < itEnd; it++, itWidth++ )
      {
         if( it != std::begin( vectorValue ) ) stringResult.append(std::string_view{" | "});     // add ` | ` to separate columns
         auto value_ = *it;

         if( value_.is_null() == true )
         {
            stringValue = "";
         }
         else if( value_.is_string() == true )
         {
            const std::string_view text_ = value_.as_string_view();
            stringValue += text_;
         }
         else
         {
            stringValue = value_.as_string();
         }

         if(stringValue.length() > uMax)
         {
            stringValue = stringValue.substr( 0, uMax - 3 );
            stringValue += std::string_view{"..."};
         }

         auto uValueLength = stringValue.length();
         if( *itWidth > uValueLength )
         {
            unsigned uAlign = gd::types::align_g( value_.type_number() );
            if( uAlign == 0 ) gd::utf8::format::pad_right( stringValue, *itWidth, ' ' );
            else              gd::utf8::format::pad_left( stringValue, *itWidth, ' ' );
         }

         stringResult += stringValue;
         stringValue.clear();
      }

      stringResult.append( std::string_view{" |\n"});
   }

   stringResult += stringLineDivide;                                           // add divide line

   stringOut += stringResult;
}


} // namespace

/** ---------------------------------------------------------------------------
 * @brief generate column information and format information for json
 * @param table table column information is taken from
 * @param stringOut string where column information is placed
 */
void to_string(const dto::table& table, std::string& stringOut, tag_io_json, tag_io_column)
{
   std::string stringColumns;
   for(unsigned uColumn = 0, uMax = table.get_column_count(); uColumn < uMax; uColumn++)
   {
      if( stringColumns.empty() == false ) { stringColumns += ','; }
      stringColumns += '{';
      const auto& column_ = table.column_get( uColumn );

      stringColumns += std::string_view{ "\"name\":\""};
      stringColumns += table.column_get_name( column_ );

      stringColumns += std::string_view{ "\", \"type\":"};
      std::string_view stringType = gd::types::type_name_g( column_.type() );
      stringColumns += '\"';
      stringColumns += stringType;
      stringColumns += '\"';

      stringColumns += std::string_view{ ", \"type_number\":"};
      stringColumns += std::to_string( column_.type() );

      stringColumns += std::string_view{ ", \"size\":"};
      stringColumns += std::to_string( column_.size() );

      stringColumns += std::string_view{ ", \"alias\":\""};
      stringColumns += table.column_get_alias( column_ );
      stringColumns += '\"';

      stringColumns += '}';
   }

   stringOut += stringColumns;
}

/** ---------------------------------------------------------------------------
 * @brief generate column information and format information for json
 * @param table table column information is taken from
 * @param stringOut string where column information is placed
 * @param vectorExtra vector containing extra values
 */
void to_string( const dto::table& table, std::string& stringOut, const std::vector<gd::argument::arguments>& vectorExtra, tag_io_json, tag_io_column )
{
   std::string stringValue;
   std::string stringColumns;
   for(unsigned uColumn = 0, uMax = table.get_column_count(); uColumn < uMax; uColumn++)
   {
      if( stringColumns.empty() == false ) { stringColumns += ','; }
      stringColumns += '{';
      const auto& column_ = table.column_get( uColumn );

      stringColumns += std::string_view{ "\"name\":\""};
      stringColumns += table.column_get_name( column_ );

      stringColumns += std::string_view{ "\", \"type\":"};
      std::string_view stringType = gd::types::type_name_g( column_.type() );
      stringColumns += '\"';
      stringColumns += stringType;
      stringColumns += '\"';

      stringColumns += std::string_view{ ", \"type_number\":"};
      stringColumns += std::to_string( column_.type() );

      stringColumns += std::string_view{ ", \"size\":"};
      stringColumns += std::to_string( column_.size() );

      stringColumns += std::string_view{ ", \"alias\":\""};
      stringColumns += table.column_get_alias( column_ );
      stringColumns += '\"';

      if( uColumn < vectorExtra.size() )
      {
         for( auto arguments_ : vectorExtra )
         {
            /*
            for( auto it = arguments_.begin(); it != arguments_.end(); it++ )
            {
               std::string_view name_ = it.name();
               gd::variant_view value_  = it.get_argument().as_variant_view();

               stringColumns += std::string_view{ ", \"" };
               stringColumns += name_;
               stringColumns += std::string_view{ "\":" };
               if( value_.is_string() == true )
               {
                  stringColumns += '\"';
                  stringValue = value_.as_string();
                  if( gd::utf8::json::find_character_to_escape( stringValue ) == nullptr ) { stringColumns += stringValue; }
                  else
                  {
                     std::string stringJsonValue;
                     gd::utf8::json::convert_utf8_to_json( stringValue, stringJsonValue );
                     stringColumns += stringJsonValue;
                  }
                  stringColumns += '\"';
               }
               else
               {
                  stringColumns += value_.as_string();   
               }
            }
            */
         }
      }

      stringColumns += '}';
   }

   stringOut += stringColumns;
}

// ## CLI IO (command line interface) -----------------------------------------

void to_string(const dto::table& table, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_header, tag_io_cli)
{
   to_string_s( table, argumentOption, stringOut, tag_io_header{});
}

void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli)
{
   to_string_s( table, uBegin, uCount, vectorWidth, argumentOption, stringOut );
}

void to_string(const table& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli)
{
   to_string_s( table, uBegin, uCount, vectorWidth, argumentOption, stringOut );
}

void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const std::vector<unsigned>& vectorcolumn, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli)
{
   to_string_s( table, uBegin, uCount, vectorWidth, vectorcolumn, argumentOption, stringOut );
}

void to_string(const table& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const std::vector<unsigned>& vectorcolumn, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli)
{
   to_string_s( table, uBegin, uCount, vectorWidth, vectorcolumn, argumentOption, stringOut );
}



namespace {
   /// Helper method to add column names to string used to print sql statements
   inline void add_column_name_s(std::string& string_, const std::vector<std::string_view>& vectorName) {
      auto it = vectorName.begin();
      string_ += *it;
      it++;
      for(auto itEnd = vectorName.end(); it != itEnd; it++ )
      {
         string_.append( std::string_view{ ", " } );
         string_ += *it;
      }
   }

   /// Helper method to add column values to string used to print sql statements
   inline void add_column_value_s(std::string& string_, const std::vector<gd::variant_view>& vectorValue) {
      auto it = vectorValue.begin();
      gd::sql::append_g( *it, string_ );
      it++;
      for(const auto itEnd = vectorValue.cend(); it != itEnd; it++ )
      {
         string_.append( std::string_view{ ", " } );
         gd::sql::append_g( *it, string_ );
      }
   }

}

/** ---------------------------------------------------------------------------
 * @brief Generate SQL INSERT statements for table data
 * @code
// sample code how to use write_insert_g creating insert queries
std::string stringCsvCodeGroup = R"(1,Group1,Group1 description, 1)";
 gd::table::dto::table tableCodeGroup( (table::eTableFlagNull32|table::eTableFlagRowStatus), { { "int64", "GroupK"}, { "rstring", "FName"}, { "rstring", "FDescription"}, { "bool", "FActive"} }, gd::table::tag_prepare{} );
 auto vectorType = tableCodeGroup.column_get_type();
 gd::table::read_g( tableCodeGroup, stringCsvCodeGroup, ',', '\n', gd::table::tag_io_csv{} );
 std::string stringInsert;
 gd::table::write_insert_g( "TCodeGroup", tableCodeGroup, stringInsert, gd::table::tag_io_sql{} );  
 std::cout << stringInsert << std::endl;
 * @endcode
 * @param stringTableName Table name to generate insert statement for
 * @param uBegin start row for data to generate values to insert into table
 * @param uCount number of rows to generate
 * @param table table data with values and names for insert statement
 * @param stringInsert string that gets insert statement
 * @param argumentsOption options used to modify logic on how to generate sql insert queries
 * @param argumentsOption.names {string} comma separated text with column names used to generate insert query
*/
void write_insert_g(const std::string_view& stringTableName, const dto::table& table, uint64_t uBegin, uint64_t uCount, std::string& stringInsert, const gd::argument::arguments& argumentsOption, tag_io_sql)
{                                                                                                  assert( (uBegin + uCount) <= table.get_row_count() ); 
   std::string stringValues;
   stringInsert += std::string_view{ "INSERT INTO " };
   stringInsert += stringTableName;
   stringInsert.append( std::string_view{ " ( " } );

   decltype( table.column_get_name() ) vectorName;
   if(argumentsOption.exists("names") == true)
   {
      auto stringNames = argumentsOption["names"].as_variant_view().as_string_view();
      vectorName = gd::utf8::split( stringNames, ',', gd::utf8::tag_string_view{});                assert( vectorName.size() == table.get_column_count() );
   }
   else { vectorName = table.column_get_name(); }

   add_column_name_s( stringInsert, vectorName );
   stringInsert.append( std::string_view{ " )\nVALUES(" } );

   std::vector< gd::variant_view > vectorValue;
   for(auto uRow = uBegin, uRowEnd = uBegin + uCount; uRow < uRowEnd; uRow++)
   {
      if( uRow != uBegin ) { stringValues.append( std::string_view{ ",\n( " } ); }

      table.row_get_variant_view( uRow, vectorValue );
      add_column_value_s( stringValues, vectorValue );
      stringValues.append( std::string_view{ " )" } );
      vectorValue.clear();
   }

   stringInsert += stringValues;
}


void write_insert_g(const std::string_view& stringTableName, const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, std::string& stringInsert, const gd::argument::arguments& argumentsOption, tag_io_sql)
{                                                                                                  assert( (uBegin + uCount) <= table.get_row_count() ); 
   std::string stringValues;
   stringInsert += std::string_view{ "INSERT INTO " };
   stringInsert += stringTableName;
   stringInsert.append( std::string_view{ " ( " } );

   decltype( table.column_get_name() ) vectorName;
   if(argumentsOption.exists("names") == true)
   {
      auto stringNames = argumentsOption["names"].as_variant_view().as_string_view();
      vectorName = gd::utf8::split( stringNames, ',', gd::utf8::tag_string_view{});                assert( vectorName.size() == vectorColumn.size() );
   }
   else { vectorName = table.column_get_name(); }

   add_column_name_s( stringInsert, vectorName );
   stringInsert.append( std::string_view{ " )\nVALUES(" } );

   std::vector< gd::variant_view > vectorValue;
   for(auto uRow = uBegin, uRowEnd = uBegin + uCount; uRow < uRowEnd; uRow++)
   {
      if( uRow != uBegin ) { stringValues.append( std::string_view{ ",\n( " } ); }

      table.row_get_variant_view( uRow, vectorColumn, vectorValue );
      add_column_value_s( stringValues, vectorValue );
      stringValues.append( std::string_view{ " )" } );
      vectorValue.clear();
   }

   stringInsert += stringValues;
}


_GD_TABLE_END
