/**
 * @file string.h
 * @author Derek Huang
 * @brief String formatting
 * @copyright MIT License
 */

#ifndef PDXKA_STRING_H_
#define PDXKA_STRING_H_

#include <cstdint>
#include <string>

namespace pdxka {

std::string line_wrap(
  const std::string& orig, std::size_t line_length, bool hard_wrap = false);

/**
 * Return new string wrapped at 80 columns.
 *
 * Words (white-delimited tokens) will not split across lines unless
 * `hard_wrap` is `true` and the word is longer than `line_length`.
 *
 * @param orig `const std::string&` original string
 * @param hard_wrap `bool` to split a word whose length is longer than
 *  `line_length` across lines if `true`, otherwise allow oveflow
 */
inline std::string line_wrap(const std::string& orig, bool hard_wrap = false)
{
  return line_wrap(orig, 80, hard_wrap);
}

}  // namespace pdxka

#endif  // PDXKA_STRING_H_
