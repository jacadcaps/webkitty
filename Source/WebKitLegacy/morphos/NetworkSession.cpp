#include "NetworkSession.h"
#include "WebProcess.h"

namespace WebKit {

void NetworkSession::ensureCacheEngine(Function<void(CacheStorage::Engine&)>&& callback)
{
    if (m_cacheEngine)
        return callback(*m_cacheEngine);

    m_cacheEngine = CacheStorage::Engine::create(*this, "PROGDIR:Cache/CacheStorage"_s);
    callback(*m_cacheEngine);
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

