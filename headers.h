#include <string>
#include <map>

namespace Multiplexer
{
  class Headers
  {
    public:
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

      typedef struct Version
      {
        int major;
        int minor;
        int patch;
      } Version;

      Headers() : 
        m_method(NONE),
        m_path(),
        m_http_version{0, 0, 0},
        m_fields()
      {}

      std::string getField(std::string name) { return m_fields[name]; }
      void setField(std::string name, std::string value) { m_fields[name] = value; }

      Method getMethod() { return m_method; }
      void setMethod(Method method) { m_method = method; }

      Version getHTTPVersion() { return m_http_version; }
      void setHTTPVersion(Version v) { m_http_version = v; }

    private:
      Method m_method;
      std::string m_path;
      Version m_http_version;
      std::map<std::string, std::string> m_fields;
  };
}
