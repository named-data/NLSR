#ifndef NLSR_FACE_MAP_HPP
#define NLSR_FACE_MAP_HPP

namespace nlsr {

class FaceMapEntry {

public:
  FaceMapEntry(const std::string& faceUri, uint32_t faceId)
    : m_faceUri(faceUri)
    , m_faceId(faceId)
  {
  }

  void
  setFaceUri(const std::string& faceUri)
  {
    m_faceUri = faceUri;
  }

  const std::string&
  getFaceUri() const
  {
    return m_faceUri;
  }

  void
  setFaceId(uint32_t faceId)
  {
    m_faceId = faceId;
  }

  uint32_t
  getFaceId() const
  {
    return m_faceId;
  }

  bool
  compare(const FaceMapEntry& fme)
  {
    return m_faceUri == fme.getFaceUri();
  }

private:
  std::string m_faceUri;
  uint32_t m_faceId;
};

inline std::ostream&
operator<<(std::ostream& os, const FaceMapEntry& fme)
{
  os << "Face Map Entry (FaceUri: " << fme.getFaceUri() << " Face Id: ";
  os << fme.getFaceId() << ")" << std::endl;
  return os;
}

class FaceMap {

public:
  FaceMap()
  {
  }

  ~FaceMap()
  {
  }

  inline void
  update(const std::string& faceUri, uint32_t faceId)
  {
    FaceMapEntry fme(faceUri, faceId);
    std::list<FaceMapEntry>::iterator it = std::find_if(m_table.begin(),
                                                        m_table.end(),
                                                        bind(&FaceMapEntry::compare,
                                                             &fme, _1));
    if (it == m_table.end()) {
      m_table.push_back(fme);
    }
    else {
      (*it).setFaceId(fme.getFaceId());
    }
  }

  inline uint32_t
  getFaceId(const std::string& faceUri)
  {
    FaceMapEntry fme(faceUri, 0);
    std::list<FaceMapEntry>::iterator it = std::find_if(m_table.begin(),
                                                        m_table.end(),
                                                        bind(&FaceMapEntry::compare,
                                                             &fme, _1));
    if (it != m_table.end()) {
      return (*it).getFaceId();
    }
    return 0;
  }

  inline void
  print()
  {
    std::cout << "------- Face Map-----------" << std::endl;
    for(std::list<FaceMapEntry>::iterator it = m_table.begin();
        it != m_table.end(); ++it) {
          std::cout << (*it);
    }
  }

private:
  std::list<FaceMapEntry> m_table;
};

} //namespace nlsr

#endif //NLSR_FACE_MAP_HPP
