#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <string_view>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "gd_arguments.h"
#include "gd_table.h"
#include "gd_table_column.h"
#include "gd_types.h"
#include "gd_variant_view.h"

#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wreorder-ctor"
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 26495 )
#endif



#ifndef _GD_TABLE_BEGIN
#  define _GD_TABLE_BEGIN namespace gd { namespace table {
#  define _GD_TABLE_END } }
#endif

_GD_TABLE_BEGIN

class table_column_buffer;
namespace dto {
   using table = table_column_buffer;
}


/**
 * \brief  Manage table data store as a big block
 * 
 * 

## Internal data for table

Table stores its data in one single memory block. First part is cell values and
each value has a fixed buffer where data is stored.
int32 is four byte, int64 is 8 byte and string has the max number of bytes specified.
Same logic is used for all primitives and derived types are set with a max buffer size.
Table also support references, they work like pointers so for each column set as 
reference value buffer in table is storing a index to reference and that value can have any size.

Optional is to have meta data for table data. If table store information for null values and/or
valid, deleted or some other row state then a memory block is appended to block that holds table data.
All is stored in one single memory block.

If table need to grow memory block it creates a new block that is larger and data
is copied to that block, old block is deleted.

*Sample data layout*

    ╔═══════╦════════════╦═════════════════════════╦════╗
    ║ int32 ║   int64    ║         string          ║int8║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ╠═══════╩════════════╬═════════════════════════╩════╝
    ║ meta data for each ║
    ║ row                ║
    ║                    ║
    ║                    ║
    ║                    ║
    ║                    ║
    ║                    ║
    ╚════════════════════╝


 *
 *
 *
 \code
 \endcode
 */
class table
{
   friend table_column_buffer;
public:
   /** 
    * \brief constant numbers used in table or items used in table
    */
   enum 
   { 
      // ## column flags marking column states, how column behaves/works
      eColumnStateLength      = 0x01,                                          ///< column flag marking that value begins with length
      eColumnStateReference   = 0x02,                                          ///< column flag marking that value is stored in reference object
      eColumnStateKey         = 0x04,                                          ///< column acts as key column

      // ## row state flags
      eRowStateUse            = 0x01,                                          ///< row flag marking that row is in use
      eRowStateDeleted        = 0x02,                                          ///< row flag marking that row is deleted

      // ## table flags marking table states, how table behaves
      eTableStateNull32       = 0x0001,                                        ///< reserve 32 bit for each row to mark null for column if no value
      eTableFlagNull32        = 0x0001,                                        ///< reserve 32 bit for each row to mark null for column if no value
      eTableStateNull64       = 0x0002,                                        ///< reserve 64 bit for each row to mark null for column if no value
      eTableFlagNull64        = 0x0002,                                        ///< reserve 64 bit for each row to mark null for column if no value
      eTableStateRowStatus    = 0x0004,                                        ///< enable row status (if row is valid, modified, deleted)
      eTableFlagRowStatus     = 0x0004,                                        ///< enable row status (if row is valid, modified, deleted)
      eTableStateMAX          = 0x0010,                                        ///< max state value

      // ## size information used to calculate space needed by table
      eSpaceNull32Columns     = sizeof( uint32_t ),                            ///< space marking null columns
      eSpaceNull64Columns     = sizeof( uint64_t ),                            ///< space marking null columns
      eSpaceRowState          = sizeof( uint32_t ),                            ///< space where row state data is placed
      eSpaceRowGrowBy         = 10,                                            ///< default number of rows to grow by
      eSpaceFirstAllocate     = 10,                                            ///< number of rows to allocate before any values is added

   };


public:
   /**
    * @brief iterator to move trough rows in table
   */
   struct iterator_row
   {
      iterator_row(): m_uRow(0), m_ptable(nullptr) {}
      iterator_row( uint64_t uRow, table* ptable ): m_uRow(uRow), m_ptable(ptable ) {}

      auto operator*() const { return gd::table::row<table>( m_ptable, m_uRow ); }
      uint64_t get_row() const noexcept { return m_uRow; }

      bool operator==( const iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow == m_uRow; }
      bool operator!=( const iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow != m_uRow; }

      iterator_row& operator++() { m_uRow++; return *this; }
      iterator_row operator++(int) { iterator_row it_ = *this; ++(*this); return it_; }
      iterator_row& operator--() { m_uRow--; return *this; }
      iterator_row operator--(int) { iterator_row it_ = *this; --(*this); return it_; }

      auto operator+( std::ptrdiff_t iDistance ) { return iterator_row( (std::ptrdiff_t)m_uRow + iDistance, m_ptable ); }
      auto operator-( std::ptrdiff_t iDistance ) { return iterator_row( (std::ptrdiff_t)m_uRow - iDistance, m_ptable ); }

      gd::variant_view cell_get_variant_view( unsigned uIndex ) const { return m_ptable->cell_get_variant_view( m_uRow, uIndex ); }
      gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const { return m_ptable->cell_get_variant_view( m_uRow, stringName ); }
      std::vector< gd::variant_view > cell_get_variant_view() const { return m_ptable->cell_get_variant_view( m_uRow ); }

      void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue ) { m_ptable->cell_set( m_uRow, uColumn, variantviewValue ); }
      void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue ) { m_ptable->cell_set( m_uRow, stringName, variantviewValue ); }
      void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert ) { m_ptable->cell_set( m_uRow, uColumn, variantviewValue, tag_convert{} ); }
      void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_convert ) { m_ptable->cell_set( m_uRow, stringName, variantviewValue, tag_convert{} ); }

      uint64_t m_uRow;     ///< active row index 
      table* m_ptable; ///< pointer to table that owns the iterator
   };

   struct const_iterator_row
   {
      const_iterator_row(): m_uRow(0), m_ptable(nullptr) {}
      const_iterator_row( uint64_t uRow, const table* ptable ): m_uRow(uRow), m_ptable(ptable) {}
      const_iterator_row( int64_t iRow, table* ptable ): m_uRow((uint64_t)iRow), m_ptable(ptable) {}

      uint64_t get_row() const noexcept { return m_uRow; }

      bool operator==( const const_iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow == m_uRow; }
      bool operator!=( const const_iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow != m_uRow; }

      const_iterator_row& operator++() { m_uRow++; return *this; }
      const_iterator_row operator++(int) { const_iterator_row it_ = *this; ++(*this); return it_; }
      const_iterator_row& operator--() { m_uRow--; return *this; }
      const_iterator_row operator--(int) { const_iterator_row it_ = *this; --(*this); return it_; }

      const_iterator_row operator+( int64_t iDistance ) { return const_iterator_row( m_uRow + iDistance, m_ptable ); }
      const_iterator_row operator-( int64_t iDistance ) { return const_iterator_row( m_uRow - iDistance, m_ptable ); }

      gd::variant_view cell_get_variant_view( unsigned uIndex ) const { return m_ptable->cell_get_variant_view( m_uRow, uIndex ); }
      gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const { return m_ptable->cell_get_variant_view( m_uRow, stringName ); }
      std::vector< gd::variant_view > cell_get_variant_view() const { return m_ptable->cell_get_variant_view( m_uRow ); }

      uint64_t m_uRow;
      const table* m_ptable;
   };


public:
   // ## stl container aliases, simplify for templates using table data

   using column_value_type = detail::column;
   using column_const_value_type = const detail::column;
   using column_iterator = std::vector<detail::column>::iterator;
   using column_const_iterator = std::vector<detail::column>::const_iterator;
   typedef std::random_access_iterator_tag iterator_category;

   using row_value_type = std::vector<gd::table::cell<table> >;  ///< used to simplify stl and containers working with table rows
   using row_const_value_type = const std::vector<gd::table::cell<table> >;///< used to simplify stl and containers working with table rows
   using row_iterator = iterator_row;
   using row_const_iterator = const_iterator_row;
   using row_difference_type = std::ptrdiff_t;

   using value_type = row_value_type;
   using const_value_type = row_const_value_type;
   using iterator = iterator_row;
   using const_iterator = const_iterator_row;
   using difference_type = row_difference_type;

// ## construction -------------------------------------------------------------
public:
   /// @name construction
   /// Constructs table_column_buffer objects. 
   /// - `uRowCount` number of rows that are pre allocated when table is prepared
   /// - `uFlags` flags in enum above (eTableFlagNull32 = manage nulls for max 32 columns, eTableFlagNull64 = manage nulls for max 64 columns, eTableFlagRowStatus = reserve space to mark different row states like deleted etc)
   /// - `uGrowBy` how many rows table should grow by if it needs to increase its size.
   /// - `variantviewValue` create table with one value in constructor
   /// - `tag_prepare` prepare complete table in constructor, no need to call any more methods to start using table logic
   ///@{
   table() : m_uFlags( 0 ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( 0 ), m_uRowGrowBy( 0 ), m_pcolumns{} {}
   table( unsigned uRowCount ) : m_uFlags( 0 ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( uRowCount ), m_uRowGrowBy( 0 ), m_pcolumns{} {}
   table( unsigned uRowCount, unsigned uFlags ) : m_uFlags( uFlags ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( uRowCount ), m_uRowGrowBy( 0 ), m_pcolumns{} { assert( m_uFlags < eTableStateMAX ); }
   table( unsigned uRowCount, unsigned uFlags, unsigned uGrowBy ) : m_uFlags( uFlags ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( uRowCount ), m_uRowGrowBy( uGrowBy ), m_pcolumns{} { assert( m_uFlags < eTableStateMAX ); }
   table( tag_null ) : m_uFlags( eTableFlagNull64 ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( 0 ), m_pcolumns{} { assert( m_uFlags < eTableStateMAX ); }
   table( tag_full_meta ) : m_uFlags( eTableFlagNull64 | eTableFlagRowStatus ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( 0 ), m_pcolumns{} { assert( m_uFlags < eTableStateMAX ); }
   table( unsigned uRowCount, tag_null ) : m_uFlags( eTableFlagNull64 ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( uRowCount ), m_pcolumns{} { assert( m_uFlags < eTableStateMAX ); }
   table( unsigned uRowCount, tag_full_meta ) : m_uFlags( eTableFlagNull64 | eTableFlagRowStatus ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uReservedRowCount( uRowCount ), m_pcolumns{} { assert( m_uFlags < eTableStateMAX ); }

   table( const gd::variant_view& variantviewValue, tag_prepare );
   table( const std::vector< std::string_view >& vectorValue, tag_prepare );
   table( unsigned uFlags, const std::vector< std::tuple< std::string_view, std::string_view > >& vectorValue, tag_prepare );
   table( unsigned uFlags, const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorValue, tag_prepare );
   table( const std::vector< std::tuple< std::string_view, unsigned, std::string_view, gd::variant_view > >& vectorValue, tag_prepare );
// copy
   table( const table& o ): m_puData(nullptr) { common_construct( o ); }
   table( table&& o ) noexcept : m_puData(nullptr) { common_construct( std::move( o ) ); }
   table( detail::columns* pcolumns, const table_column_buffer* ptable, uint64_t uFrom, uint64_t uCount );
   table( detail::columns* pcolumns, const table* ptable, uint64_t uFrom, uint64_t uCount );
// assign
   table& operator=( const table& o ) { clear(); common_construct( o ); return *this; }
   table& operator=( table&& o ) noexcept { clear(); common_construct( std::move( o ) ); return *this; }
   void operator=( detail::columns* pcolumns );

   ~table();
   ///@}

private:
// common copy
   void common_construct( const table& o );
   void common_construct( const table& o, tag_columns );
   void common_construct( const table& o, tag_body );
   void common_construct( table&& o ) noexcept;
   void common_construct( detail::columns* pcolumns );

// ## operator -----------------------------------------------------------------
public:
   std::vector<gd::variant_view> operator[]( uint64_t uRow ) const { return row_get_variant_view( uRow ); }

   gd::variant_view operator[]( const std::pair<unsigned, unsigned>& pairCell ) const { return cell_get_variant_view( pairCell.first, pairCell.second ); }
   gd::variant_view operator[]( const std::pair<unsigned, const std::string_view>& pairCell ) const { return cell_get_variant_view( pairCell.first, pairCell.second ); }

   table& operator+=( const table& o ) { append( o ); return *this; }

   gd::variant_view operator()( uint64_t uRow, unsigned uColumn ) const { return cell_get_variant_view( uRow, uColumn ); }
   gd::variant_view operator()( uint64_t uRow, const std::string_view& stringName ) const { return cell_get_variant_view( uRow, stringName ); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   void set_state [[deprecated]] ( uint32_t uFlags ) noexcept { m_uFlags = uFlags; }
   void set_flags( uint32_t uFlags ) noexcept { m_uFlags = uFlags; }
   void set_state [[deprecated]] ( tag_full_meta ) noexcept { m_uFlags = eTableFlagRowStatus|eTableFlagNull64; }
   void set_flags( tag_full_meta ) noexcept { m_uFlags = eTableFlagRowStatus|eTableFlagNull64; }
   void set_state [[deprecated]] ( uint32_t uSet, uint32_t uClear ) noexcept { m_uFlags |= uSet; m_uFlags &= ~uSet; }
   void set_flags( uint32_t uSet, uint32_t uClear ) noexcept { m_uFlags |= uSet; m_uFlags &= ~uSet; }
   unsigned get_column_count() const noexcept { return (unsigned)m_pcolumns->size(); }
   /// Get number of rows with values
   uint64_t get_row_count() const noexcept { assert( m_puData != nullptr ); return m_uRowCount; }
   /// Number of rows memory is allocated for
   uint64_t get_reserved_row_count() const noexcept { return m_uReservedRowCount; }
   /// get allocated size in bytes for table
   uint64_t get_reserved_size() const noexcept { return m_uReservedRowCount; }
   uint64_t get_row_count( uint32_t uFlags ) const noexcept;
   /// Last valid row index where to insert cell values
   uint64_t get_row_back() const noexcept { assert( m_puData != nullptr ); return m_uRowCount - 1; }
   void set_row_count( uint64_t uCount ) { assert( uCount <= m_uReservedRowCount ); m_uRowCount = uCount; }
   void set_reserved_row_count( uint64_t uCount ) { assert( uCount >= m_uRowCount ); m_uReservedRowCount = uCount; }
   /// return pointer to internal columns object
   detail::columns* get_columns() noexcept { return m_pcolumns; }
   const detail::columns* get_columns() const noexcept { return m_pcolumns; }
   void set_columns( detail::columns* pcolumns ) { assert( m_pcolumns == nullptr ); assert( pcolumns != nullptr ); m_pcolumns = pcolumns; m_pcolumns->add_reference(); }

   // ## state methods, check state flags

   bool is_null() const { return m_uFlags & (eTableFlagNull32|eTableFlagNull64); }
   bool is_null32() const { return m_uFlags & eTableFlagNull32; }
   bool is_null64() const { return m_uFlags & eTableFlagNull64; }
   bool is_rowstatus() const { return m_uFlags & eTableFlagRowStatus; }
   bool is_rowmeta() const { return m_puMetaData != nullptr; }

   unsigned size_row() const noexcept { return m_uRowSize; }
   unsigned size_row_meta() const noexcept;
   /// get meta block size
   uint64_t size_meta_total() const noexcept { return size_row_meta() * m_uReservedRowCount; }
   /// get meta block size for rows
   uint64_t size_meta_total( uint64_t uRowCount ) const noexcept { return size_row_meta() * uRowCount; }
   /// calc and return total allocated memory size
   uint64_t size_reserved_total() const noexcept { return (m_uRowSize + size_row_meta()) * m_uReservedRowCount; }
   /// calc and return total allocated memory size for rows
   uint64_t size_reserved_total( uint64_t uRowCount ) const noexcept { return (m_uRowSize + size_row_meta()) * uRowCount; }

//@}

/** \name OPERATION
*///@{

   // ## column methods

   /// @name column_add
   /// Add columns to table, this is typically done before adding values to table. Remember to call @see prepare before adding data
   /// Parameters:
   /// - `uColumnType` type of column to add @see: gd::types::enumType
   /// - `stringType` type of columns as string name, will be converted to the type number
   /// - `uSize` if type do not have a fixed size then size will have the maximum length for text
   /// - `columnToAdd` has all column properties for column to add
   /// - `stringName` name for column
   /// - `stringAlias` alias name for column (column can have both name and alias)
   ///@{
   table& column_add( const detail::column& columnToAdd ) { m_pcolumns->add( columnToAdd ); return *this; }
   table& column_add( unsigned uColumnType, const std::string_view& stringName ) { return column_add( uColumnType, 0, stringName ); }
   table& column_add( unsigned uColumnType, unsigned uSize );
   table& column_add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias );
   table& column_add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName ) { return column_add( uColumnType, uSize, stringName, std::string_view{} ); }
   table& column_add( unsigned uColumnType, unsigned uSize, const std::string_view& stringAlias, tag_alias ) { return column_add( uColumnType, uSize, std::string_view{}, stringAlias ); }
   table& column_add( const std::vector< std::tuple< unsigned, unsigned, std::string_view > >& vectorColumn );
   table& column_add( const std::string_view& stringType ) { return column_add( detail::column( (unsigned)gd::types::type_g( stringType ) ) ); }
   table& column_add( const std::string_view& stringType, const std::string_view& stringName ) { return column_add( (unsigned)gd::types::type_g( stringType ), 0, stringName, std::string_view{}); }
   table& column_add( const std::string_view& stringType, unsigned uSize ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize ); }
   table& column_add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringName ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize, stringName, std::string_view{}); }
   table& column_add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringAlias, tag_alias ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize, std::string_view{}, stringAlias); }
   table& column_add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize, stringName, stringAlias); }

   table& column_add( const std::vector< std::pair< std::string_view, unsigned > >& vectorType, tag_type_name );
   table& column_add( const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorType, tag_type_name );
   table& column_add( const std::vector< std::tuple< std::string_view, unsigned, std::string_view, std::string_view > >& vectorType, tag_type_name );
   table& column_add( const std::initializer_list< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name );
   table& column_add( const std::vector< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name );
   table& column_add( const std::vector< std::pair< unsigned, unsigned > >& vectorType, tag_type_constant );
   table& column_add( const table* p_ );
   ///@}

   // ### access column or find index for column/columns
   int column_find_index( const std::string_view& stringName ) const noexcept;
   int column_find_index( const std::string_view& stringAlias, tag_alias ) const noexcept;
   int column_find_index( const std::string_view& stringWildcard, tag_wildcard ) const noexcept;
   unsigned column_get_index( const std::string_view& stringName ) const noexcept;
   unsigned column_get_index( const std::string_view& stringAlias, tag_alias ) const noexcept;
   unsigned column_get_index( const std::string_view& stringName, tag_wildcard ) const noexcept;
   std::vector<uint32_t> column_get_index( std::initializer_list< std::string_view > listName ) const noexcept;
   std::vector<uint32_t> column_get_index( const std::vector< std::string_view >& vectorName ) const noexcept;
   unsigned column_get_index_for_alias( const std::string_view& stringAlias ) const noexcept { return column_get_index( stringAlias, tag_alias{}); }
   unsigned column_get_type( unsigned uIndex ) const { return m_pcolumns->type( uIndex ); }
   std::vector<unsigned> column_get_type( const std::vector<unsigned>& vectorIndex ) const;
   std::vector<unsigned> column_get_type() const;
   unsigned column_get_ctype( unsigned uIndex ) const { return m_pcolumns->ctype( uIndex ); }
   std::vector<unsigned> column_get_ctype() const;
   unsigned column_get_ctype_number( unsigned uIndex ) const { return m_pcolumns->ctype_number( uIndex ); }
   unsigned column_get_size( unsigned uIndex ) const { return m_pcolumns->size( uIndex ); }
   void column_set_size( unsigned uIndex, unsigned uSize ) { m_pcolumns->get( uIndex )->size( uSize ); }
   void column_set_size( const std::string_view& stringName, unsigned uSize ) { column_set_size( column_get_index( stringName ), uSize ); }
   std::string_view column_get_name( unsigned uIndex ) const;
   std::vector<std::string_view> column_get_name() const;
   std::vector<std::string_view> column_get_name(const std::vector<unsigned>& vectorColumn) const;
   std::string_view column_get_alias( unsigned uIndex ) const;
   void column_get( unsigned uIndex, argument::column& column_ ) const;

   /// Rename column
   std::string column_rename( unsigned uColumn, const std::string_view& stringNewName );


   void column_for_each( std::function<void( detail::column&, unsigned )> callback_ );
   void column_for_each( std::function<void( const detail::column&, unsigned )> callback_ ) const;

   // ## fill methods - set 0 or more values in table

   /// @name column_fill
   /// fill specified column with value
   ///@{
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue ) { column_fill(  uColumn, variantviewValue, 0, m_uRowCount ); }
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert ) { column_fill(  uColumn, variantviewValue, 0, m_uRowCount, tag_convert{}); }
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uBeginRow, uint64_t uEndRow );
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uBeginRow, uint64_t uEndRow, tag_convert );
   void column_fill( unsigned uColumn, const gd::variant_view* pvariantviewValue, size_t uCount, uint64_t uBeginRow );
   void column_fill( unsigned uColumn, const std::vector< gd::variant_view >& vectorValue) { column_fill( uColumn, vectorValue.data(), vectorValue.size(), 0 ); }
   void column_fill( unsigned uColumn, const std::vector< gd::variant_view >& vectorValue, uint64_t uBeginRow ) { column_fill( uColumn, vectorValue.data(), vectorValue.size(), uBeginRow ); }
   ///@}

   template< typename... Arguments >
   void column_fill( const std::string_view& stringName, const gd::variant_view& variantviewValue, Arguments&&... arguments );
   template< typename... Arguments >
   void column_fill( const std::string_view& stringName, const std::vector< gd::variant_view >& vectorValue, Arguments&&... arguments );

   auto column_begin() { return m_pcolumns->begin(); }
   auto column_end() { return m_pcolumns->end(); }
   auto column_begin() const { return m_pcolumns->begin(); }
   auto column_end() const { return m_pcolumns->end(); }
   auto column_cbegin() const { return m_pcolumns->cbegin(); }
   auto column_cend() const { return m_pcolumns->cend(); }

   void column_prepare();
   detail::column column_get( std::size_t uIndex ) { return *m_pcolumns->get( uIndex ); }
   const detail::column& column_get( std::size_t uIndex ) const { return *m_pcolumns->get( uIndex ); }
   detail::column* column_get( std::size_t uIndex, tag_pointer ) { return m_pcolumns->get( uIndex ); }
   const detail::column* column_get( std::size_t uIndex, tag_pointer ) const { return m_pcolumns->get( uIndex ); }
   std::size_t column_size() const { return m_pcolumns->size(); }
   bool column_empty() const { return m_pcolumns == nullptr || m_pcolumns->empty(); }
   void column_clear() { m_pcolumns->clear(); }

   bool column_exists( const std::string_view& stringName ) const noexcept;
   bool column_exists( const std::string_view& stringAlias, tag_alias ) const noexcept;

   /// return collection object wrapping columns
   gd::table::columns<table> columns() { return gd::table::columns<gd::table::table>( this ); }


   /// Prepares table for use, this has to be called before adding values to table
   std::pair<bool, std::string> prepare();


   // ## row methods, row related functionality 

   void row_set_state( uint64_t uRow, unsigned uFlags ) { assert( uRow < m_uReservedRowCount ); *row_get_state( uRow ) = uFlags; }
   void row_set_state( uint64_t uRow, unsigned uSet, unsigned uClear ); 
   uint8_t* row_get( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); return m_puData + uRow * m_uRowSize; }
   uint8_t* row_get_meta( uint64_t uRow ) const noexcept { return row_get_null( uRow ); }
   /// return pointer to section holding null column information
   uint8_t* row_get_null( uint64_t uRow ) const noexcept;
   /// Get pointer to row state part
   uint32_t* row_get_state( uint64_t uRow ) const noexcept;
   /// if row is in used (when state information is used for row)
   bool row_is_use( uint64_t uRow ) const noexcept;
   /// Get pointer to row part used to mark null columns
   uint64_t* row_get_null_columns( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); return reinterpret_cast<uint64_t*>(m_puData + uRow * m_uRowSize); }


   // ### edit rows (add or remove)

   void row_add( uint64_t uCount );
   void row_add() { row_add( 1 ); }
   void row_add( uint64_t uCount, tag_null );
   void row_add(tag_null) { row_add( 1, tag_null{} ); }

   /// @name row_add
   /// add row/rows to table and insert values to added row
   ///@{
   void row_add( const std::initializer_list<gd::variant_view>& vectorValue );
   void row_add( const std::initializer_list<gd::variant_view>& vectorValue, tag_convert );
   void row_add( const std::vector<gd::variant_view>& vectorValue );
   void row_add( const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn );
   void row_add( const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn, tag_convert );
   void row_add( const std::vector<gd::variant_view>& vectorValue, tag_convert );
   void row_add( const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue );
   void row_add( const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue, tag_convert );
   void row_add( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue );
   void row_add( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue, tag_convert );
   void row_add( const gd::argument::arguments& argumentsRow, tag_arguments );
   void row_add( uint64_t uRowToCopy, tag_copy );
   void row_add( unsigned uFirst, const std::string_view& stringRowValue, char chSplit, tag_parse );
   void row_add( const std::string_view& stringRowValue, char chSplit, tag_parse ) { row_add( 0, stringRowValue, chSplit, tag_parse{}); }
   void row_add( const unsigned* puColumn, std::string_view& stringRowValue, char chSplit, tag_parse );
   ///@}

   /// @name row_set                                                                                       {
   /// set values in row
   ///@{
   void row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue );
   void row_set( uint64_t uRow, unsigned uSart, const std::initializer_list<gd::variant_view>& listValue );
   void row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue, tag_convert );
   void row_set( uint64_t uRow, unsigned uSart, const std::initializer_list<gd::variant_view>& listValue, tag_convert );
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue );
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue, tag_convert );
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue, const std::vector<unsigned>& vectorColumn );
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue, const std::vector<unsigned>& vectorColumn, tag_convert );
   void row_set( uint64_t uRow, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue );
   void row_set( uint64_t uRow, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue, tag_convert );
   void row_set( uint64_t uRow, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue );
   void row_set( uint64_t uRow, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue, tag_convert );
   void row_set( uint64_t uRow, const gd::argument::arguments& argumentsRow, tag_arguments );
   void row_set( uint64_t uRow, uint64_t uRowToCopy );
   void row_set( uint64_t uRow, const std::string_view& stringRowValue, char chSplit, tag_parse );
   void row_set( uint64_t uRow, unsigned uFirst, const std::string_view& stringRowValue, char chSplit, tag_parse );
   void row_set( uint64_t uRow, const unsigned* puColumn, const std::string_view& stringRowValue, char chSplit, tag_parse );
   void row_set_null( uint64_t uRow );
   void row_set_null( uint64_t uFrom, uint64_t uCount );
   void row_set_range( uint64_t uRow, const gd::variant_view variantviewSet, tag_convert ) { row_set_range( uRow, 0, get_column_count(), variantviewSet, tag_convert{}); }
   void row_set_range( uint64_t uRow, unsigned uStartColumn, unsigned uCount, const gd::variant_view variantviewSet, tag_convert );
   ///@}

   /// @name row_delete
   /// deletes row/rows in table
   ///@{
   /// Deletes all rows in table (just set the row count to 0)
   void row_delete() { m_uRowCount = 0; }
   ///@}


   /// @name row_reserve_add
   /// reserve memory to store more rows in table
   ///@{
   void row_reserve_add( uint64_t uCount );
   void row_reserve_add() { row_reserve_add( 1 ); }
   ///@}

   row_value_type row_get( uint64_t uRow, tag_cell );
   // row_const_value_type row_get( uint64_t uRow, tag_cell ) const;

   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow ) const;
   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const;
   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow, const std::vector<unsigned>& vectorIndex ) const { return row_get_variant_view( uRow, vectorIndex.data(), (unsigned)vectorIndex.size() ); }
   void row_get_variant_view( uint64_t uRow, std::vector<gd::variant_view>& vectorValue ) const;
   void row_get_variant_view( uint64_t uRow, const unsigned* puIndex, unsigned uSize, std::vector<gd::variant_view>& vectorValue ) const;
   void row_get_variant_view( uint64_t uRow, const std::vector<unsigned>& vectorIndex, std::vector<gd::variant_view>& vectorValue ) const { row_get_variant_view( uRow, vectorIndex.data(), vectorIndex.size(), vectorValue ); }

   int64_t row_get_variant_view( unsigned uColumn, const gd::variant_view& variantviewFind, std::vector<gd::variant_view>& vectorValue ) const;

   /// @name get values in row packed in arguments object
   /// reserve memory to store more rows in table
   ///@{
   void row_get_arguments( uint64_t uRow, gd::argument::arguments& argumentsValue ) const;
   gd::argument::arguments row_get_arguments( uint64_t uRow ) const { gd::argument::arguments a_; row_get_arguments( uRow, a_ ); return a_; }
   gd::argument::arguments row_get_arguments( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const;
   gd::argument::arguments row_get_arguments( uint64_t uRow, const std::vector<unsigned>& vectorIndex ) const { return row_get_arguments( uRow, vectorIndex.data(), (unsigned)vectorIndex.size() ); }   
   ///@}

   bool row_for_each( std::function<bool( std::vector<gd::variant_view>&, uint64_t )> callback_ );
   bool row_for_each( std::function<bool( const std::vector<gd::variant_view>&, uint64_t )> callback_ ) const;
   bool row_for_each( uint64_t uFrom, uint64_t uCount, std::function<bool( std::vector<gd::variant_view>&, uint64_t )> callback_ );
   bool row_for_each( uint64_t uFrom, uint64_t uCount, std::function<bool( const std::vector<gd::variant_view>&, uint64_t )> callback_ ) const;
   bool row_for_each( unsigned uColumn, std::function<bool( const gd::variant_view&, uint64_t )> callback_ ) const { return row_for_each( uColumn, 0, get_row_count(), callback_ ); }
   bool row_for_each( unsigned uColumn, uint64_t uFrom, uint64_t uCount, std::function<bool( const gd::variant_view&, uint64_t )> callback_ ) const;

   iterator_row row_begin() { return iterator_row( (uint64_t)0, this ); }
   iterator_row row_end() { return iterator_row( get_row_count(), this); }
   const_iterator_row row_begin() const { return const_iterator_row( (uint64_t)00, this); }
   const_iterator_row row_end() const { return const_iterator_row( get_row_count(), this); }
   const_iterator_row row_cbegin() const { return const_iterator_row( (uint64_t)00, this); }
   const_iterator_row row_cend() const { return const_iterator_row( get_row_count(), this); }

   /// get row index based on row status 
   int64_t row_get_absolute(uint64_t uRelativeRow, unsigned uStatus) const;

   iterator begin() { return row_begin(); }
   iterator end() { return row_end(); }
   const_iterator begin() const { return row_begin(); }
   const_iterator end() const { return row_end(); }
   const_iterator cbegin() const { return row_cbegin(); }
   const_iterator cend() const { return row_cend(); }

   /// return collection object wrapping rows
   gd::table::rows<table> rows() { return gd::table::rows<gd::table::table>( this ); }

   // ## cell methods, cell related functionality

   uint8_t* cell_get( uint64_t uRow, unsigned uColumn ) noexcept;
   const uint8_t* cell_get( uint64_t uRow, unsigned uColumn ) const noexcept;
   uint8_t* cell_get( uint64_t uRow, const std::string_view& stringName ) noexcept;
   uint8_t* cell_get( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) noexcept;
   const uint8_t* cell_get( uint64_t uRow, const std::string_view& stringName ) const noexcept;
   const uint8_t* cell_get( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) const noexcept;
   uint8_t* cell_get( uint64_t uRow, const std::string_view& stringWildcard, tag_wildcard ) noexcept;

   template <typename TYPE>
   TYPE cell_get( uint64_t uRow, unsigned uColumn ) const noexcept;

   bool cell_is_null( uint64_t uRow, unsigned uColumn ) const noexcept;
   const reference* cell_get_reference( uint64_t uRow, unsigned uColumn ) const noexcept;

   gd::variant_view cell_get_variant_view( uint64_t uRow, unsigned uColumn ) const noexcept;
   std::vector< gd::variant_view > cell_get_variant_view( uint64_t uRow, unsigned uFromColumn, unsigned uToColumn ) const;
   std::vector< gd::variant_view > cell_get_variant_view( uint64_t uRow ) const { return cell_get_variant_view( uRow, 0, get_column_count() ); }
   gd::variant_view cell_get_variant_view( uint64_t uRow, const std::string_view& stringName ) const noexcept;
   gd::variant_view cell_get_variant_view( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) const noexcept;
   gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const noexcept { assert(m_uRowCount != 0); return cell_get_variant_view( m_uRowCount -1, stringName ); }
   gd::variant_view cell_get_variant_view( const std::string_view& stringAlias, tag_alias ) const noexcept { assert(m_uRowCount != 0); return cell_get_variant_view( m_uRowCount -1, stringAlias, tag_alias{}); }
   /// return value without any checks, use this if you know about the internals and just need the value
   gd::variant_view cell_get_variant_view( uint64_t uRow, unsigned uColumn, tag_raw ) const noexcept;
   /// get cell value using name or column index, if name then column gets index to speed up the process next time value is returned
   gd::variant_view cell_get_variant_view( uint64_t uRow, std::variant< unsigned, std::string_view >* pvariantColumn ) const noexcept;

   unsigned cell_get_length( uint64_t uRow, unsigned uColumn ) const noexcept;

   void cell_set( uint64_t uRow, unsigned uColumn, const gd::variant_view& variantviewValue );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue );
   void cell_set( uint64_t uRow, const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_alias );
   void cell_set_null( uint64_t uRow, unsigned uColumn );
   void cell_set_null( uint64_t uRow, const std::string_view& stringName );
   void cell_set_not_null( uint64_t uRow, unsigned uColumn );
   void cell_set( uint64_t uRow, unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_convert );
   void cell_set( uint64_t uRow, const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_convert, tag_alias );
   void cell_set( uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue );
   void cell_set( uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue, tag_convert );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const std::vector<gd::variant_view>& vectorValue ) { cell_set( uRow, column_get_index(stringName), vectorValue ); }
   void cell_set( uint64_t uRow, const std::string_view& stringName, const std::vector<gd::variant_view>& vectorValue, tag_convert ) { cell_set( uRow, column_get_index(stringName), vectorValue, tag_convert{}); }

   void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue ) { assert(m_uRowCount != 0); cell_set( m_uRowCount - 1, uColumn, variantviewValue ); }
   void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue ) { assert(m_uRowCount != 0); cell_set( m_uRowCount - 1, stringName, variantviewValue ); }
   void cell_set( const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_alias ) { assert(m_uRowCount != 0); cell_set( m_uRowCount - 1, stringAlias, variantviewValue, tag_alias{}); }

   void cell_set( const range& rangeSet, const gd::variant_view& variantviewValue );
   void cell_set( const range& rangeSet, const gd::variant_view& variantviewValue, tag_convert );

   // ## find methods

   int64_t find( unsigned uColumn, const gd::variant_view& variantviewFind ) const noexcept { return find( uColumn, 0, get_row_count(), variantviewFind ); }
   int64_t find( const std::string_view& stringName, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( stringName, 0, get_row_count(), variantviewFind ); }
   int64_t find( unsigned uColumn, bool bAscending, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( uColumn, bAscending, 0, get_row_count(), variantviewFind ); }
   int64_t find( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;
   
   int64_t find_variant_view( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;
   int64_t find_variant_view( const std::string_view& stringName, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;
   int64_t find_variant_view( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind, tag_meta ) const noexcept;
   int64_t find_variant_view( unsigned uColumn, bool bAscending, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;
   int64_t find_variant_view( unsigned uColumn, bool bAscending, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( uColumn, bAscending, 0, get_row_count(), variantviewFind ); }
   range find_variant_view( unsigned uColumn, bool bAscending, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind, tag_range ) const noexcept;
   int64_t find_variant_view( unsigned uColumn, const gd::variant_view& variantviewFind, tag_meta ) const noexcept { return find_variant_view( uColumn, 0, get_row_count(), variantviewFind, tag_meta{}); }
   int64_t find_variant_view( unsigned uColumn, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( uColumn, 0, get_row_count(), variantviewFind); }
   range find_variant_view( unsigned uColumn, bool bAscending, const gd::variant_view& variantviewFind, tag_range ) const noexcept { return find_variant_view( uColumn, bAscending, 0, get_row_count(), variantviewFind, tag_range{}); }

   size_t find_all(uint64_t uStartRow, uint64_t uCount, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorFilter, std::vector<uint64_t>* pvectorResult ) const;
   std::vector<uint64_t> find_all(uint64_t uStartRow, uint64_t uCount, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorFilter ) const;
   std::vector<uint64_t> find_all( const std::vector< std::pair<unsigned, gd::variant_view> >& vectorFilter ) const;
   std::vector<uint64_t> find_all( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorFilter ) const;


   /// Find first row marked as free (flag `eRowStateUse` is not used)
   int64_t find_first_free_row( uint64_t uStartRow ) const;
   int64_t find_first_free_row() const { return find_first_free_row( 0 ); }

   /// counts rows in use
   uint64_t count_used_rows() const;
   /// count number of free rows
   uint64_t count_free_rows() const;



   // ## property methods for table - set or get property values for table and other property methods

   template<typename TYPE>
   void property_set( const std::string_view& stringName, TYPE value_ ) { m_argumentsProperty.set( stringName, value_ ); }
   void property_set( const std::pair< std::string, gd::variant_view>& pair_ ) { m_argumentsProperty.set( pair_.first, pair_.second ); }
   gd::argument::arguments::argument property_get( const std::string_view& stringName ) const { return m_argumentsProperty.get_argument( stringName ); }
   bool property_exists( const std::string_view& stringName ) const { return (m_argumentsProperty.find( stringName ) != nullptr); }

   /// @name append
   /// append row data from one table into this table
   ///@{
   void append( const table& tableFrom );
   void append( const table& tableFrom, tag_convert );
   void append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom );
   void append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, tag_convert );
   void append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo );
   void append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo, tag_convert );
   void append( const table& tableFrom, const unsigned* puColumnIndexFrom, const unsigned* puColumnIndexTo, unsigned uColumnCount );
   void append( const table& tableFrom, const unsigned* puColumnIndexFrom, const unsigned* puColumnIndexTo, unsigned uColumnCount, tag_convert );
   void append( const table& tableFrom, tag_name );
   void append( const table& tableAppend, tag_name, tag_convert );
   void append( const table& tableFrom, uint64_t uFrom, uint64_t uCount );
   void append( const table_column_buffer& tableFrom, uint64_t uFrom, uint64_t uCount );
   ///@}

   /// clears all internal data in table, like a reset (columns are also deleted) 
   void clear();
   /// check if table is empty, don't have and data in table rows
   bool empty() const noexcept { return (m_puData == nullptr || m_uRowSize == 0); }
   /// checks if table isn't even initialized (not able to store data)
   bool empty( tag_raw ) const noexcept { return (m_uRowSize == 0 || m_puData == nullptr); }

   /// @name equal operations, compare table or parts of table with the `equal` (= operator)
   /// append row data from one table into this table
   ///@{
   bool equal( const table& tableEqualTo, uint64_t uBeginRow, uint64_t uCount ) const noexcept;
   bool equal( const table& tableEqualTo ) const noexcept { return equal( tableEqualTo, 0, get_row_count() ); }
   ///@}

   /// harvest, read, copy (what word is best ?)

   /// @name harvest values from table into other type of container objects
   /// 
   ///@{
   template <typename TYPE>
   std::vector<TYPE> harvest( uint64_t uRow, unsigned uColumn, unsigned uCount, tag_row ) const noexcept;
   template <typename TYPE>
   std::vector<TYPE> harvest( uint64_t uRow, const std::string_view& stringName, unsigned uCount, tag_row ) const noexcept { return harvest<TYPE>( uRow, column_get_index( stringName ), uCount, tag_row{} ); }
   template<typename TYPE>
   std::vector< TYPE > harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount ) const;
   template<typename TYPE>
   std::vector< TYPE > harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount, tag_null ) const;
   template<typename TYPE>
   std::vector< TYPE > harvest( const std::string_view& stringColumnName, uint64_t uBeginRow, uint64_t uEndRow ) const { return harvest<TYPE>( column_get_index(stringColumnName), uBeginRow, uEndRow); }
   template<typename TYPE>
   std::vector< TYPE > harvest( unsigned uColumn ) const { return harvest<TYPE>( uColumn, (uint64_t)0, get_row_count() ); }
   template<typename TYPE>
   std::vector< TYPE > harvest( const std::string_view& stringColumnName ) const { return harvest<TYPE>( column_get_index(stringColumnName), ( uint64_t )0, get_row_count()); }
   template<typename TYPE>
   std::vector< TYPE > harvest( const std::string_view& stringColumnName, tag_null ) const { return harvest<TYPE>( column_get_index(stringColumnName), ( uint64_t )0, get_row_count(), tag_null{}); }

   /// harvest row values into vector with arguments
   void harvest( uint64_t uBeginRow, uint64_t uCount, std::vector<gd::argument::arguments>& vectorArguments ) const;
   void harvest( std::vector<gd::argument::arguments>& vectorArguments ) const { harvest( 0, get_row_count(), vectorArguments ); }
   std::vector<gd::argument::arguments> harvest( tag_arguments ) const { std::vector<gd::argument::arguments> v_; harvest( 0, get_row_count(), v_ ); return v_; }
   void harvest( const std::vector<uint64_t>& vectorRow, std::vector<gd::argument::arguments>& vectorArguments ) const;
   void harvest( const std::vector<uint64_t>& vectorRow, std::vector< std::vector<gd::variant_view> >& vectorRowValue ) const;
   void harvest( const std::vector<uint64_t>& vectorRow, table& tableHarvest );

   void harvest( const std::vector<uint64_t>& vectorRow, dto::table& tableHarvest ) const;

   void harvest( const std::vector< unsigned >& vectorColumn, const std::vector<uint64_t>& vectorRow, dto::table& tableHarvest ) const;
   void harvest( const std::vector< std::string_view >& vectorColumnName, const std::vector<uint64_t>& vectorRow, dto::table& tableHarvest ) const;


   /// gd::argument::arguments harvest( uint64_t uRow, tag_arguments ) const;
   /// void harvest( uint64_t uRow, gd::argument::arguments& arguments ) const;
   /// void harvest( uint64_t uRow, unsigned* puColumnIndex, unsigned uCount, gd::argument::arguments& arguments ) const;
   /// gd::argument::arguments harvest( uint64_t uRow, unsigned* puColumnIndex, unsigned uCount, tag_arguments ) const;
   /// gd::argument::arguments harvest( uint64_t uRow, const std::vector<unsigned>& vectorColumnIndex, tag_arguments ) const;
   /// std::vector< gd::argument::arguments > harvest( uint64_t uBeginRow, uint64_t uEndRow, tag_arguments ) const;
   /// void harvest( uint64_t uBeginRow, uint64_t uEndRow, std::vector< gd::argument::arguments >& vectorArguments ) const;
   /// void harvest( uint64_t uBeginRow, uint64_t uEndRow, const std::vector<unsigned>& vectorColumnIndex, std::vector< gd::argument::arguments >& vectorArguments ) const;
   /// void harvest( uint64_t uBeginRow, uint64_t uEndRow, unsigned* puColumnIndex, unsigned uCount, std::vector< gd::argument::arguments >& vectorArguments ) const;
   /// void harvest( unsigned* puColumnIndex, unsigned uCount, std::vector< gd::argument::arguments >& vectorArguments ) const;
   /// void harvest( const std::vector<unsigned>& vectorColumnIndex, std::vector< gd::argument::arguments >& vectorArguments ) const;
   /// std::vector< gd::argument::arguments > harvest( uint64_t uBeginRow, uint64_t uEndRow, const std::vector<unsigned>& vectorColumnIndex, tag_arguments ) const;
   /// std::vector< gd::argument::arguments > harvest( uint64_t uBeginRow, uint64_t uEndRow, unsigned* puColumnIndex, unsigned uCount, tag_arguments ) const;
   /// std::vector< gd::argument::arguments > harvest( unsigned* puColumnIndex, unsigned uCount, tag_arguments ) const;
   /// std::vector< gd::argument::arguments > harvest( const std::vector<unsigned>& vectorColumnIndex, tag_arguments ) const;
   ///@}
   /// 

   void plant( const table& table, tag_name );
   void plant( const table& table, const std::string_view& stringColumnName );
   void plant( const table& table, uint64_t uFromRow, uint64_t uToRow, tag_name );
   void plant( const table& table, const std::string_view& stringColumnName, uint64_t uFromRow, uint64_t uCount );
   void plant( const table& tablePlant, unsigned uColumnFrom, unsigned uColumnTo, uint64_t uFrom, uint64_t uCount );
   template< typename TYPE >
   void plant( unsigned uColumn, const std::vector< TYPE >& vectorValue, uint64_t uFrom, uint64_t uCount );
   template< typename TYPE >
   void plant( unsigned uColumn, const std::vector< TYPE >& vectorValue );

   template< typename TYPE, typename... Arguments >
   void plant( const std::string_view& stringName, const std::vector< TYPE >& vectorValue, Arguments&&... arguments );

   void plant( unsigned uColumn, const gd::variant_view& variantviewValue );
   void plant( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uFrom, uint64_t uCount );

   void swap( uint64_t uRow1, uint64_t uRow2 );

   // https://github.com/kevinhermawan/sortire

   void sort( unsigned uColumn, bool bAscending, uint64_t uFrom, uint64_t uCount, tag_sort_selection );
   void sort( unsigned uColumn, bool bAscending, uint64_t uFrom, uint64_t uCount, tag_sort_bubble );

   template<typename TAG_ALGORITHM>
   void sort( unsigned uColumn, bool bAscending, TAG_ALGORITHM tag_ ) { sort( uColumn, bAscending, 0, get_row_count(), tag_ ); }
   template<typename TAG_ALGORITHM>
   void sort( unsigned uColumn, TAG_ALGORITHM tag_ ) { sort( uColumn, true, 0, get_row_count(), tag_); }
   template<typename TAG_ALGORITHM>
   void sort( const std::string_view& stringColumnName, TAG_ALGORITHM tag_ ) { sort( column_get_index(stringColumnName), true, 0, get_row_count(), tag_); }
   template<typename TAG_ALGORITHM>
   void sort( const std::string_view& stringColumnName, bool bAscending, TAG_ALGORITHM tag_ ) { sort( column_get_index(stringColumnName), bAscending, 0, get_row_count(), tag_); }
   template<typename TAG_ALGORITHM>
   void sort( const std::string_view& stringColumnName, bool bAscending, uint64_t uFrom, uint64_t uCount, TAG_ALGORITHM tag_ ) { sort( column_get_index(stringColumnName), bAscending, uFrom, uCount, tag_); }

/** \name RANGE
* Range operations for selected parts in table
*///@{
   /// get range object for all cells in column
   range range_column( unsigned uColumn ) { return range( 0, uColumn, get_row_count() - 1, uColumn ); }
   range range_column( const std::string_view& stringColumnName  ) { return range_column( column_get_index(stringColumnName) ); }
   /// get range object for all cells in row
   range range_row( uint64_t uRow ) { return range( uRow, 0, uRow, get_column_count() - 1 ); }
//@}

   table split( range rangeSplit ) const;
   void split( uint64_t uRowCount, std::vector<table>& vectorSplit ) const;

/** \name ERASE
* Erase parts in table
*///@{
   /// Erase from row index and number of rows from that row
   void erase( uint64_t uFrom, uint64_t uCount );
   /// Erase selected row
   void erase( uint64_t uRow ) { erase( uRow, 1 ); }
//@}



//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   uint8_t* m_puData = nullptr;        ///< data to hold values in table
   uint8_t* m_puMetaData = nullptr;    ///< data block with row meta information
   unsigned m_uFlags;                  ///< state information for table
   unsigned m_uRowSize;                ///< row size in bytes
   unsigned m_uRowMetaSize;            ///< meta data size in bytes for each row
   unsigned m_uRowGrowBy = eSpaceRowGrowBy;///< if table needs more space, this holds number of rows to grow by
   uint64_t m_uRowCount;               ///< row count (row count * row size = total amount of bytes allocated)
   uint64_t m_uReservedRowCount;       ///< reserved row count, max number of rows that can be placed in allocated memory
   gd::argument::arguments m_argumentsProperty; ///< table properties
   references m_references;            ///< Stores blob data
   detail::columns* m_pcolumns;

#ifndef NDEBUG
   uint64_t m_uAllocatedBlockSize_d = 0;
#endif // NDEBUG



// ## free functions ------------------------------------------------------------
public:
   /// Create columns object on heap
   static detail::columns* new_columns_s();
   static detail::columns* new_columns_s( const detail::columns* pcolumnsFrom, unsigned* puColumn, unsigned uSize );

   // ## modify null flag for row
   inline static void cell_set_null_s( uint8_t* puRow, unsigned uIndex ) { assert( uIndex < 64 ); *(uint64_t*)puRow |= (1ULL << uIndex); }
   inline static void cell_set_not_null_s( uint8_t* puRow, unsigned uIndex ) { assert( uIndex < 64 ); *(uint64_t*)puRow &= ~(1ULL << uIndex); }
   inline static bool cell_is_null_s( uint8_t* puRow, unsigned uIndex ) { assert( uIndex < 64 ); return (*(uint64_t*)puRow & (1ULL << uIndex)) != 0; }

   // ## convert names to column index in table
   std::vector<int32_t> column_get_index_s( const table& tablecolumnbuffer, const std::vector<std::string>& vectorName );

   // ## match columns beteen tables

   /// return column indexes for tables that match based on name
   static std::vector< std::pair< unsigned, unsigned > > column_match_s( const table& t1_, const table& t2_, tag_name );
   /// return column indexes for tables that match based on alias
   static std::vector< std::pair< unsigned, unsigned > > column_match_s( const table& t1_, const table& t2_, tag_alias );
   /// return match between two vectors containing string_view objects, this for more general use where table need to match for external objects
   static std::vector< std::pair< unsigned, unsigned > > column_match_s( const std::vector<std::string_view>& v1_, const std::vector<std::string_view>& v2_ );
   /// fill vectors with indexes to columns that have same names
   static void column_match_s( const table& t1_, const table& t2_, std::vector<unsigned>* pvector1, std::vector<unsigned>* pvector2, tag_name );


};

inline void table::common_construct( table&& o ) noexcept {            assert( m_puData == nullptr );
   m_uFlags          = o.m_uFlags; 
   m_uRowSize        = o.m_uRowSize;  
   m_uRowMetaSize    = o.m_uRowMetaSize;
   m_uRowCount       = o.m_uRowCount; 
   m_uReservedRowCount = o.m_uReservedRowCount;
   m_puData          = o.m_puData; o.m_puData = nullptr;
   m_puMetaData      = o.m_puMetaData; o.m_puMetaData = nullptr;
   m_pcolumns        = o.m_pcolumns;
   o.m_pcolumns      = nullptr;
   m_references      = std::move( o.m_references );
   m_argumentsProperty = std::move( o.m_argumentsProperty );
#ifndef NDEBUG
   m_uAllocatedBlockSize_d = o.m_uAllocatedBlockSize_d;
#endif // NDEBUG

}

/** ---------------------------------------------------------------------------
 * @brief operator to set columns for table
 * You need to make sure that internal data isn't messed up with this, columns
 * decide how data is retrieved
 * @param pcolumns pointer to columns object set to table
*/
inline void table::operator=( detail::columns* pcolumns ) {
   if( m_pcolumns != nullptr ) { m_pcolumns->release(); }
   pcolumns->add_reference();
   m_pcolumns = pcolumns;
}

/** ---------------------------------------------------------------------------
 * @brief return column type for all columns in table
 * @return std::vector<unsigned> vector with column type for all columns
 */
inline std::vector<unsigned> table::column_get_type() const {
   std::vector<unsigned> vectorType;
   for(const auto it : *m_pcolumns) { vectorType.push_back( it.type() ); }
   return vectorType;
}

/** ---------------------------------------------------------------------------
 * @brief return column type for all columns in table
 * @return std::vector<unsigned> vector with column type for all columns
 */
inline std::vector<unsigned> table::column_get_ctype() const {
   std::vector<unsigned> vectorCType;
   for(const auto it : *m_pcolumns) { vectorCType.push_back( it.ctype() ); }
   return vectorCType;
}


/** ---------------------------------------------------------------------------
 * @brief Get number of bytes used to store meta information for each row
 * @return unsigned bytes needed to store meta information for row
*/
inline unsigned table::size_row_meta() const noexcept {
   unsigned uMetaDataSize = 0;
   if( is_null32() == true )      uMetaDataSize += eSpaceNull32Columns;
   else if( is_null64() == true ) uMetaDataSize += eSpaceNull64Columns;

   if( is_rowstatus() == true )   uMetaDataSize += eSpaceRowState;

   return uMetaDataSize;
}

/// prepare to store column information
inline void table::column_prepare() {
   if( m_pcolumns == nullptr ) m_pcolumns = new_columns_s();
}


/** ---------------------------------------------------------------------------
 * @brief Add row to table (note that table has "taken" rows and reserved or allocated rows)
 * 
 * Add row/rows to table, if number of total rows need larger memory block table will grow
 * with "grow by" member or if "grow by" member is 0 it will grow by adding 50% to
 * total amount of rows.
 * 
 * @param uCount number of rows to add
*/
inline void table::row_add( uint64_t uCount ) { 
   m_uRowCount += uCount; 
   if( m_uRowCount > m_uReservedRowCount ) {
      uint64_t uAddRowCount = m_uRowCount - m_uReservedRowCount;               // number of rows to grow
      if( m_uRowGrowBy == 0 ) { uAddRowCount += m_uRowCount / 2; }             // add 50% extra rows
      else                    { uAddRowCount += m_uRowGrowBy; }                // add with grow by
      row_reserve_add( uAddRowCount );                                         // increase memory block
   }
}

/** ---------------------------------------------------------------------------
 * @brief Add row to table and set columns in added row to null
 * @param uCount number of rows to add
*/
inline void table::row_add( uint64_t uCount, tag_null ) {                                          assert( is_null() == true );
   auto uBegin = m_uRowCount;
   row_add( uCount );
   row_set_null( uBegin, m_uRowCount - uBegin );
}


/** ---------------------------------------------------------------------------
 * @brief get column name for column index
 * @param uIndex index to column name is returned for if column has name
 * @return std::string_view with column name, empty string if column do not have a name 
*/
inline std::string_view table::column_get_name( unsigned uIndex ) const {      assert( uIndex < m_pcolumns->size() );
   return m_pcolumns->name( uIndex );
}

/** ---------------------------------------------------------------------------
 * @brief Returns names for columns that has name in vector, empty string if no name 
 * @return std::vector<std::string_view> vector with column names
*/
inline std::vector<std::string_view> table::column_get_name() const {
   std::vector<std::string_view> vectorName;
   for( unsigned uColumn = 0; uColumn < get_column_count(); uColumn++ ) {
      vectorName.push_back( column_get_name( uColumn ) );
   }
   return vectorName;
}

/** ---------------------------------------------------------------------------
* @brief Returns names for columns that has name in vector, empty string if no name 
* @param vectorColumn column indexes names are returned for
* @return std::vector<std::string_view> vector with names
*/
inline std::vector<std::string_view> table::column_get_name(const std::vector<unsigned>& vectorColumn) const {
   std::vector<std::string_view> vectorName;
   for(auto uColumn : vectorColumn) {                                                              assert( uColumn < get_column_count() );
      vectorName.push_back( column_get_name( uColumn ) );
   }
   return vectorName;
}


/** ---------------------------------------------------------------------------
 * @brief get column alias for column index
 * @param uIndex index to column alias is returned for if column has name
 * @return std::string_view with column name, empty string if column do not have alias
*/
inline std::string_view table::column_get_alias( unsigned uIndex ) const {                         assert( uIndex < m_pcolumns->size() );
   return m_pcolumns->alias( uIndex );
}

/** ---------------------------------------------------------------------------
 * @brief get column information and place it in `argument::column` object
 * @param uIndex index to column reading information about
 * @param column_ reference to column where information is placed
 */
inline void table::column_get(unsigned uIndex, argument::column& column_) const {                  assert( uIndex < m_pcolumns->size() );
   m_pcolumns->get( uIndex )->get( column_ );
}

/** ---------------------------------------------------------------------------
* @brief Fill column with value
* @param stringName column name to fill with value
* @param variantviewValue value to fill
* @param arguments extra arguments to set to and from rows
*/
template< typename... Arguments >
void table::column_fill( const std::string_view& stringName, const gd::variant_view& variantviewValue, Arguments&&... arguments ) {
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   column_fill( uColumnIndex, variantviewValue, std::forward< Arguments >(arguments)... );         // select proper method to fill column with value
}

/** ---------------------------------------------------------------------------
* @brief Fill column with value
* @param stringName column name to fill with value
* @param vectorValue values to add to column
* @param arguments extra arguments to set to and from rows
*/
template< typename... Arguments >
void table::column_fill( const std::string_view& stringName, const std::vector< gd::variant_view >& vectorValue, Arguments&&... arguments ) {
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   column_fill( uColumnIndex, vectorValue, std::forward< Arguments >(arguments)... );              // select proper method to fill column with values from vector
}

/** ---------------------------------------------------------------------------
 * @brief Return pointer to row null value section (flags in metadata marking null values)
 * @param uRow index for row null value is returned for
 * @return uint8_t* pointer to row null value section
*/
inline uint8_t* table::row_get_null( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( m_puMetaData != nullptr ); 
   return reinterpret_cast<uint8_t*>( m_puMetaData + (uRow * m_uRowMetaSize) );
}

/** ---------------------------------------------------------------------------
 * @brief get position in buffer to row state information for row at index
 * @param uRow index to row where state is located
 * @return uint32_t* pointer to position in internal buffer for row state
*/
inline uint32_t* table::row_get_state( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( is_rowstatus() == true ); 
   // calculate number of bytes used to store flags for culumns marked as null (cant be over sizeof(uint32_t) * 2 or 8 bytes)
   // note that state cant be set to both 32 and 64 columns
   unsigned uNullSize = (m_uFlags & (eTableFlagNull32|eTableFlagNull64)) * sizeof(uint32_t);     assert( uNullSize <= (sizeof(uint32_t) * 2) );
   return reinterpret_cast<uint32_t*>( m_puMetaData + (uRow * m_uRowMetaSize) + uNullSize ); // return pointer to state value
}

/** ---------------------------------------------------------------------------
 * @brief set and clear row state flags
 * @param uRow index for row state is modified for
 * @param uSet flags set to row
 * @param uClear flags cleared
*/
inline void table::row_set_state( uint64_t uRow, unsigned uSet, unsigned uClear ) { assert( uRow < m_uReservedRowCount ); 
   uint32_t* puFlags = row_get_state( uRow );
   *puFlags |= uSet;
   *puFlags &= ~uClear;
}

/** ---------------------------------------------------------------------------
 * @brief check if row is in use
 * @param uRow index to row where state is located that is checked for use
 * @return bool true if row is used, false if not
*/
inline bool table::row_is_use( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( is_rowstatus() == true ); 
   // calculate number of bytes used to store flags for culumns marked as null (cant be over sizeof(uint32_t) * 2 or 8 bytes)
   // note that state cant be set to both 32 and 64 columns
   unsigned uNullSize = (m_uFlags & (eTableFlagNull32|eTableFlagNull64)) * sizeof(uint32_t);     assert( uNullSize <= (sizeof(uint32_t) * 2) );
   return (*reinterpret_cast<uint32_t*>( m_puMetaData + (uRow * m_uRowMetaSize) + uNullSize ) & (uint32_t)eRowStateUse) == (uint32_t)eRowStateUse; // return if row is used
}


/** ---------------------------------------------------------------------------
 * @brief set all columns to null in row
 * @param uRow index to row where values are set to null
*/
inline void table::row_set_null( uint64_t uRow ) { assert( uRow < m_uReservedRowCount ); assert( is_null() == true );
   auto puRow = row_get_null( uRow );

   if( is_null32() ) *(uint32_t*)puRow =((uint32_t)-1);
   else              *(uint64_t*)puRow =((uint64_t)-1);
}

/** ---------------------------------------------------------------------------
 * @brief Set all values in row to null
 * @param uFrom start row to set all values to null
 * @param uCount number of sequential rows to set to null
*/
inline void table::row_set_null( uint64_t uFrom, uint64_t uCount ) { assert( (uFrom + uCount) <= get_row_count() );
   for( auto u = uFrom, uMax = (uFrom + uCount); u < uMax; u++ ) row_set_null( u );
}

/** ---------------------------------------------------------------------------
 * @brief set row values from string that is split into parts from selected character
 * @param uRow row in table were data are placed
 * @param stringRowValue string with row data
 * @param chSplit character used to split string into parts
 */
inline void table::row_set(uint64_t uRow, const std::string_view& stringRowValue, char chSplit, tag_parse) {
   row_set( uRow, 0u, stringRowValue, chSplit, tag_parse{} );
}

/** ---------------------------------------------------------------------------
 * @brief If you know the type value in column and it is not null then this is very fast to return exact value
 * @param uRow index for row where cell values is found
 * @param uColumn index to column in row value is returned
 * @return TYPE value returned that will have the specified type
*/
template <typename TYPE>
TYPE table::cell_get( uint64_t uRow, unsigned uColumn ) const noexcept {
   return *(TYPE*)cell_get( uRow, uColumn );
}

/** ---------------------------------------------------------------------------
 * @brief Check if cell is null
 * @param uRow row for cell
 * @param uColumn index for cell column
 * @return true if null, false if not null
*/
inline bool table::cell_is_null( uint64_t uRow, unsigned uColumn ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   uint64_t uNullRow = 0;
   auto puRow = row_get_null( uRow );
   if( is_null32() ) uNullRow = (uint64_t)*(uint32_t*)puRow;
   else              uNullRow = *(uint64_t*)puRow;
   
   return (uNullRow & (1ULL << uColumn)) != 0;
}

/** ---------------------------------------------------------------------------
 * @brief Set value in column to null (marks null flag for column)
 * @param uRow row where cell is
 * @param uColumn cell column
*/
inline void table::cell_set_null( uint64_t uRow, unsigned uColumn ) { assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   auto puRow = row_get_null( uRow );

#ifdef _DEBUG
   uint64_t uNull_d = 0;
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

   if( is_null32() ) *(uint32_t*)puRow |= ((uint32_t)1 << uColumn);
   else              *(uint64_t*)puRow |= ((uint64_t)1 << uColumn);

#ifdef _DEBUG
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

}


/** ---------------------------------------------------------------------------
 * @brief Set value in column to null (marks null flag for column) 
 * @param uRow row where cell is
 * @param stringName cell column name
*/
inline void table::cell_set_null( uint64_t uRow, const std::string_view& stringName ) { assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   unsigned uColumnIndex = column_get_index( stringName ); 
   cell_set_null( uRow, uColumnIndex);
}


inline void table::cell_set_not_null( uint64_t uRow, unsigned uColumn ) { 
                                                                                                   assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   auto puRow = row_get_null( uRow );

#ifdef _DEBUG
   uint64_t uNull_d = 0;
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

   if( is_null32() ) *(uint32_t*)puRow &= ~((uint32_t)1 << uColumn);
   else              *(uint64_t*)puRow &= ~((uint64_t)1 << uColumn);

#ifdef _DEBUG
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

}

/** ---------------------------------------------------------------------------
 * @brief destruct table
 * Column information is stored in shared object, reference counter is decreased 
*/
inline table::~table() {
   if( m_pcolumns != nullptr ) m_pcolumns->release();                          // release columns information
   delete[] m_puData;
}

/** ---------------------------------------------------------------------------
 * @brief Append to table from another table and select from that table what columns that are added
 * @param tableFrom table data is appended from
 * @param vectorColumnIndex list of column index values used to append from
*/
inline void table::append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom ) { assert( vectorColumnIndexFrom.size() <= get_column_count() );
   std::vector<unsigned> vectorColumnIndexTo;
   for( decltype( vectorColumnIndexFrom.size() ) u = 0, uMax = vectorColumnIndexFrom.size(); u < uMax; u++ ) { vectorColumnIndexTo.push_back( (unsigned)u ); }
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), (unsigned)vectorColumnIndexFrom.size() );
}

/** ---------------------------------------------------------------------------
 * @brief Append to table from another table and select from that table what columns that are added
 * @param tableFrom table data is appended from
 * @param vectorColumnIndex list of column index values used to append from
*/
inline void table::append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, tag_convert ) { assert( vectorColumnIndexFrom.size() <= get_column_count() );
   std::vector<unsigned> vectorColumnIndexTo;
   for( decltype( vectorColumnIndexFrom.size() ) u = 0, uMax = vectorColumnIndexFrom.size(); u < uMax; u++ ) { vectorColumnIndexTo.push_back( (unsigned)u ); }
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), (unsigned)vectorColumnIndexFrom.size(), tag_convert{});
}


/** ---------------------------------------------------------------------------
 * @brief Append selected data from "from" table into selected columns in source (this) table
 * @param tableFrom table data is appended from
 * @param vectorColumnIndexFrom index to columns cell values are copied from
 * @param vectorColumnIndexTo index to columns cell values are copied to
*/
inline void table::append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo ) {
   unsigned uColumnCount = (unsigned)std::min<unsigned>( (unsigned)vectorColumnIndexFrom.size(), (unsigned)vectorColumnIndexTo.size() );
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), uColumnCount );
}

/** ---------------------------------------------------------------------------
 * @brief Append selected data from "from" table into selected columns in source (this) table
 * @param tableFrom table data is appended from
 * @param vectorColumnIndexFrom index to columns cell values are copied from
 * @param vectorColumnIndexTo index to columns cell values are copied to
*/
inline void table::append( const table& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo, tag_convert ) {
   unsigned uColumnCount = (unsigned)std::min<unsigned>( (unsigned)vectorColumnIndexFrom.size(), (unsigned)vectorColumnIndexTo.size() );
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), uColumnCount, tag_convert{});
}

/** ---------------------------------------------------------------------------
 * @brief find value in column
 * @param stringName name for column where value are searched
 * @param uStartRow start row
 * @param uCount number of rows to search in
 * @param variantviewFind value to search for
 * @return index to row if value was found, -1 if not found
 */
inline int64_t table::find_variant_view( const std::string_view& stringName, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind) const noexcept {
   unsigned uColumn = column_get_index(stringName);
   return find_variant_view( uColumn, uStartRow, uColumn, variantviewFind );
}

/** ---------------------------------------------------------------------------
 * @brief return vector with values in row. cast to specified type
 * @param uRow row where cells exists
 * @param uColumn first column
 * @param uCount number of columns from first placing values in vector
 * @return vector with cell values from specified row
*/
template <typename TYPE>
std::vector<TYPE> table::harvest( uint64_t uRow, unsigned uColumn, unsigned uCount, tag_row ) const noexcept { assert( (uColumn + uCount) <= get_column_count() ); assert( m_pcolumns->primitive_size( uColumn ) == sizeof(TYPE) );
   std::vector<TYPE> vectorType; // vector that gets values in row
   vectorType.reserve( uCount );
   const TYPE* p_ = (const TYPE*)cell_get( uRow, uColumn );                    // first cell position in row
   for( auto u = 0; u < uCount; u++ ) { 
      vectorType.emplace_back( p_[u] ); 
   }
   return vectorType;
}

/** ---------------------------------------------------------------------------
 * @brief get vector with values based starting from selected row and count of values
 * @param uColumn column index values are taken from
 * @param uFrom start row where harvesting starts
 * @param uCount number of values (rows) to harvest
 * @return vector std::vector< TYPE > vector with harvested values
*/
template<typename TYPE>
inline std::vector< TYPE > table::harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount ) const { 
   std::vector< TYPE > vector_;
   vector_.reserve( uCount );
   auto uEndRow = uFrom + uCount; 
   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = column_get_ctype( uColumn );
   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) )
   {
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         TYPE v_ = (TYPE)cell_get_variant_view( uRow, uColumn );
         vector_.push_back( v_ );
      }
   }
   else
   {
      gd::variant variantConverted;
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         auto VVValue = cell_get_variant_view( uRow, uColumn );
         VVValue.convert_to( eType, variantConverted );
         TYPE v_ = (TYPE)variantConverted;
         vector_.push_back( v_ );
      }
   }

   return vector_;
}

template<>
inline std::vector< std::string > table::harvest( unsigned uColumn, uint64_t uBeginRow, uint64_t uEndRow ) const { assert( uEndRow > uBeginRow );
   std::vector< std::string > vector_;
   vector_.reserve( uEndRow - uBeginRow );
   for( auto uRow = uBeginRow; uRow < uEndRow; uRow++ )
   {
      std::string v_ = cell_get_variant_view( uRow, uColumn ).as_string();
      vector_.push_back( v_ );
   }

   return vector_;
}

/** ---------------------------------------------------------------------------
 * @brief get vector with values based starting from selected row and count of values
 * Method checks for null values, if null values are found they are skipped
 * @param uColumn column index values are taken from
 * @param uFrom start row where harvesting starts
 * @param uCount number of values (rows) to harvest
 * @return vector std::vector< TYPE > vector with harvested values
*/
template<typename TYPE>
inline std::vector< TYPE > table::harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount, tag_null ) const {
   std::vector< TYPE > vector_;
   vector_.reserve( uCount );
   auto uEndRow = uFrom + uCount; 
   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = column_get_ctype( uColumn );
   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) )                    // check if return type is same as calculated type, then no conversion is needed (faster)
   {
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         if(cell_is_null(uRow, uColumn)) continue;
         TYPE v_ = (TYPE)cell_get_variant_view( uRow, uColumn );
         vector_.push_back( v_ );
      }
   }
   else
   {                                                                           // return type do not match column type, we need to convert value to requested type
      gd::variant variantConverted;
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         if(cell_is_null(uRow, uColumn)) continue;
         auto VVValue = cell_get_variant_view( uRow, uColumn );
         VVValue.convert_to( eType, variantConverted );
         TYPE v_ = (TYPE)variantConverted;
         vector_.push_back( v_ );
      }
   }

   return vector_;
}

/** ---------------------------------------------------------------------------
 * @brief harvest selected columns from selected rows into table
 * @param vectorColumnName columns to harvest values from
 * @param vectorRow selected rows to read values from
 * @param tableHarvest table to the put read values from selected rows into
 */
inline void table::harvest(const std::vector< std::string_view >& vectorColumnName, const std::vector<uint64_t>& vectorRow, dto::table& tableHarvest) const {
   std::vector< unsigned > vectorColumn = column_get_index( vectorColumnName );
   harvest( vectorColumn, vectorRow, tableHarvest );
}


template<typename TYPE>
void table::plant( unsigned uColumn, const std::vector< TYPE >& vectorValue, uint64_t uFrom, uint64_t uCount ) { assert( uColumn < get_column_count() ); 
   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = column_get_ctype( uColumn );
   uint64_t uEndRow = uFrom + uCount;
   if( uCount > vectorValue.size() ) uCount = vectorValue.size();
   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) )
   {
      for( uint64_t uIndex = 0; uIndex < uCount; uIndex++ )
      {
         cell_set( uIndex + uFrom, uColumn, vectorValue[uIndex] );
      }
   }
   else
   {
      gd::variant variantConverted;
      for( uint64_t uIndex = 0; uIndex < uCount; uIndex++ )
      {
         cell_set( uIndex + uFrom, uColumn, vectorValue[uIndex], tag_convert{} );
      }
   }
}

template<typename TYPE>
void table::plant( unsigned uColumn, const std::vector< TYPE >& vectorValue ) {
   plant( uColumn, vectorValue, 0, get_row_count() );
}

template< typename TYPE, typename... Arguments >
void table::plant( const std::string_view& stringName, const std::vector< TYPE >& vectorValue, Arguments&&... arguments ) {
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   plant( uColumnIndex, vectorValue, std::forward< Arguments >(arguments)... ); // call matching plant method using index for column
}

inline void table::plant( unsigned uColumn, const gd::variant_view& variantviewValue ) {
   plant( uColumn, variantviewValue, 0, get_row_count() );
}


namespace debug {
   std::string print( const table& table, uint64_t uCount );
   std::string print( const table& table );
   std::string print( const table& table, tag_columns );
   std::string print( const table* ptable, tag_columns );
   std::string print_column( const table* ptable );
   std::string print_row( const table& table, uint64_t uRow );
}

_GD_TABLE_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
