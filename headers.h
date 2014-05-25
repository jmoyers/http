#ifndef __HEADERS_H
#define __HEADERS_H

#include <string>
#include <algorithm>
#include <map>

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

    std::string& getField(std::string name) 
    { 
      return m_fields[name]; 
    }
    
    void setField(std::string &name, const std::string &value) 
    {
      // http://goo.gl/gEA0Tn
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      m_fields[name] = value;
    }

    Upgrade getUpgrade() 
    { 
      if (m_fields["upgrade"] == "websocket")
      {
        return Upgrade::WEBSOCKET;
      }

      return Upgrade::NONE;
    }

    Method getMethod() { return m_method; }
    void setMethod(Method method) { m_method = method; }

    Version getHTTPVersion() { return m_http_version; }
    void setHTTPVersion(Version v) { m_http_version = v; }

  private:
    Method m_method;
    std::string m_path;
    Upgrade m_upgrade;
    Version m_http_version;
    std::map<std::string, std::string> m_fields;
};

#endif
