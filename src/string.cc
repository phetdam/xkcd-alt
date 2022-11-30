/**
 * @file string.cc
 * @author Derek Huang
 * @brief String formatting
 * @copyright MIT License
 */

#include "pdxka/string.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>

namespace pdxka {

/**
 * Return new string wrapped at `line_length`.
 *
 * Words (white-delimited tokens) will not split across lines unless
 * `hard_wrap` is `true` and the word is longer than `line_length`.
 *
 * @param orig `const std::string&` original string
 * @param line_length `std::size_t` line length to wrap at
 * @param hard_wrap `bool` to split a word whose length is longer than
 *  `line_length` across lines if `true`, otherwise allow oveflow
 */
std::string line_wrap(
  const std::string& orig, std::size_t line_length, bool hard_wrap)
{
  // output stream + number of characters used in line
  std::stringstream stream;
  std::size_t n_used = 0;
  // size of string, number of characters written to stream
  std::size_t n_chars = orig.size();
  std::size_t n_written = 0;
  // loop through the string + 1
  for (std::size_t i = 0; i <= n_chars; i++) {
    // if past end or we see whitespace (undefined behavior without cast)
    if (i == n_chars || std::isspace(static_cast<unsigned char>(orig[i]))) {
      // number of characters that could be written
      auto n_writable = i - n_written;
      // if there is space in the line, write chars from n_written up to i
      if (n_used + n_writable <= line_length) {
        for (auto j = n_written; j < i; j++) stream.put(orig[j]);
        n_used += n_writable;
        // if the line is not full after writing, add a single space
        // if (n_used < line_length) stream.put(' ');
      }
      // otherwise, no space, so new line. but if word exceeds line_length and
      // we choose to enforce hard wrap (break the word across lines).
      else if (hard_wrap && n_writable > line_length) {
        // increment n_written to "fake" writing the whitespace character
        n_written++;
        stream.put('\n');
        // chunk size; we write in chunks until everything is written
        std::size_t chunk_size;
        // write until end of length, insert newline, continue, and repeat this
        // until all the characters are written. then set n_used
        while (n_written < i) {
          // number of chars to write is either line_length or i - n_written
          chunk_size = std::min(line_length, i - n_written);
          for (auto j = n_written; j < n_written + chunk_size; j++)
            stream.put(orig[j]);
          stream.put('\n');
          n_written += chunk_size;
        }
        // final chunk size is number of characters written in new line
        n_used = chunk_size;
      }
      // no space but either soft wrap or word doesn't exceed line length
      else {
        // increment n_written to "fake" writing the whitespace character
        n_written++;
        stream.put('\n');
        for (auto j = n_written; j < i; j++) stream.put(orig[j]);
        // n_used is just the number of characters written
        n_used = n_writable;
      }
      // update n_written to i
      n_written = i;
    }
    // otherwise, keep scanning the string
  }
  // done with for loop, so return
  return stream.str();
}

}  // namespace pdxka
