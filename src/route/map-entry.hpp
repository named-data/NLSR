#ifndef NLSR_MAP_ENTRY_HPP
#define NLSR_MAP_ENTRY_HPP

#include <boost/cstdint.hpp>
#include <ndn-cxx/name.hpp>

namespace nlsr {

class MapEntry
{
public:
  MapEntry()
    : m_router()
    , m_mappingNumber(-1)
  {
  }

  ~MapEntry()
  {
  }

  MapEntry(const ndn::Name& rtr, int32_t mn)
  {
    m_router = rtr;
    m_mappingNumber = mn;
  }

  const ndn::Name&
  getRouter() const
  {
    return m_router;
  }

  int32_t
  getMappingNumber() const
  {
    return m_mappingNumber;
  }

private:
  ndn::Name m_router;
  int32_t m_mappingNumber;
};

inline std::ostream&
operator<<(std::ostream& os, const MapEntry& mpe)
{
  os << "MapEntry: ( Router: " << mpe.getRouter() << " Mapping No: ";
  os << mpe.getMappingNumber() << " )" << std::endl;
  return os;
}

} // namespace nlsr

#endif // NLSR_MAP_ENTRY_HPP
