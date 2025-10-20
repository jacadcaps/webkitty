#include "WebKit.h"
#include "LegacyHistoryItemClient.h"
#include <wtf/NeverDestroyed.h>

namespace WebKit {

LegacyHistoryItemClient& LegacyHistoryItemClient::singleton()
{
    static NeverDestroyed<Ref<LegacyHistoryItemClient>> client { adoptRef(*new LegacyHistoryItemClient) };
    return client.get().get();
}

void LegacyHistoryItemClient::historyItemChanged(const WebCore::HistoryItem&)
{
}

void LegacyHistoryItemClient::clearChildren(const WebCore::HistoryItem&) const
{
}

}
