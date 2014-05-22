#include <string>
#include "headers.h"

namespace Multiplexer
{
  class Parser
  {
    public:
      const char *m_buffer;
      int m_buffer_size;
      int m_index;

      enum State
      {
        BROKEN,
        METHOD,
        PATH,
        VERSION,
        FIELD,
        VALUE,
        DONE
      };

      State m_state;
      Headers *m_headers;

      Parser(const char *buffer, int size);
      ~Parser();

      void parseMethod();
      void parsePath();
      void parseVersion();
      void parseField();
      void parseValue();

      // convenience
      char curr();
      char next();
      char prev();

      void start();
  };

  Parser::Parser(const char *buffer, int size) :
    m_index(),
    m_buffer(buffer),
    m_buffer_size(size),
    m_state(METHOD)
  {
    m_headers = new Headers();
  }

  void Parser::start()
  {
    DEB("%s", m_buffer);

    while (m_index < m_buffer_size && m_state != DONE)
    {
      switch (m_state)
      {
        case METHOD:  parseMethod();  break;
        case PATH:    parsePath();    break;
        case VERSION: parseVersion(); break;
        case FIELD:   parseField();   break;
        case VALUE:   parseValue();   break;
        default: 
          ERR("parser broken");
          return; 
          break;
      }
    }
  }

  void Parser::parseMethod()
  {
    switch (curr())
    {
      case 'G': m_headers->setMethod(Headers::GET); m_index+=4; break;
      case 'H': m_headers->setMethod(Headers::HEAD); m_index+=5; break;
      case 'P': 
        switch (next())
        {
          case 'O': m_headers->setMethod(Headers::POST); m_index+=5; break;
          case 'U': m_headers->setMethod(Headers::PUT); m_index+=4; break;
          case 'A': m_headers->setMethod(Headers::PATCH); m_index+=6; break;
        }
        break;
      case 'D': m_headers->setMethod(Headers::DELETE); m_index+=7; break;
      case 'T': m_headers->setMethod(Headers::TRACE); m_index+=6; break;
      case 'O': m_headers->setMethod(Headers::OPTIONS); m_index+=8; break;
      case 'C': m_headers->setMethod(Headers::CONNECT); m_index+=8; break;
    }

    if (m_headers->getMethod() == Headers::Method::NONE)
    {
      ERR("bad http method: %c", c());
      m_state = BROKEN;
    }
    else
    {
      DEB("http method: %d", (int) m_headers->getMethod());
      DEB("  get: %d", GET);
      DEB("  post: %d", POST);
      DEB("  put: %d", PUT);
      DEB("  delete: %d", DELETE);
      m_state = PATH;
    }
  }

  void Parser::parsePath()
  {
    const char *curr = m_buffer + m_index;
    const char *newline = strchr(curr, '\n');

    if (newline == NULL)
    {
      m_state = BROKEN;
      return;
    }

    m_index = (newline - curr) + 1;
    m_state = FIELD;
  }

  void Parser::parseVersion()
  {
    m_index++;
  }

  /**
   * Parse the remaining header fields
   *
   * 1. Find the first delimter (: for now)
   * 2. Mark the end of the field name
   * 3. Discard any whitespace
   * 4. Find the newline/end of field
   * 5. Mark the end of the value
   * 
   * BROKEN on null terminator
   */
  void Parser::parseField()
  {

    const char *curr = m_buffer + m_index;
    const char *delim = strchr(curr, ':');

    if (delim == NULL)
    {
      m_state = BROKEN;
      return;
    }

    std::string field(curr, delim - curr);

    curr = delim + 1;

    while (*curr == ' ')
    {
      curr++;
    }

    if (*curr == '\0')
    {
      m_state = BROKEN;
      return;
    }

    char *newline = strchr(curr, '\n');

    if (newline == NULL)
    {
      m_state = BROKEN;
      return;
    }

    std::string value(curr, newline - curr);

    DEB("field: %s", field.c_str());
    DEB("value: %s", value.c_str());

    m_headers->setField(field, value);

    curr = newline + 1;
    m_index = curr - m_buffer;
  }

  void Parser::parseValue()
  {
    m_index++;
  }

  inline char Parser::curr()
  {
    return m_buffer[m_index];
  }

  inline char Parser::next()
  {
    if (m_index+1 < m_buffer_size)
    {
      return m_buffer[m_index+1];
    }
    
    return '\0';
  }

  inline char Parser::prev()
  {
    if (m_index-1 >= 0)
    {
      return m_buffer[m_index-1];
    }

    return '\0';
  }

  Parser::~Parser()
  {
    delete m_headers;
  }
}
