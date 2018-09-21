////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_AQL_QUERY_CACHE_H
#define ARANGOD_AQL_QUERY_CACHE_H 1

#include "Aql/QueryString.h"
#include "Basics/Common.h"
#include "Basics/Mutex.h"
#include "Basics/ReadWriteLock.h"

struct TRI_vocbase_t;

namespace arangodb {
namespace velocypack {
class Builder;
}
namespace aql {

/// @brief cache mode
enum QueryCacheMode { CACHE_ALWAYS_OFF, CACHE_ALWAYS_ON, CACHE_ON_DEMAND };

struct QueryCacheResultEntry {
  QueryCacheResultEntry() = delete;

  QueryCacheResultEntry(uint64_t hash, 
                        QueryString const& queryString, 
                        std::shared_ptr<arangodb::velocypack::Builder> const& results,
                        std::vector<std::string> const& dataSources);

  ~QueryCacheResultEntry() = default;

  uint64_t const _hash;
  std::string const _queryString;
  std::shared_ptr<arangodb::velocypack::Builder> _queryResult;
  std::shared_ptr<arangodb::velocypack::Builder> _stats;
  std::vector<std::string> const _dataSources;
  QueryCacheResultEntry* _prev;
  QueryCacheResultEntry* _next;
};

struct QueryCacheDatabaseEntry {
  QueryCacheDatabaseEntry(QueryCacheDatabaseEntry const&) = delete;
  QueryCacheDatabaseEntry& operator=(QueryCacheDatabaseEntry const&) = delete;

  /// @brief create a database-specific cache
  QueryCacheDatabaseEntry();

  /// @brief destroy a database-specific cache
  ~QueryCacheDatabaseEntry();

  /// @brief lookup a query result in the database-specific cache
  std::shared_ptr<QueryCacheResultEntry> lookup(uint64_t hash, QueryString const& queryString);

  /// @brief store a query result in the database-specific cache
  void store(uint64_t hash, std::shared_ptr<QueryCacheResultEntry> entry);

  /// @brief invalidate all entries for the given data sources in the
  /// database-specific cache
  void invalidate(std::vector<std::string> const& dataSources);

  /// @brief invalidate all entries for a data source in the 
  /// database-specific cache
  void invalidate(std::string const& dataSource);

  /// @brief enforce maximum number of results
  /// must be called under the cache's properties lock
  void enforceMaxResults(size_t);

  /// @brief unlink the result entry from the list
  void unlink(QueryCacheResultEntry*);

  /// @brief link the result entry to the end of the list
  void link(QueryCacheResultEntry*);

  /// @brief hash table that maps query hashes to query results
  std::unordered_map<uint64_t, std::shared_ptr<QueryCacheResultEntry>> _entriesByHash;

  /// @brief hash table that contains all data souce-specific query results
  /// maps from data sources names to a set of query results as defined in
  /// _entriesByHash
  std::unordered_map<std::string, std::unordered_set<uint64_t>>
      _entriesByDataSource;

  /// @brief beginning of linked list of result entries
  QueryCacheResultEntry* _head;

  /// @brief end of linked list of result entries
  QueryCacheResultEntry* _tail;

  /// @brief number of elements in this cache
  size_t _numElements;
};

class QueryCache {
 public:
  QueryCache(QueryCache const&) = delete;
  QueryCache& operator=(QueryCache const&) = delete;

  /// @brief create cache
  QueryCache();

  /// @brief destroy the cache
  ~QueryCache();

 public:
  /// @brief return the query cache properties
  arangodb::velocypack::Builder properties();

  /// @brief return the cache properties
  void properties(std::pair<std::string, size_t>& result);

  /// @brief sets the cache properties
  void setProperties(std::pair<std::string, size_t> const& properties);

  /// @brief test whether the cache might be active
  /// this is a quick test that may save the caller from further bothering
  /// about the query cache if case it returns `false`
  bool mayBeActive() const;

  /// @brief return whether or not the query cache is enabled
  QueryCacheMode mode() const;

  /// @brief return a string version of the mode
  static std::string modeString(QueryCacheMode);

  /// @brief lookup a query result in the cache
  std::shared_ptr<QueryCacheResultEntry> lookup(TRI_vocbase_t* vocbase, uint64_t hash, QueryString const& queryString);

  /// @brief store a query in the cache
  /// if the call is successful, the cache has taken over ownership for the
  /// query result!
  void store(TRI_vocbase_t* vocbase, uint64_t hash, QueryString const& queryString,
             std::shared_ptr<arangodb::velocypack::Builder> const& result,
             std::shared_ptr<arangodb::velocypack::Builder> const& stats,
             std::vector<std::string>&& dataSources);
  
  /// @brief store a query cache entry in the cache
  void store(TRI_vocbase_t* vocbase, std::shared_ptr<QueryCacheResultEntry> entry);

  /// @brief invalidate all queries for the given data sources
  void invalidate(TRI_vocbase_t* vocbase, std::vector<std::string> const& dataSources);

  /// @brief invalidate all queries for a particular data source
  void invalidate(TRI_vocbase_t* vocbase, std::string const& dataSource);

  /// @brief invalidate all queries for a particular database
  void invalidate(TRI_vocbase_t* vocbase);

  /// @brief invalidate all queries
  void invalidate();

  /// @brief get the pointer to the global query cache
  static QueryCache* instance();

  /// @brief invalidate all entries in the cache part
  /// note that the caller of this method must hold the write lock
  void invalidate(unsigned int);

  /// @brief sets the maximum number of elements in the cache
  void setMaxResults(size_t);

  /// @brief enable or disable the query cache
  void setMode(QueryCacheMode);

  /// @brief enable or disable the query cache
  void setMode(std::string const&);
  
 private:
  /// @brief enforce maximum number of results in each database-specific cache
  /// must be called under the cache's properties lock
  void enforceMaxResults(size_t);

  /// @brief determine which part of the cache to use for the cache entries
  unsigned int getPart(TRI_vocbase_t const*) const;


 private:
  /// @brief number of R/W locks for the query cache
  static constexpr uint64_t numberOfParts = 8;

  /// @brief protect mode changes with a mutex
  arangodb::Mutex _propertiesLock;

  /// @brief read-write lock for the cache
  arangodb::basics::ReadWriteLock _entriesLock[numberOfParts];

  /// @brief cached query entries, organized per database
  std::unordered_map<TRI_vocbase_t*, std::unique_ptr<QueryCacheDatabaseEntry>>
      _entries[numberOfParts];
};
}
}

#endif
