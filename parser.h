#ifndef __PARSER_H
#define __PARSER_H

#include <string>
#include "log.h"
#include "headers.h"

namespace Http
{

class Parser
{
  public:
    enum class State
    {
      BROKEN,
      METHOD,
      PATH,
      VERSION,
      FIELD,
      DONE
    };

    Parser(const char *buffer, size_t size);
    Parser(Parser  &p) = delete;
    Parser(Parser &&p) = delete;

    State parse();

    std::shared_ptr<Headers> get_headers() { return m_headers; }
  protected:
    State m_state;
    const char *m_buffer;
    size_t m_buffer_size = 0;
    size_t m_index = 0;
    size_t m_mark = 0;

    std::shared_ptr<Headers> m_headers;

    bool parse_expect(const char *next);

    void parse_method();
    void parse_path();
    void parse_version();
    void parse_field();

    inline const char & curr() const;
    inline const char & next() const;
    inline const char & prev() const;

    void mark();
    void eat_whitespace();
    void return_to_mark();
    const char *pos() const;
    const char *find_next(const char &) const;
    const char *advance();
    const char *advance(const char *);
    bool eof() const;

}; // class

}  // namespace

#endif /** __PARSER_H **/
