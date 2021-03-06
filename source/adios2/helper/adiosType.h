/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * adiosType.h helper functions for types: conversion, mapping, etc.
 *
 *  Created on: May 17, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_HELPER_ADIOSTYPE_H_
#define ADIOS2_HELPER_ADIOSTYPE_H_

/// \cond EXCLUDE_FROM_DOXYGEN
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
/// \endcond

#include "adios2/ADIOSTypes.h"

namespace adios2
{

struct SubFileInfo
{
    /**  from characteristics, first = Start point, second =
    End point of block of data */
    Box<Dims> BlockBox;
    Box<Dims> IntersectionBox; ///< first = Start point, second = End point
    Box<size_t> Seeks;         ///< first = Start seek, second = End seek
};

/**
 * Structure that contains the seek info per variable
 * <pre>
 *   key: subfile index
 *   value: (map)
 *     key: step
 *     value: (vector)
 *       index : block ID within step
 *       value : file seek box: first = seekStart, second = seekCount
 * </pre>
 */
using SubFileInfoMap =
    std::map<size_t, std::map<size_t, std::vector<SubFileInfo>>>;

/**
 * Gets type from template parameter T
 * @return string with type
 */
template <class T>
std::string GetType() noexcept;

/**
 * Check in types set if "type" is one of the aliases for a certain type,
 * (e.g. if type = integer is an accepted alias for "int", returning true)
 * @param type input to be compared with an alias
 * @param aliases set containing aliases to a certain type, typically
 * Support::DatatypesAliases from Support.h
 * @return true: is an alias, false: is not
 */
template <class T>
bool IsTypeAlias(
    const std::string type,
    const std::map<std::string, std::set<std::string>> &aliases) noexcept;

/**
 * Converts a vector of dimensions to a CSV string
 * @param dims vector of dimensions
 * @return comma separate value (CSV)
 */
std::string DimsToCSV(const Dims &dimensions) noexcept;

/**
 * Converts comma-separated values (csv) to a vector of integers
 * @param csv "1,2,3"
 * @return vector<int> = { 1, 2, 3 }
 */
std::vector<int> CSVToVectorInt(const std::string csv) noexcept;

/**
 * Convert a vector of uint64_t element into size_t
 * @param in  uint64_t vector
 * @param out size_t vector with contents of in
 */
void ConvertUint64VectorToSizetVector(const std::vector<uint64_t> &in,
                                      std::vector<size_t> &out) noexcept;

/** Convert a C array of uint64_t elements to a vector of std::size_t elements
 *  @param number of elements
 *  @param input array of uint64_t elements
 *  @param vector of std::size_t elements. It will be resized to nElements.
 */
void Uint64ArrayToSizetVector(const size_t nElements, const uint64_t *in,
                              std::vector<size_t> &out) noexcept;

/** Convert a C array of uint64_t elements to a vector of std::size_t elements
 *  @param number of elements
 *  @param input array of uint64_t elements
 *  @return vector of std::size_t elements
 */
std::vector<size_t> Uint64ArrayToSizetVector(const size_t nElements,
                                             const uint64_t *in) noexcept;

/**
 * Converts a recognized time unit string to TimeUnit enum
 * @param timeUnit string with acceptable time unit
 * @param debugMode true: throw exception if timeUnitString not valid
 * @return TimeUnit enum (int) TimeUnit::s, TimeUnit::ms, etc.
 */
TimeUnit StringToTimeUnit(const std::string timeUnitString,
                          const bool debugMode);

/**
 * Returns the conversion factor from input units Tb, Gb, Mb, Kb, to bytes as a
 * factor of 1024
 * @param units input
 * @param debugMode true: check if input units are valid
 * @return conversion factor to bytes, size_t
 */
size_t BytesFactor(const std::string units, const bool debugMode);

/**
 * Returns open mode as a string
 * @param openMode from ADIOSTypes.h
 * @param oneLetter if true returns a one letter version ("w", "a" or "r")
 * @return string with open mode
 */
std::string OpenModeToString(const Mode openMode,
                             const bool oneLetter = false) noexcept;

template <class T, class U>
std::vector<U> NewVectorType(const std::vector<T> &in);

template <class T, class U>
std::vector<U> NewVectorTypeFromArray(const T *in, const size_t inSize);

template <class T>
constexpr bool IsLvalue(T &&);

/**
 * Inquire for existing key and return value in a map
 * @param input contains key, value pairs
 * @return if found: a pointer to value, else a nullptr
 */
template <class T, class U>
U *InquireKey(const T &key, std::map<T, U> &input) noexcept;

/**
 * Inquire for existing key and return value in an unordered_map
 * @param input contains key, value pairs
 * @return if found: a pointer to value, else a nullptr
 */
template <class T, class U>
U *InquireKey(const T &key, std::unordered_map<T, U> &input) noexcept;

template <class T>
std::string VectorToCSV(const std::vector<T> &input) noexcept;

template <class T>
std::string ValueToString(const T value) noexcept;

} // end namespace adios2

#include "adiosType.inl"

#endif /* ADIOS2_HELPER_ADIOSTYPE_H_ */
