#pragma once
#define ID_EBML         0x1A45DFA3
#define ID_SEGMENT      0x18538067
#define ID_INFO         0x1549A966
#define ID_DURATION     0x4489
#define ID_TRACKS       0x1654AE6B
#define ID_TRACK_ENTRY  0xAE
#define ID_TRACK_TYPE   0x83
#define ID_CODEC_ID     0x86

#define TRACK_TYPE_VIDEO 1
#define TRACK_TYPE_AUDIO 2

// #pragma GCC optimize ("O0")
// #define DEBUG_FILE

namespace WebCore {

class WebMParser {
public:
    WebMParser() {
    };

    int parse(const unsigned char *buffer, size_t length, ac_initialization_segment_stream* infos, size_t infosMax)
    {
        return parse_webm(buffer, length, infos, infosMax);
    }

    static bool isWebM(const unsigned char *header, size_t length)
    {
        if (length < 8)
            return false;
        return ((uint32_t *)(header))[0] == 0x1a45dfa3 ? true : false;
    }

protected:
    static int read_vint(const uint8_t *data, size_t size, uint64_t *value, size_t *length) {
        if (size == 0) return 0;
        uint8_t first = data[0];
        uint8_t mask = 0x80;
        *length = 1;
        while (!(first & mask) && mask) {
            mask >>= 1;
            (*length)++;
        }
        if (*length > 8 || *length > size) return 0;

        *value = first & (mask - 1);
        for (size_t i = 1; i < *length; i++) {
            *value = (*value << 8) | data[i];
        }
        return 1;
    }

    // Read EBML ID (do not strip top bit like VINTs)
    static int read_id(const uint8_t *data, size_t size, uint64_t *id, size_t *length) {
        if (size == 0) return 0;
        uint8_t first = data[0];
        uint8_t mask = 0x80;
        *length = 1;
        while (!(first & mask) && mask) {
            mask >>= 1;
            (*length)++;
        }
        if (*length > 4 || *length > size) return 0;

        *id = 0;
        for (size_t i = 0; i < *length; i++) {
            *id = (*id << 8) | data[i];
        }
        return 1;
    }

    // Read big-endian integer from byte array
    static uint64_t parse_uint(const uint8_t *data, size_t size) {
        uint64_t result = 0;
        for (size_t i = 0; i < size; i++) {
            result = (result << 8) | data[i];
        }
        return result;
    }

    int parse_webm(const uint8_t *data, size_t size, ac_initialization_segment_stream* infos, size_t infosMax) {
        size_t offset = 0;
        size_t infosCount = 0;
        double duration = 0;

        while (offset < size) {
            uint64_t id, length;
            size_t id_len, len_len;

            if (!read_id(data + offset, size - offset, &id, &id_len)) return 0;
            if (!read_vint(data + offset + id_len, size - offset - id_len, &length, &len_len)) return 0;

            if (id == ID_EBML) {
                offset += id_len + len_len + (size_t)length;
                continue;
            }

            if (id == ID_SEGMENT) {
                offset += id_len + len_len;
                size_t segment_end = std::min(uint64_t(size), offset + length);
                size_t seg_off = offset;

                while (seg_off < segment_end) {
                    uint64_t sid, slen;
                    size_t sid_len, slen_len;

                    if (!read_id(data + seg_off, segment_end - seg_off, &sid, &sid_len)) break;
                    if (!read_vint(data + seg_off + sid_len, segment_end - seg_off - sid_len, &slen, &slen_len)) break;

                    const uint8_t *payload = data + seg_off + sid_len + slen_len;

                    if (sid == ID_INFO) {
                        size_t info_off = 0;
                        while (info_off < slen) {
                            uint64_t iid, ilen;
                            size_t iid_len, ilen_len;
                            if (!read_id(payload + info_off, slen - info_off, &iid, &iid_len)) break;
                            if (!read_vint(payload + info_off + iid_len, slen - info_off - iid_len, &ilen, &ilen_len)) break;

                            if (iid == ID_DURATION && (ilen == 4 || ilen == 8)) {
                                double dur = 0.0;
                                if (ilen == 4) {
                                    // 4-byte float (big endian)
                                    uint32_t tmp = 0;
                                    for (size_t i = 0; i < 4; i++) {
                                        tmp = (tmp << 8) | payload[info_off + iid_len + ilen_len + i];
                                    }
                                    float f;
                                    memcpy(&f, &tmp, sizeof(float));
                                    dur = f;
                                } else if (ilen == 8) {
                                    // 8-byte double (big endian)
                                    uint64_t tmp = 0;
                                    for (size_t i = 0; i < 8; i++) {
                                        tmp = (tmp << 8) | payload[info_off + iid_len + ilen_len + i];
                                    }
                                    double d;
                                    memcpy(&d, &tmp, sizeof(double));
                                    dur = d;
                                }
                                duration = dur;
                            }

                            info_off += iid_len + ilen_len + ilen;
                        }
                    } else if (sid == ID_TRACKS) {
                        size_t track_off = 0;
                        while (track_off < slen) {
                            uint64_t tid, tlen;
                            size_t tid_len, tlen_len;
                            if (!read_id(payload + track_off, slen - track_off, &tid, &tid_len)) break;
                            if (!read_vint(payload + track_off + tid_len, slen - track_off - tid_len, &tlen, &tlen_len)) break;

                            const uint8_t *track_entry = payload + track_off + tid_len + tlen_len;

                            if (tid == ID_TRACK_ENTRY) {
                                size_t entry_off = 0;
                                uint8_t track_type = 0;
                                char codec_id[64] = {0};

                                while (entry_off < tlen) {
                                    uint64_t eid, elen;
                                    size_t eid_len, elen_len;
                                    if (!read_id(track_entry + entry_off, tlen - entry_off, &eid, &eid_len)) break;
                                    if (!read_vint(track_entry + entry_off + eid_len, tlen - entry_off - eid_len, &elen, &elen_len)) break;

                                    const uint8_t *edata = track_entry + entry_off + eid_len + elen_len;

                                    if (eid == ID_TRACK_TYPE && elen == 1) {
                                        track_type = edata[0];
                                    } else if (eid == ID_CODEC_ID && elen < sizeof(codec_id)) {
                                        memcpy(codec_id, edata, elen);
                                        codec_id[elen] = 0;
                                    }

                                    entry_off += eid_len + elen_len + elen;
                                }

                                if (infosCount >= infosMax)
                                    break;
                                auto info = &infos[infosCount];

                                if (track_type == TRACK_TYPE_VIDEO) {
                                    info->type = AC_STREAM_TYPE_VIDEO;
                                    stccpy(info->codecName, codec_id, sizeof(info->codecName));
                                    info->duration = duration;
                                    info->bitrate = 0;
                                    infosCount++;
                                } else if (track_type == TRACK_TYPE_AUDIO) {
                                    info->type = AC_STREAM_TYPE_AUDIO;
                                    stccpy(info->codecName, codec_id, sizeof(info->codecName));
                                    info->duration = duration;
                                    info->bitrate = 0;
                                    infosCount++;
                                }
                            }

                            track_off += tid_len + tlen_len + tlen;
                        }
                    }

                    seg_off += sid_len + slen_len + slen;
                }

                break;
            } else {
                offset += id_len + len_len + (size_t)length;
            }
        }
        return infosCount;
    }
};

}
