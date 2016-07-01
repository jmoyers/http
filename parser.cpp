#include "parser.h"

namespace Http
{

Parser::Parser(const char *buffer, size_t size) :
  m_index(),
  m_buffer(buffer),
  m_buffer_size(size),
  m_state(State::METHOD)
{
  m_headers = std::make_shared<Headers>();
}

Parser::State Parser::parse()
{
  DEBUG("parsing started for buffer len: %zu", m_buffer_size);
  
  while (m_index < m_buffer_size)
  {
    switch (m_state)
    {
      case State::METHOD:  parse_method();  break;
      case State::PATH:    parse_path();    break;
      case State::VERSION: parse_version(); break;
      case State::FIELD:   parse_field();   break;
      default: 
        DEBUG("parser stopped");
        return m_state;
        break;
    }
  }

  m_state = State::DONE;

  return m_state;
}

/**
 * If we see the string specified by next, we will advance the pointer forward
 * by that number of characters and return true, otherwise, we'll return false
 * and keep the pointer where it was.
 */
bool Parser::parse_expect(const char *next)
{
  mark();

  while (*next && !eof())
  {
    if(*next != curr()) {
      return_to_mark();
      return false;
    }
    next++;
    advance();
  }

  return true;
}

void Parser::parse_method()
{
  if (parse_expect("GET")) {
    m_headers->set_method(Headers::Method::GET);
  } else if (parse_expect("HEAD")) {
    m_headers->set_method(Headers::Method::HEAD);
  } else if (parse_expect("POST")) {
    m_headers->set_method(Headers::Method::POST); 
  } else if (parse_expect("PUT")) {
    m_headers->set_method(Headers::Method::PUT);
  } else if (parse_expect("PATCH")) {
    m_headers->set_method(Headers::Method::PATCH);
  } else if (parse_expect("DELETE")) {
    m_headers->set_method(Headers::Method::DELETE);
  } else if (parse_expect("TRACE")) {
    m_headers->set_method(Headers::Method::TRACE);
  } else if (parse_expect("OPTIONS")) {
    m_headers->set_method(Headers::Method::OPTIONS);
  } else if (parse_expect("CONNECT")) {
    m_headers->set_method(Headers::Method::CONNECT);
  }

  if (m_headers->get_method() == Headers::Method::NONE)
  {
    ERR("bad http method at: %zu %c", m_index, curr());
    m_state = State::BROKEN;
  }
  else
  {
    DEBUG("http method: %d", (int) m_headers->get_method());
    m_state = State::PATH;
  }
}

void Parser::eat_whitespace()
{
  while (!eof() && (curr() == ' ' || curr() == '\r' || curr() == '\n'))
    advance();
}

void Parser::parse_path()
{
  // eat the whitespace before the path
  eat_whitespace();

  DEBUG("parsing path at %zu '%c'", m_index, curr());

  // end of line, includes version
  const char *newline = find_next('\n');

  // end of path
  const char *end_of_path = find_next(' ');

  if (end_of_path == NULL)
  {
    DEBUG("parsing path failed, no whitespace before http version");
    m_state = State::BROKEN;
    return;
  }

  if (newline && end_of_path > newline)
  {
    DEBUG("parsing path failed, newline detected before whitespace");
    m_state = State::BROKEN;
    return;
  }

  std::string path(pos(), end_of_path);
  m_headers->set_path(path);

  m_index = end_of_path - m_buffer;
  m_state = State::VERSION;
}

void Parser::parse_version()
{
  eat_whitespace();

  if (!parse_expect("HTTP/"))
  {
    DEBUG("bad http version, missing HTTP/");
    m_state = State::BROKEN;
    return;
  }

  DEBUG("parsing version at %zu '%c'", m_index, curr());

  mark();

  int major = 0, minor = 0;

  if (!eof())
  {
    major = static_cast<int>(curr()) - 48;
    advance();
  }
  else
  {
    DEBUG("bad http version, missing major version");
    return_to_mark();
    m_state = State::BROKEN;
    return;
  }

  // .
  advance();

  if (!eof())
  {
    minor = static_cast<int>(curr()) - 48;
    advance();
  } 
  else
  {
    DEBUG("bad http version, missing minor version");
    return_to_mark();
    m_state = State::BROKEN;
    return;
  }

  DEBUG("parse http version %d.%d", major, minor);

  m_headers->set_http_version(Headers::Version{major, minor});
  m_state = State::FIELD;
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
void Parser::parse_field()
{
  eat_whitespace();
  mark();
  
  const char *delim = find_next(':');

  if (delim == NULL)
  {
    m_state = State::BROKEN;
    return;
  }

  std::string field(pos(), delim);

  advance(delim + 1);

  eat_whitespace();

  if (eof()) {
    m_state = State::BROKEN;
    return;
  }

  const char *newline = find_next('\n');

  if (newline == NULL)
  {
    m_state = State::BROKEN;
    return;
  }

  size_t cr_offset = *(newline - 1) == '\r' ? 1 : 0;
  
  std::string value(pos(), newline - cr_offset);

  DEBUG("field: %s", field.c_str());
  DEBUG("value: %s", value.c_str());

  m_headers->set_field(field, value);

  advance(newline + 1);
}

inline const char & Parser::curr() const
{
  return m_buffer[m_index];
}

inline const char & Parser::next() const
{
  if (m_index+1 < m_buffer_size)
  {
    return m_buffer[m_index+1];
  }
  
  return m_buffer[m_buffer_size];
}

inline const char & Parser::prev() const
{
  return m_buffer[m_index-1];
}

inline void Parser::mark()
{
  m_mark = m_index;
}

inline void Parser::return_to_mark()
{
  m_index = m_mark;
}

inline const char *Parser::pos() const
{
  return m_buffer + m_index;
}

inline const char *Parser::find_next(const char &find) const
{
  return strchr(m_buffer + m_index, find);
}

inline const char *Parser::advance()
{
  m_index++;
  return m_buffer + m_index;
} 

inline const char *Parser::advance(const char *to)
{
  m_index = to - m_buffer;
  return m_buffer + m_index;
}

inline bool Parser::eof() const
{
  return m_buffer_size < m_index;
}

} // namespace
