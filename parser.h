#ifndef __PARSER_H
#define __PARSER_H

#include <string>
#include "log.h"
#include "headers.h"

class Parser
{
  public:
    const char *m_buffer;
    int m_buffer_size;
    int m_index;

    enum State
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

    Parser(const char *buffer, int size);

    void parseMethod();
    void parsePath();
    void parseVersion();
    void parseField();

    // convenience
    char curr();
    char next();
    char prev();

    void run();

    std::shared_ptr<Headers> getHeaders() { return m_headers; }
};

#endif /** __PARSER_H **/
