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
    size_t m_buffer_size;
    size_t m_index;

    std::shared_ptr<Headers> m_headers;

    void parse_method();
    void parse_path();
    void parse_version();
    void parse_field();

    inline const char & curr();
    inline const char & next();
    inline const char & prev();

}; // class

}  // namespace

#endif /** __PARSER_H **/
