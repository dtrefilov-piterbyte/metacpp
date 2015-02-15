#ifndef URI_H
#define URI_H
#include "Array.h"
#include "String.h"
#include <map>

namespace metacpp
{

/** \brief Basic URI parser */
template<typename CharT>
class UriBase
{
public:
    UriBase()=delete;
    UriBase(const UriBase&)=default;
    UriBase& operator=(const UriBase&)=default;
    explicit UriBase(const StringBase<CharT>& uri)
    {
        parseUri(uri);
    }

    ~UriBase()
    {
    }

private:
    void parseUri(const StringBase<CharT>& uri)
    {
        const size_t npos = StringBase<CharT>::npos;
        if (uri.isNullOrEmpty()) return;
        size_t schemeEnd = uri.firstIndexOf("://");
        auto hierarchyBegin = uri.begin();
        if (npos != schemeEnd)
        {
            hierarchyBegin += schemeEnd + 3;
            m_schemeName = uri.substr(0, schemeEnd);
        }
        auto hierarchyEnd = std::find(hierarchyBegin, uri.end(), '?');
        m_hierarchy = StringBase<CharT>(hierarchyBegin, hierarchyEnd);
        size_t authSepPos = m_hierarchy.lastIndexOf("@");
        if (npos != authSepPos)
        {
            StringBase<CharT> authInfo = m_hierarchy.substr(0, authSepPos);
            size_t passStart = authInfo.firstIndexOf(":");
            if (npos != passStart)
            {
                m_username = authInfo.substr(0, passStart);
                m_password = authInfo.substr(passStart + 1);
            }
            else
                m_username = authInfo;
        }
        size_t hostStart = npos != authSepPos ? authSepPos + 1 : 0;
        size_t portStart = m_hierarchy.nextIndexOf(":", hostStart);
        size_t pathStart = m_hierarchy.nextIndexOf("/", hostStart);
        if (npos != portStart)
        {
            pathStart = m_hierarchy.nextIndexOf("/", portStart + 1);
            m_host = m_hierarchy.substr(hostStart, portStart - hostStart);
            m_port = m_hierarchy.substr(portStart + 1, npos != pathStart ? pathStart - portStart - 1 : npos);
        }
        else
            m_host = m_hierarchy.substr(hostStart, npos != pathStart ? pathStart - hostStart : npos);
        if (npos != pathStart)
            m_path = m_hierarchy.substr(pathStart + 1);

        if (hierarchyEnd != uri.end())
        {
            StringBase<CharT> paramsString(hierarchyEnd + 1, uri.end());
            for (auto& param : paramsString.split('&'))
            {
                size_t nSep = param.firstIndexOf("=");
                if (param.npos != nSep)
                    m_params.push_back(std::make_pair(param.substr(0, nSep), param.substr(nSep + 1)));
                else
                    m_params.push_back(std::make_pair(param, StringBase<CharT>()));
            }
        }
    }
public:
    const StringBase<CharT>& schemeName() const { return m_schemeName; }
    const StringBase<CharT>& hierarchy() const { return m_hierarchy; }
    const StringBase<CharT>& host() const { return m_host; }
    const StringBase<CharT>& port() const { return m_port; }
    const StringBase<CharT>& username() const { return m_username; }
    const StringBase<CharT>& password() const { return m_password; }
    const StringBase<CharT>& path() const { return m_path; }

    const StringBase<CharT>& param(const StringBase<CharT>& key) const
    {
        auto it = std::find_if(m_params.begin(), m_params.end(),
            [=](const std::pair<StringBase<CharT>, StringBase<CharT> >& param) { return param.first == key; });
        if (it != m_params.end())
            return it->second;
        else
            return StringBase<CharT>::getNull();
    }
private:
    StringBase<CharT> m_schemeName, m_hierarchy, m_host, m_port, m_username, m_password, m_path;
    Array<std::pair<StringBase<CharT>, StringBase<CharT> > > m_params;
};

typedef UriBase<char> Uri;
typedef UriBase<char16_t> WUri;

}

#endif // URI_H
