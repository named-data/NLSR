#ifndef NLSR_KM_HPP
#define NLSR_KM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "nlsr_conf_param.hpp"

namespace nlsr
{
    class KeyManager
    {
    public:
        KeyManager()
            :kChain()
        {
        }

        ndn::KeyChain& getKeyChain()
        {
            return kChain;
        }

        void initKeyManager(ConfParameter &cp);

    private:
        ndn::KeyChain kChain;
    };
}

#endif
