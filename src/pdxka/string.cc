/**
 * @file string.cc
 * @author Derek Huang
 * @brief String formatting
 * @copyright MIT License
 */

#include "pdxka/string.hh"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>

namespace pdxka {

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
// GCC warns about n_used possibly being used uninitialized for release builds
// with -03. this seems like a false positive, so we silence the warning
#if defined(__GNUG__) && defined(__OPTIMIZE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif  // !defined(__GNUG__) || !defined(__OPTIMIZE__)
      if (n_used + n_writable <= line_length) {
#if defined(__GNUG__) && defined(__OPTIMIZE__)
#pragma GCC diagnostic pop
#endif  // !defined(__GNUG__) || !defined(__OPTIMIZE__)
        for (auto j = n_written; j < i; j++) stream.put(orig[j]);
        n_used += n_writable;
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
