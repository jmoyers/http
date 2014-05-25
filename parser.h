#ifndef __PARSER_H
#define __PARSER_H

#include <string>
#include "log.h"
#include "headers.h"

class Parser
{
  public:
    Parser(const char *buffer, int size);
    Parser(Parser  &p) = delete;
    Parser(Parser &&p) = delete;

    void run();

    std::shared_ptr<Headers> getHeaders() { return m_headers; }
  private:
    const char *m_buffer;
    int m_buffer_size;
    int m_index;

    enum class State
    {
      BROKEN,
      PAUSED,
      METHOD,
      PATH,
      VERSION,
      FIELD,
      DONE
    };

    State m_state;
    std::shared_ptr<Headers> m_headers;

    void parseMethod();
    void parsePath();
    void parseVersion();
    void parseField();

    // convenience
    inline const char & curr();
    inline const char & next();
    inline const char & prev();

};

#endif /** __PARSER_H **/
