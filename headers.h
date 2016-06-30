#ifndef __HEADERS_H
#define __HEADERS_H

#include <string>
#include <algorithm>
#include <map>

namespace Http
{

class Headers
{
  public:
    enum class Method
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

    enum class Upgrade
    {
      NONE,
      TLS,
      WEBSOCKET
    };

    typedef struct Version
    {
      int major;
      int minor;
      int patch;
    } Version;

    Headers() : 
      m_upgrade(Upgrade::NONE),
      m_method(Method::NONE),
      m_path(),
      m_http_version{0, 0, 0},
      m_fields()
    {}

    std::string& get_field(std::string name) 
    { 
      return m_fields[name]; 
    }
    
    void set_field(std::string &name, const std::string &value) 
    {
      // http://goo.gl/gEA0Tn
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      m_fields[name] = value;
    }

    Upgrade get_upgrade() 
    { 
      if (m_fields["upgrade"] == "websocket")
      {
        return Upgrade::WEBSOCKET;
      }

      return Upgrade::NONE;
    }

    Method get_method() { return m_method; }
    void set_method(Method method) { m_method = method; }

    Version get_http_version() { return m_http_version; }
    void set_http_version(Version v) { m_http_version = v; }

  private:
    Method m_method;
    std::string m_path;
    Upgrade m_upgrade;
    Version m_http_version;
    std::map<std::string, std::string> m_fields;
};

} // namespace

#endif
