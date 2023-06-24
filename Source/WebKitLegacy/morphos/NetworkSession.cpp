#include "NetworkSession.h"
#include "WebProcess.h"

namespace WebKit {

CacheStorage::Engine& NetworkSession::ensureCacheEngine()
{
    if (m_cacheEngine)
        return *(m_cacheEngine.get());

    m_cacheEngine = CacheStorage::Engine::create(*this, "PROGDIR:Cache/CacheStorage"_s);
    return *(m_cacheEngine.get());
}

void NetworkSession::clearCacheEngine()
{
    m_cacheEngine = nullptr;
}

PAL::SessionID NetworkSession::sessionID() const
{
    return WebProcess::singleton().sessionID();
}

void NetworkSession::shutdown()
{
    if (m_cacheEngine)
        m_cacheEngine->shutdown();
    m_cacheEngine = nullptr;
}

}

