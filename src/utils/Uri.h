/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#ifndef URI_H
#define URI_H
#include "Array.h"
#include "StringBase.h"
#include <map>

namespace metacpp
{

/** \brief Basic URI parser
 *
 * Uri example: http://user:password@example.com:8081/foo/bar?key1=value1&key2=value2
*/
template<typename CharT>
class UriBase
{
public:
    UriBase()=delete;
    UriBase(const UriBase&)=default;
    UriBase& operator=(const UriBase&)=default;
    /** \brief Construct a new instance of UriBase using specified uri string */
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
    /** \brief Extracts scheme name from the uri
     *
     * In the above example this method will return "http"
    */
    const StringBase<CharT>& schemeName() const { return m_schemeName; }
    /** \brief Extracts hierarchy from the uri
     *
     * In the above example this method will return "user:password@example.com:8081/foo/bar"
    */
    const StringBase<CharT>& hierarchy() const { return m_hierarchy; }
    /** \brief Extracts host from the uri
     *
     * In the above example this method will return "example.com"
    */
    const StringBase<CharT>& host() const { return m_host; }
    /** \brief Extracts port from the uri
     *
     * In the above example this method will return "8081"
    */
    const StringBase<CharT>& port() const { return m_port; }
    /** \brief Extracts user name from the uri
     *
     * In the above example this method will return "user"
    */
    const StringBase<CharT>& username() const { return m_username; }
    /** \brief Extracts password from the uri
     *
     * In the above example this method will return "password"
    */
    const StringBase<CharT>& password() const { return m_password; }
    /** \brief Extracts path from the uri
     *
     * In the above example this method will return "foo/bar"
    */
    const StringBase<CharT>& path() const { return m_path; }
    /** \brief Extracts named parameter from the uri
     *
     * In the above example named parameters are key1=value1 and key2=value2
    */
    const StringBase<CharT>& param(const StringBase<CharT>& key) const
    {
        auto it = std::find_if(m_params.begin(), m_params.end(),
            [=](const std::pair<StringBase<CharT>, StringBase<CharT> >& param) { return param.first == key; });
        if (it != m_params.end())
            return it->second;
        else
            return StringBase<CharT>::getNull();
    }

    String toString() const
    {
        StringStream os;
        if (!m_schemeName.isNullOrEmpty()) os << m_schemeName << "://";
        if (!m_username.isNullOrEmpty()) os << m_username;
        if (!m_password.isNullOrEmpty()) os << ":" << m_password;
        if (!m_username.isNullOrEmpty() || m_username.isNullOrEmpty()) os << "@";
        if (!m_path.isNullOrEmpty()) os << "/" << m_path;
        if (m_params.size())
        {
            auto assignments = m_params.template map<StringBase<CharT> >
                    ([](const std::pair<StringBase<CharT>, StringBase<CharT> >& param) -> StringBase<CharT>
                        { return param.first + "=" + param.second; });
            os << "?" << join(assignments, "&");
        }
        return os.str();
    }
private:
    StringBase<CharT> m_schemeName, m_hierarchy, m_host, m_port, m_username, m_password, m_path;
    Array<std::pair<StringBase<CharT>, StringBase<CharT> > > m_params;
};

typedef UriBase<char> Uri;
typedef UriBase<char16_t> WUri;

template<typename CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const UriBase<CharT>& uri)
{
    return os << uri.toString();
}

} // namespace metacpp

#endif // URI_H
