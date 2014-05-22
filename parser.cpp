#include <unordered_map>
#include <string>
#include <iostream>

namespace Multiplexer
{
  class Parser
  {
    public:
      const char *m_buffer;
      int m_buffer_size;
      int m_index;
      
      enum Method
      {
        NONE,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
      };

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

      std::shared_ptr<std::unordered_map<std::string, std::string>> m_headers;
      Method m_method;

      Parser(const char *buffer, int size);
      ~Parser();

      void parseMethod();
      void parsePath();
      void parseVersion();
      void parseField();
      void parseValue();

      char c(); // current
      char f(); // forward 1
      char b(); // back 1

      void start();
  };

  Parser::Parser(const char *buffer, int size)
  {
    m_index = 0;
    m_buffer = buffer;
    m_buffer_size = size;
    m_state = METHOD;
    m_method = NONE;
    m_headers = std::make_shared<std::unordered_map<std::string, std::string>>();
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
    switch (c())
    {
      case 'G': m_method = GET; m_index+=4; break;
      case 'H': m_method = HEAD; m_index+=5; break;
      case 'P': 
        switch (f())
        {
          case 'O': m_method = POST; m_index+=5; break;
          case 'U': m_method = PUT; m_index+=4; break;
          case 'A': m_method = PATCH; m_index+=6; break;
        }
        break;
      case 'D': m_method = DELETE; m_index+=7; break;
      case 'T': m_method = TRACE; m_index+=6; break;
      case 'O': m_method = OPTIONS; m_index+=8; break;
      case 'C': m_method = CONNECT; m_index+=8; break;
    }

    if (m_method == NONE)
    {
      ERR("bad http method: %c", c());
      m_state = BROKEN;
    }
    else
    {
      DEB("http method: %d", (int) m_method);
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

  void Parser::parseField()
  {
    /* 1. Find the first delimter
     * 2. Mark the field name
     * 3. Eat remaining whitespace after delimter
     * 4. Find the newline
     * 5. Mark the field value
     * 6. Abort on null terminator
     */

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

    //m_headers->insert(field, value);

    curr = newline + 1;
    m_index = curr - m_buffer;
  }

  void Parser::parseValue()
  {
    m_index++;
  }

  inline char Parser::c()
  {
    if (m_index < m_buffer_size) return m_buffer[m_index];
    else return '\0';
  }

  inline char Parser::f()
  {
    if (m_index+1 < m_buffer_size) return m_buffer[m_index+1];
    else return '\0';
  }

  inline char Parser::b()
  {
    if (m_index-1 >= 0) return m_buffer[m_index-1];
    else return '\0';
  }

  Parser::~Parser()
  {
  }
}
